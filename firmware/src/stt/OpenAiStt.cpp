#include "OpenAiStt.h"

#include <SD.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

namespace OpenAiStt {

bool transcribeFile(const String& wavPath, const String& apiKey, const String& model,
                    const String& language, String& outText, String& outError) {
  outText = "";
  outError = "";

  File file = SD.open(wavPath, FILE_READ);
  if (!file) {
    outError = "WAV open failed";
    return false;
  }
  const size_t fileSize = file.size();
  if (fileSize < 44) {
    file.close();
    outError = "WAV too small";
    return false;
  }

  // Multipart body built in PSRAM-friendly chunks via Stream — simple buffer approach for MVP
  const String boundary = "----INoteBoundary7MA4YWxkTrZu0gW";
  String head;
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"model\"\r\n\r\n";
  head += model + "\r\n";
  if (language.length()) {
    head += "--" + boundary + "\r\n";
    head += "Content-Disposition: form-data; name=\"language\"\r\n\r\n";
    head += language + "\r\n";
  }
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n";
  head += "Content-Type: audio/wav\r\n\r\n";

  const String tail = "\r\n--" + boundary + "--\r\n";
  const size_t totalLen = head.length() + fileSize + tail.length();

  // Load file into PSRAM buffer if available
  uint8_t* audioBuf = (uint8_t*)ps_malloc(fileSize);
  if (!audioBuf) audioBuf = (uint8_t*)malloc(fileSize);
  if (!audioBuf) {
    file.close();
    outError = "OOM reading WAV";
    return false;
  }
  if (file.read(audioBuf, fileSize) != (int)fileSize) {
    free(audioBuf);
    file.close();
    outError = "WAV read failed";
    return false;
  }
  file.close();

  WiFiClientSecure client;
  client.setInsecure();  // bring-up simplicity; pin certs later

  HTTPClient https;
  if (!https.begin(client, "https://api.openai.com/v1/audio/transcriptions")) {
    free(audioBuf);
    outError = "HTTPS begin failed";
    return false;
  }
  https.addHeader("Authorization", "Bearer " + apiKey);
  https.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
  https.setTimeout(120000);

  uint8_t* body = (uint8_t*)ps_malloc(totalLen);
  if (!body) body = (uint8_t*)malloc(totalLen);
  if (!body) {
    free(audioBuf);
    https.end();
    outError = "OOM multipart";
    return false;
  }
  size_t off = 0;
  memcpy(body + off, head.c_str(), head.length());
  off += head.length();
  memcpy(body + off, audioBuf, fileSize);
  off += fileSize;
  memcpy(body + off, tail.c_str(), tail.length());
  free(audioBuf);

  const int code = https.POST(body, totalLen);
  free(body);
  String resp = https.getString();
  https.end();

  if (code < 200 || code >= 300) {
    outError = "HTTP " + String(code) + ": " + resp.substring(0, 120);
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, resp);
  if (err) {
    outError = "JSON parse failed";
    return false;
  }
  outText = doc["text"] | "";
  outText.trim();
  return true;
}

}  // namespace OpenAiStt
