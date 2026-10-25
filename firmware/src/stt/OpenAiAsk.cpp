#include "OpenAiAsk.h"

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

namespace {
String buildContext(const std::vector<NoteMeta>& notes, size_t maxChars) {
  String ctx;
  for (const auto& n : notes) {
    String chunk = String("- [") + n.id + "] type=" + (n.type.length() ? n.type : "note");
    if (n.title.length()) chunk += String(" title=") + n.title;
    if (n.dueAt.length()) chunk += String(" due=") + n.dueAt;
    chunk += String(" done=") + (n.done ? "1" : "0");
    chunk += "\n  " + n.transcript + "\n";
    if (ctx.length() + chunk.length() > maxChars) break;
    ctx += chunk;
  }
  return ctx;
}

bool chat(const String& system, const String& user, const String& apiKey, const String& model,
          String& outText, String& outError) {
  outText = "";
  outError = "";
  JsonDocument body;
  body["model"] = model.length() ? model : "gpt-4o-mini";
  body["temperature"] = 0.2;
  JsonArray messages = body["messages"].to<JsonArray>();
  JsonObject sys = messages.add<JsonObject>();
  sys["role"] = "system";
  sys["content"] = system;
  JsonObject u = messages.add<JsonObject>();
  u["role"] = "user";
  u["content"] = user;

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
  https.setTimeout(90000);
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
  outText = content;
  outText.trim();
  return true;
}
}  // namespace

namespace OpenAiAsk {

bool ask(const String& question, const std::vector<NoteMeta>& context, const String& apiKey,
         const String& model, String& outAnswer, String& outError) {
  const String ctx = buildContext(context, 12000);
  const String system =
      "Answer using only the provided personal notes context. "
      "If unknown, say you do not know. Reply in the user's language. Plain text only.";
  const String user = String("Context:\n") + ctx + "\n\nQuestion:\n" + question;
  return chat(system, user, apiKey, model, outAnswer, outError);
}

bool briefing(const String& range, const std::vector<NoteMeta>& context, const String& apiKey,
              const String& model, String& outText, String& outError) {
  const String ctx = buildContext(context, 12000);
  const String system =
      "Write a concise personal briefing for the requested range (day or week). "
      "Include open tasks, upcoming appointments, and notable notes. Plain text only.";
  const String user = String("range=") + range + "\nContext:\n" + ctx;
  return chat(system, user, apiKey, model, outText, outError);
}

}  // namespace OpenAiAsk
