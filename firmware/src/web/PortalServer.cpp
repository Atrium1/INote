#include "PortalServer.h"

#include "../storage/NoteStorage.h"
#include "../wifi_setup/WifiSetup.h"
#include "../power/Power.h"
#include "../queue/JobQueue.h"

#include <WebServer.h>
#include <SD.h>
#include <ArduinoJson.h>

namespace {
WebServer server(80);
const InoteConfig* cfgRef = nullptr;

// Minimal embedded UI if SD/LittleFS assets missing
const char* kFallbackHtml = R"HTML(
<!DOCTYPE html><html lang="tr"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>INote</title><style>body{font-family:sans-serif;margin:1.5rem;background:#1a1f1c;color:#e8efe9}
a{color:#c4a35a}li{margin:.5rem 0}</style></head><body>
<h1>INote</h1><p id="s"></p><ul id="l"></ul>
<script>
fetch('/api/notes').then(r=>r.json()).then(d=>{
 document.getElementById('s').textContent=(d.device&&d.device.wifi?'Wi-Fi':'Offline');
 const ul=document.getElementById('l');
 (d.notes||[]).forEach(n=>{const li=document.createElement('li');
 li.textContent=(n.status||'')+' — '+(n.transcript||n.id);
 ul.appendChild(li);});
});
</script></body></html>
)HTML";

bool streamFile(const String& path, const char* contentType) {
  if (!SD.exists(path)) return false;
  File f = SD.open(path, FILE_READ);
  if (!f) return false;
  server.streamFile(f, contentType);
  f.close();
  return true;
}

void handleRoot() {
  if (streamFile("/inote/web/index.html", "text/html")) return;
  server.send(200, "text/html", kFallbackHtml);
}

void handleAsset() {
  String uri = server.uri();
  if (uri.endsWith(".css")) {
    if (streamFile("/inote/web" + uri, "text/css")) return;
  } else if (uri.endsWith(".js")) {
    if (streamFile("/inote/web" + uri, "application/javascript")) return;
  }
  server.send(404, "text/plain", "not found");
}

void handleHealth() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["ip"] = WifiSetup::staIp();
  doc["setup"] = WifiSetup::isSetupMode();
  doc["queueBusy"] = JobQueue::isBusy();
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleNotes() {
  std::vector<IndexEntry> index;
  NoteStorage::loadIndex(index);
  JsonDocument doc;
  JsonObject device = doc["device"].to<JsonObject>();
  device["name"] = "INote";
  device["wifi"] = WifiSetup::isConnected();
  device["batteryPercent"] = Power::batteryPercent();
  device["pendingCount"] = NoteStorage::countByStatus(NoteStatus::Pending);
  JsonArray arr = doc["notes"].to<JsonArray>();
  for (const auto& e : index) {
    NoteMeta meta;
    if (!NoteStorage::readMeta(e.id, meta)) continue;
    JsonObject o = arr.add<JsonObject>();
    o["id"] = meta.id;
    o["createdAt"] = meta.createdAt;
    o["updatedAt"] = meta.updatedAt;
    o["audioPath"] = meta.audioPath;
    o["transcript"] = meta.transcript;
    o["status"] = noteStatusToString(meta.status);
    o["tags"] = JsonArray();
    if (meta.type.length()) o["type"] = meta.type;
    else o["type"] = nullptr;
    if (meta.error.length()) o["error"] = meta.error;
    else o["error"] = nullptr;
    o["durationMs"] = meta.durationMs;
  }
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleNote() {
  String uri = server.uri();  // /api/notes/:id
  const int prefixLen = String("/api/notes/").length();
  String rest = uri.substring(prefixLen);
  int slash = rest.indexOf('/');
  String id = slash < 0 ? rest : rest.substring(0, slash);
  if (rest.endsWith("/audio")) {
    const String path = NoteStorage::audioPath(id);
    if (!streamFile(path, "audio/wav")) {
      server.send(404, "text/plain", "audio missing");
    }
    return;
  }
  NoteMeta meta;
  if (!NoteStorage::readMeta(id, meta)) {
    server.send(404, "text/plain", "not found");
    return;
  }
  JsonDocument doc;
  doc["id"] = meta.id;
  doc["createdAt"] = meta.createdAt;
  doc["updatedAt"] = meta.updatedAt;
  doc["audioPath"] = meta.audioPath;
  doc["transcript"] = meta.transcript;
  doc["status"] = noteStatusToString(meta.status);
  doc["durationMs"] = meta.durationMs;
  if (meta.error.length()) doc["error"] = meta.error;
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
  server.on("/api/health", HTTP_GET, handleHealth);
  server.on("/api/notes", HTTP_GET, handleNotes);
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

}  // namespace PortalServer
