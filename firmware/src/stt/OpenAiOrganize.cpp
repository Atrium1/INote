#include "OpenAiOrganize.h"

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

namespace OpenAiOrganize {

bool classify(const String& transcript, const String& nowUtc, const String& apiKey,
              const String& model, Result& out, String& outError) {
  out = {};
  outError = "";
  if (!transcript.length()) {
    outError = "empty transcript";
    return false;
  }

  JsonDocument body;
  body["model"] = model.length() ? model : "gpt-4o-mini";
  body["temperature"] = 0;
  body["response_format"]["type"] = "json_object";
  JsonArray messages = body["messages"].to<JsonArray>();
  JsonObject sys = messages.add<JsonObject>();
  sys["role"] = "system";
  sys["content"] =
      "Classify a personal voice note. Reply JSON only with keys: "
      "type (note|task|appointment), title (short), dueAt (ISO-8601 or null), done (false), tags ([]). "
      "Turkish or English input. If unclear use type=note and dueAt=null. "
      "Resolve relative dates using the provided now UTC. Do not invent dates without cues.";
  JsonObject user = messages.add<JsonObject>();
  user["role"] = "user";
  user["content"] = String("nowUtc=") + nowUtc + "\ntranscript:\n" + transcript;

  String payload;
  serializeJson(body, payload);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  if (!https.begin(client, "https://api.openai.com/v1/chat/completions")) {
    outError = "HTTPS begin failed";
    return false;
  }
  https.addHeader("Authorization", "Bearer " + apiKey);
  https.addHeader("Content-Type", "application/json");
  https.setTimeout(60000);
  const int code = https.POST(payload);
  String resp = https.getString();
  https.end();
  if (code < 200 || code >= 300) {
    outError = "HTTP " + String(code) + ": " + resp.substring(0, 120);
    return false;
  }

  JsonDocument doc;
  if (deserializeJson(doc, resp)) {
    outError = "JSON parse failed";
    return false;
  }
  const char* content = doc["choices"][0]["message"]["content"];
  if (!content) {
    outError = "missing content";
    return false;
  }

  JsonDocument parsed;
  if (deserializeJson(parsed, content)) {
    outError = "content JSON parse failed";
    return false;
  }

  out.type = parsed["type"] | "note";
  if (out.type != "note" && out.type != "task" && out.type != "appointment") {
    out.type = "note";
  }
  out.title = parsed["title"] | "";
  out.dueAt = parsed["dueAt"].isNull() ? "" : String(parsed["dueAt"].as<const char*>());
  out.done = parsed["done"] | false;
  if (out.type != "task") out.done = false;
  return true;
}

}  // namespace OpenAiOrganize
