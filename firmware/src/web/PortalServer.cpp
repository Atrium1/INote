#include "PortalServer.h"

#include "../storage/NoteStorage.h"
#include "../wifi_setup/WifiSetup.h"
#include "../power/Power.h"
#include "../queue/JobQueue.h"
#include "../stt/OpenAiAsk.h"

#include <WebServer.h>
#include <SD.h>
#include <ArduinoJson.h>

namespace {
WebServer server(80);
const InoteConfig* cfgRef = nullptr;
bool busy = false;
String briefingHint;

const char* kFallbackHtml = R"HTML(
<!DOCTYPE html><html lang="tr"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>INote</title></head><body><h1>INote</h1><p>Copy web/ to /inote/web on SD.</p>
<p><a href="/api/notes">/api/notes</a></p></body></html>
)HTML";

bool streamFile(const String& path, const char* contentType) {
  if (!SD.exists(path)) return false;
  File f = SD.open(path, FILE_READ);
  if (!f) return false;
  server.streamFile(f, contentType);
  f.close();
  return true;
}

String readPostBody() {
  if (server.hasArg("plain")) return server.arg("plain");
  return "";
}

void fillMetaObject(JsonObject o, const NoteMeta& meta) {
  o["id"] = meta.id;
  o["createdAt"] = meta.createdAt;
  o["updatedAt"] = meta.updatedAt;
  o["audioPath"] = meta.audioPath;
  o["transcript"] = meta.transcript;
  o["status"] = noteStatusToString(meta.status);
  o["tags"] = JsonArray();
  if (meta.type.length()) o["type"] = meta.type;
  else o["type"] = nullptr;
  if (meta.title.length()) o["title"] = meta.title;
  else o["title"] = nullptr;
  if (meta.dueAt.length()) o["dueAt"] = meta.dueAt;
  else o["dueAt"] = nullptr;
  o["done"] = meta.done;
  if (meta.organizedAt.length()) o["organizedAt"] = meta.organizedAt;
  else o["organizedAt"] = nullptr;
  if (meta.organizeError.length()) o["organizeError"] = meta.organizeError;
  else o["organizeError"] = nullptr;
  if (meta.error.length()) o["error"] = meta.error;
  else o["error"] = nullptr;
  o["durationMs"] = meta.durationMs;
  JsonArray marks = o["markers"].to<JsonArray>();
  for (const auto& m : meta.markers) {
    JsonObject mo = marks.add<JsonObject>();
    mo["tMs"] = m.tMs;
    if (m.label.length()) mo["label"] = m.label;
    else mo["label"] = nullptr;
  }
}

void handleRoot() {
  if (streamFile("/inote/web/index.html", "text/html")) return;
  server.send(200, "text/html", kFallbackHtml);
}

void handleAsset() {
  String uri = server.uri();
  if (uri.endsWith(".css") && streamFile("/inote/web" + uri, "text/css")) return;
  if (uri.endsWith(".js") && streamFile("/inote/web" + uri, "application/javascript")) return;
  server.send(404, "text/plain", "not found");
}

