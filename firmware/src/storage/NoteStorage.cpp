#include "NoteStorage.h"

#include <SD.h>
#include <ArduinoJson.h>
#include <time.h>

namespace {
const char* kRoot = "/inote";
const char* kIndex = "/inote/index.json";
}

namespace NoteStorage {

bool begin() {
  return ensureLayout();
}

bool ensureLayout() {
  if (!SD.exists(kRoot)) {
    if (!SD.mkdir(kRoot)) return false;
  }
  if (!SD.exists("/inote/notes")) {
    if (!SD.mkdir("/inote/notes")) return false;
  }
  if (!SD.exists(kIndex)) {
    File f = SD.open(kIndex, FILE_WRITE);
    if (!f) return false;
    f.print("{\"version\":1,\"notes\":[]}");
    f.close();
  }
  return true;
}

String utcNow() {
  time_t now = time(nullptr);
  if (now < 100000) {
    // Clock not set — use millis-based placeholder stamp
    char buf[32];
    snprintf(buf, sizeof(buf), "1970-01-01T00:%02lu:%02luZ",
             (unsigned long)((millis() / 60000) % 60),
             (unsigned long)((millis() / 1000) % 60));
    return String(buf);
  }
  struct tm t;
  gmtime_r(&now, &t);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &t);
  return String(buf);
}

String newNoteId() {
  time_t now = time(nullptr);
  char buf[40];
  if (now > 100000) {
    struct tm t;
    gmtime_r(&now, &t);
    strftime(buf, sizeof(buf), "%Y%m%dT%H%M%SZ", &t);
  } else {
    snprintf(buf, sizeof(buf), "T%lu", (unsigned long)millis());
  }
  String id = String(buf) + "-" + String((uint32_t)esp_random(), HEX);
  id.toLowerCase();
  return id;
}

String noteDir(const String& id) { return String(kRoot) + "/notes/" + id; }
String metaPath(const String& id) { return noteDir(id) + "/meta.json"; }
String audioPath(const String& id) { return noteDir(id) + "/audio.wav"; }
String transcriptPath(const String& id) { return noteDir(id) + "/transcript.txt"; }

bool writeMeta(const NoteMeta& meta) {
  const String dir = noteDir(meta.id);
  if (!SD.exists(dir)) {
    if (!SD.mkdir(dir)) return false;
  }
  JsonDocument doc;
  doc["id"] = meta.id;
  doc["createdAt"] = meta.createdAt;
  doc["updatedAt"] = meta.updatedAt;
  doc["audioPath"] = meta.audioPath.length() ? meta.audioPath : ("notes/" + meta.id + "/audio.wav");
  doc["transcript"] = meta.transcript;
  doc["status"] = noteStatusToString(meta.status);
  doc["tags"] = JsonArray();
  if (meta.type.length()) doc["type"] = meta.type;
  else doc["type"] = nullptr;
  if (meta.error.length()) doc["error"] = meta.error;
  else doc["error"] = nullptr;
  doc["durationMs"] = meta.durationMs;

  File f = SD.open(metaPath(meta.id), FILE_WRITE);
  if (!f) return false;
  serializeJsonPretty(doc, f);
  f.close();

  if (meta.transcript.length()) {
    File t = SD.open(transcriptPath(meta.id), FILE_WRITE);
    if (t) {
      t.print(meta.transcript);
      t.close();
    }
  }
  return updateIndex(meta);
}

bool readMeta(const String& id, NoteMeta& out) {
  File f = SD.open(metaPath(id), FILE_READ);
  if (!f) return false;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;
  out.id = doc["id"] | id;
  out.createdAt = doc["createdAt"] | "";
  out.updatedAt = doc["updatedAt"] | "";
  out.audioPath = doc["audioPath"] | "";
  out.transcript = doc["transcript"] | "";
  out.status = noteStatusFromString(doc["status"] | "error");
  out.error = doc["error"].isNull() ? "" : String(doc["error"].as<const char*>());
  out.durationMs = doc["durationMs"] | 0;
  out.type = doc["type"].isNull() ? "" : String(doc["type"].as<const char*>());
  return true;
}

bool loadIndex(std::vector<IndexEntry>& out) {
  out.clear();
  File f = SD.open(kIndex, FILE_READ);
  if (!f) return false;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;
  for (JsonObject o : doc["notes"].as<JsonArray>()) {
    IndexEntry e;
    e.id = o["id"] | "";
    e.status = noteStatusFromString(o["status"] | "error");
    e.createdAt = o["createdAt"] | "";
    if (e.id.length()) out.push_back(e);
  }
  return true;
}

bool updateIndex(const NoteMeta& meta) {
  std::vector<IndexEntry> entries;
  loadIndex(entries);
  bool found = false;
  for (auto& e : entries) {
    if (e.id == meta.id) {
      e.status = meta.status;
      e.createdAt = meta.createdAt;
      found = true;
      break;
    }
  }
  if (!found) {
    entries.insert(entries.begin(), {meta.id, meta.status, meta.createdAt});
  }

  JsonDocument doc;
  doc["version"] = 1;
  JsonArray arr = doc["notes"].to<JsonArray>();
  for (const auto& e : entries) {
    JsonObject o = arr.add<JsonObject>();
    o["id"] = e.id;
    o["status"] = noteStatusToString(e.status);
    o["createdAt"] = e.createdAt;
  }
  File f = SD.open(kIndex, FILE_WRITE);
  if (!f) return false;
  serializeJsonPretty(doc, f);
  f.close();
  return true;
}

bool listPending(std::vector<String>& ids) {
  ids.clear();
  std::vector<IndexEntry> entries;
  if (!loadIndex(entries)) return false;
  for (const auto& e : entries) {
    if (e.status == NoteStatus::Pending || e.status == NoteStatus::Error) {
      ids.push_back(e.id);
    }
  }
  return true;
}

int countByStatus(NoteStatus status) {
  std::vector<IndexEntry> entries;
  if (!loadIndex(entries)) return 0;
  int n = 0;
  for (const auto& e : entries) {
    if (e.status == status) ++n;
  }
  return n;
}

}  // namespace NoteStorage