void handleHealth() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["ip"] = WifiSetup::staIp();
  doc["setup"] = WifiSetup::isSetupMode();
  doc["queueBusy"] = JobQueue::isBusy();
  doc["portalBusy"] = busy;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleConfig() {
  JsonDocument doc;
  JsonObject ui = doc["ui"].to<JsonObject>();
  ui["language"] = cfgRef ? cfgRef->uiLanguage : "tr";
  ui["dateFormat"] = cfgRef ? cfgRef->dateFormat : "DMY";
  ui["timeFormat"] = cfgRef ? cfgRef->timeFormat : "H24";
  JsonObject feat = doc["features"].to<JsonObject>();
  feat["organize"] = cfgRef ? cfgRef->organize : false;
  feat["ask"] = cfgRef ? cfgRef->ask : false;
  feat["briefing"] = cfgRef ? cfgRef->briefing : false;
  feat["autoTranscribe"] = cfgRef ? cfgRef->autoTranscribe : true;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleNotes() {
  Power::noteActivity();
  std::vector<IndexEntry> index;
  NoteStorage::loadIndex(index);
  JsonDocument doc;
  JsonObject device = doc["device"].to<JsonObject>();
  device["name"] = "INote";
  device["wifi"] = WifiSetup::isConnected();
  device["batteryPercent"] = Power::batteryPercent();
  device["pendingCount"] = NoteStorage::countByStatus(NoteStatus::Pending);
  device["openTasks"] = NoteStorage::countOpenTasks();
  JsonArray arr = doc["notes"].to<JsonArray>();
  for (const auto& e : index) {
    NoteMeta meta;
    if (!NoteStorage::readMeta(e.id, meta)) continue;
    fillMetaObject(arr.add<JsonObject>(), meta);
  }
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleNoteDone(const String& id) {
  Power::noteActivity();
  NoteMeta meta;
  if (!NoteStorage::readMeta(id, meta)) {
    server.send(404, "text/plain", "not found");
    return;
  }
  if (meta.type != "task") {
    server.send(400, "text/plain", "not a task");
    return;
  }
  bool done = true;
  const String raw = readPostBody();
  if (raw.length()) {
    JsonDocument body;
    if (!deserializeJson(body, raw) && !body["done"].isNull()) done = body["done"].as<bool>();
  }
  meta.done = done;
  meta.updatedAt = NoteStorage::utcNow();
  NoteStorage::writeMeta(meta);
  JsonDocument doc;
  fillMetaObject(doc.to<JsonObject>(), meta);
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleAsk() {
  Power::noteActivity();
  if (!cfgRef || !cfgRef->ask) {
    server.send(403, "text/plain", "ask disabled");
    return;
  }
  if (!cfgRef->hasApiKey() || !WifiSetup::isConnected()) {
    server.send(503, "text/plain", "wifi or api key missing");
    return;
  }
  JsonDocument body;
  if (deserializeJson(body, readPostBody()) || !body["question"].is<const char*>()) {
    server.send(400, "text/plain", "need question");
    return;
  }
  const String question = body["question"].as<String>();
  busy = true;
  std::vector<NoteMeta> notes;
  NoteStorage::listReadyNotes(notes, 20);
  String answer;
  String err;
  const bool ok =
      OpenAiAsk::ask(question, notes, cfgRef->openaiApiKey, cfgRef->askModel, answer, err);
  busy = false;
  if (!ok) {
    server.send(502, "application/json", String("{\"error\":\"") + err + "\"}");
    return;
  }
  const String id = NoteStorage::newNoteId();
  NoteStorage::saveAskLog(id, question, answer);
  JsonDocument doc;
  doc["id"] = id;
  doc["answer"] = answer;
  doc["text"] = answer;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleBriefing() {
  Power::noteActivity();
  if (!cfgRef || !cfgRef->briefing) {
    server.send(403, "text/plain", "briefing disabled");
    return;
  }
  if (!cfgRef->hasApiKey() || !WifiSetup::isConnected()) {
    server.send(503, "text/plain", "wifi or api key missing");
    return;
  }
  String range = "day";
  JsonDocument body;
  if (!deserializeJson(body, readPostBody()) && body["range"].is<const char*>()) {
    range = body["range"].as<String>();
  }
  if (range != "day" && range != "week") range = "day";

  busy = true;
  std::vector<NoteMeta> notes;
  NoteStorage::listReadyNotes(notes, 20);
  String text;
  String err;
  const bool ok =
      OpenAiAsk::briefing(range, notes, cfgRef->openaiApiKey, cfgRef->briefingModel, text, err);
  busy = false;
  if (!ok) {
    server.send(502, "application/json", String("{\"error\":\"") + err + "\"}");
    return;
  }
  const String key = range + "-" + NoteStorage::utcNow().substring(0, 10);
  NoteStorage::saveBriefingCache(key, text, range);
  briefingHint = "briefing ok";
  JsonDocument doc;
  doc["range"] = range;
  doc["generatedAt"] = NoteStorage::utcNow();
  doc["text"] = text;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleNote() {
  Power::noteActivity();
  String uri = server.uri();
  const int prefixLen = String("/api/notes/").length();
  String rest = uri.substring(prefixLen);
  int slash = rest.indexOf('/');
  String id = slash < 0 ? rest : rest.substring(0, slash);
  String tail = slash < 0 ? "" : rest.substring(slash + 1);

  if (tail == "audio") {
    if (!streamFile(NoteStorage::audioPath(id), "audio/wav")) {
      server.send(404, "text/plain", "audio missing");
    }
    return;
  }
  if (tail == "done" && server.method() == HTTP_POST) {
    handleNoteDone(id);
    return;
  }

  NoteMeta meta;
  if (!NoteStorage::readMeta(id, meta)) {
    server.send(404, "text/plain", "not found");
    return;
  }
  JsonDocument doc;
  fillMetaObject(doc.to<JsonObject>(), meta);
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}
}  // namespace

namespace PortalServer {

void begin() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/styles.css", HTTP_GET, handleAsset);
  server.on("/app.js", HTTP_GET, handleAsset);
  server.on("/format.js", HTTP_GET, handleAsset);
  server.on("/api/health", HTTP_GET, handleHealth);
  server.on("/api/config", HTTP_GET, handleConfig);
  server.on("/api/notes", HTTP_GET, handleNotes);
  server.on("/api/ask", HTTP_POST, handleAsk);
  server.on("/api/briefing", HTTP_POST, handleBriefing);
  server.onNotFound([]() {
    if (server.uri().startsWith("/api/notes/")) {
      handleNote();
      return;
    }
    handleAsset();
  });
  server.begin();
}

void loop() { server.handleClient(); }

void setConfig(const InoteConfig* cfg) { cfgRef = cfg; }

bool isBusy() { return busy; }

String lastBriefingHint() { return briefingHint; }

void clearBriefingHint() { briefingHint = ""; }

}  // namespace PortalServer
