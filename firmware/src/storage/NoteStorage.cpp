#include "NoteStorage.h"

#include <SD.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>

namespace {
const char* kRoot = "/inote";
const char* kIndex = "/inote/index.json";
Preferences prefs;

void fillMetaDoc(JsonDocument& doc, const NoteMeta& meta) {
  doc["id"] = meta.id;
  doc["displayId"] = meta.displayId;
  doc["tag"] = meta.tag.length() ? meta.tag : "note";
  doc["createdAt"] = meta.createdAt;
  doc["updatedAt"] = meta.updatedAt;
  doc["audioPath"] = meta.audioPath.length() ? meta.audioPath : ("notes/" + meta.id + "/audio.wav");
  doc["transcript"] = meta.transcript;
  doc["status"] = noteStatusToString(meta.status);
  doc["tags"] = JsonArray();
  if (meta.type.length()) doc["type"] = meta.type;
  else doc["type"] = nullptr;
  if (meta.title.length()) doc["title"] = meta.title;
  else doc["title"] = nullptr;
  if (meta.dueAt.length()) doc["dueAt"] = meta.dueAt;
  else doc["dueAt"] = nullptr;
  doc["done"] = meta.done;
  if (meta.organizedAt.length()) doc["organizedAt"] = meta.organizedAt;
  else doc["organizedAt"] = nullptr;
  if (meta.organizeError.length()) doc["organizeError"] = meta.organizeError;
  else doc["organizeError"] = nullptr;
  if (meta.error.length()) doc["error"] = meta.error;
  else doc["error"] = nullptr;
  doc["durationMs"] = meta.durationMs;
  JsonArray marks = doc["markers"].to<JsonArray>();
  for (const auto& m : meta.markers) {
    JsonObject o = marks.add<JsonObject>();
    o["tMs"] = m.tMs;
    if (m.label.length()) o["label"] = m.label;
    else o["label"] = nullptr;
  }
}
}  // namespace

namespace NoteStorage {

bool begin() {
  prefs.begin("inote", false);
  return ensureLayout();
}

bool ensureLayout() {
  if (!SD.exists(kRoot) && !SD.mkdir(kRoot)) return false;
  if (!SD.exists("/inote/notes") && !SD.mkdir("/inote/notes")) return false;
  if (!SD.exists("/inote/asks") && !SD.mkdir("/inote/asks")) return false;
  if (!SD.exists("/inote/briefings") && !SD.mkdir("/inote/briefings")) return false;
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

String nextDisplayId() {
  uint32_t seq = prefs.getUInt("noteSeq", 0) + 1;
  if (seq > 999) seq = 1;
  prefs.putUInt("noteSeq", seq);
  char buf[8];
  snprintf(buf, sizeof(buf), "#%03lu", (unsigned long)seq);
  return String(buf);
}

String noteDir(const String& id) { return String(kRoot) + "/notes/" + id; }
String metaPath(const String& id) { return noteDir(id) + "/meta.json"; }
String audioPath(const String& id) { return noteDir(id) + "/audio.wav"; }
String transcriptPath(const String& id) { return noteDir(id) + "/transcript.txt"; }

bool writeMeta(const NoteMeta& meta) {
  const String dir = noteDir(meta.id);
  if (!SD.exists(dir) && !SD.mkdir(dir)) return false;
  JsonDocument doc;
  fillMetaDoc(doc, meta);

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
  out = {};
  out.id = doc["id"] | id;
  out.displayId = doc["displayId"].isNull() ? "" : String(doc["displayId"].as<const char*>());
  out.tag = doc["tag"].isNull() ? "note" : String(doc["tag"].as<const char*>());
  if (!isValidTag(out.tag)) out.tag = "note";
  out.createdAt = doc["createdAt"] | "";
  out.updatedAt = doc["updatedAt"] | "";
  out.audioPath = doc["audioPath"] | "";
  out.transcript = doc["transcript"] | "";
  out.status = noteStatusFromString(doc["status"] | "error");
  out.error = doc["error"].isNull() ? "" : String(doc["error"].as<const char*>());
  out.durationMs = doc["durationMs"] | 0;
  out.type = doc["type"].isNull() ? "" : String(doc["type"].as<const char*>());
  out.title = doc["title"].isNull() ? "" : String(doc["title"].as<const char*>());
  out.dueAt = doc["dueAt"].isNull() ? "" : String(doc["dueAt"].as<const char*>());
  out.done = doc["done"] | false;
  out.organizedAt = doc["organizedAt"].isNull() ? "" : String(doc["organizedAt"].as<const char*>());
  out.organizeError =
      doc["organizeError"].isNull() ? "" : String(doc["organizeError"].as<const char*>());
  out.markers.clear();
  if (doc["markers"].is<JsonArray>()) {
    for (JsonObject o : doc["markers"].as<JsonArray>()) {
      NoteMarker m;
      m.tMs = o["tMs"] | 0;
      m.label = o["label"].isNull() ? "" : String(o["label"].as<const char*>());
      out.markers.push_back(m);
    }
  }
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
    e.displayId = o["displayId"].isNull() ? "" : String(o["displayId"].as<const char*>());
    e.tag = o["tag"].isNull() ? "note" : String(o["tag"].as<const char*>());
    e.status = noteStatusFromString(o["status"] | "error");
    e.createdAt = o["createdAt"] | "";
    e.type = o["type"].isNull() ? "" : String(o["type"].as<const char*>());
    e.done = o["done"] | false;
    e.dueAt = o["dueAt"].isNull() ? "" : String(o["dueAt"].as<const char*>());
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
      e.displayId = meta.displayId;
      e.tag = meta.tag;
      e.type = meta.type;
      e.done = meta.done;
      e.dueAt = meta.dueAt;
      found = true;
      break;
    }
  }
  if (!found) {
    IndexEntry e;
    e.id = meta.id;
    e.displayId = meta.displayId;
    e.tag = meta.tag;
    e.status = meta.status;
    e.createdAt = meta.createdAt;
    e.type = meta.type;
    e.done = meta.done;
    e.dueAt = meta.dueAt;
    entries.insert(entries.begin(), e);
  }

  JsonDocument doc;
  doc["version"] = 1;
  JsonArray arr = doc["notes"].to<JsonArray>();
  for (const auto& e : entries) {
    JsonObject o = arr.add<JsonObject>();
    o["id"] = e.id;
    if (e.displayId.length()) o["displayId"] = e.displayId;
    else o["displayId"] = nullptr;
    o["tag"] = e.tag.length() ? e.tag : "note";
    o["status"] = noteStatusToString(e.status);
    o["createdAt"] = e.createdAt;
    if (e.type.length()) o["type"] = e.type;
    else o["type"] = nullptr;
    o["done"] = e.done;
    if (e.dueAt.length()) o["dueAt"] = e.dueAt;
    else o["dueAt"] = nullptr;
  }
  File f = SD.open(kIndex, FILE_WRITE);
  if (!f) return false;
  serializeJsonPretty(doc, f);
  f.close();
  return true;
}

bool removeFromIndex(const String& id) {
  std::vector<IndexEntry> entries;
  loadIndex(entries);
  std::vector<IndexEntry> kept;
  for (const auto& e : entries) {
    if (e.id != id) kept.push_back(e);
  }
  JsonDocument doc;
  doc["version"] = 1;
  JsonArray arr = doc["notes"].to<JsonArray>();
  for (const auto& e : kept) {
    JsonObject o = arr.add<JsonObject>();
    o["id"] = e.id;
    if (e.displayId.length()) o["displayId"] = e.displayId;
    else o["displayId"] = nullptr;
    o["tag"] = e.tag.length() ? e.tag : "note";
    o["status"] = noteStatusToString(e.status);
    o["createdAt"] = e.createdAt;
    if (e.type.length()) o["type"] = e.type;
    else o["type"] = nullptr;
    o["done"] = e.done;
    if (e.dueAt.length()) o["dueAt"] = e.dueAt;
    else o["dueAt"] = nullptr;
  }
  File f = SD.open(kIndex, FILE_WRITE);
  if (!f) return false;
  serializeJsonPretty(doc, f);
  f.close();
  return true;
}

bool deleteNote(const String& id) {
  if (!id.length()) return false;
  SD.remove(metaPath(id));
  SD.remove(audioPath(id));
  SD.remove(transcriptPath(id));
  // Remove any leftover files in note dir
  File dir = SD.open(noteDir(id));
  if (dir && dir.isDirectory()) {
    File child = dir.openNextFile();
    while (child) {
      String path = noteDir(id) + "/" + String(child.name());
      child.close();
      SD.remove(path);
      child = dir.openNextFile();
    }
    dir.close();
  }
  SD.rmdir(noteDir(id));
  return removeFromIndex(id);
}

bool listPending(std::vector<String>& ids) {
  ids.clear();
  std::vector<IndexEntry> entries;
  if (!loadIndex(entries)) return false;
  for (const auto& e : entries) {
    if (e.status == NoteStatus::Pending || e.status == NoteStatus::Error) ids.push_back(e.id);
  }
  return true;
}

bool listNeedingOrganize(std::vector<String>& ids) {
  ids.clear();
  std::vector<IndexEntry> entries;
  if (!loadIndex(entries)) return false;
  for (const auto& e : entries) {
    if (e.status != NoteStatus::Ready) continue;
    NoteMeta meta;
    if (!readMeta(e.id, meta)) continue;
    if (meta.organizedAt.length()) continue;
    if (!meta.transcript.length()) continue;
    ids.push_back(e.id);
  }
  return true;
}

bool listReadyNotes(std::vector<NoteMeta>& out, int limit) {
  out.clear();
  std::vector<IndexEntry> entries;
  if (!loadIndex(entries)) return false;
  for (const auto& e : entries) {
    if ((int)out.size() >= limit) break;
    if (e.status != NoteStatus::Ready) continue;
    NoteMeta meta;
    if (!readMeta(e.id, meta)) continue;
    out.push_back(meta);
  }
  return true;
}

bool listByTag(const String& tag, std::vector<NoteMeta>& out, int limit) {
  out.clear();
  std::vector<IndexEntry> entries;
  if (!loadIndex(entries)) return false;
  for (const auto& e : entries) {
    if ((int)out.size() >= limit) break;
    if (tag.length() && e.tag != tag) continue;
    NoteMeta meta;
    if (!readMeta(e.id, meta)) continue;
    out.push_back(meta);
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

int countOpenTasks() {
  std::vector<IndexEntry> entries;
  if (!loadIndex(entries)) return 0;
  int n = 0;
  for (const auto& e : entries) {
    if (e.type == "task" && !e.done) ++n;
  }
  return n;
}

String nextUpcomingDueAt() {
  std::vector<IndexEntry> entries;
  if (!loadIndex(entries)) return "";
  String best;
  for (const auto& e : entries) {
    if (!e.dueAt.length()) continue;
    if (e.type == "task" && e.done) continue;
    if (!best.length() || e.dueAt < best) best = e.dueAt;
  }
  return best;
}

bool hasPendingWork(bool organizeEnabled) {
  if (countByStatus(NoteStatus::Pending) > 0) return true;
  if (countByStatus(NoteStatus::Error) > 0) return true;
  if (!organizeEnabled) return false;
  std::vector<String> ids;
  listNeedingOrganize(ids);
  return !ids.empty();
}

bool saveAskLog(const String& id, const String& question, const String& answer) {
  ensureLayout();
  JsonDocument doc;
  doc["id"] = id;
  doc["createdAt"] = utcNow();
  doc["question"] = question;
  doc["answer"] = answer;
  doc["text"] = answer;
  File f = SD.open(String("/inote/asks/") + id + ".json", FILE_WRITE);
  if (!f) return false;
  serializeJsonPretty(doc, f);
  f.close();
  return true;
}

bool saveBriefingCache(const String& key, const String& text, const String& range) {
  ensureLayout();
  JsonDocument doc;
  doc["range"] = range;
  doc["generatedAt"] = utcNow();
  doc["text"] = text;
  File f = SD.open(String("/inote/briefings/") + key + ".json", FILE_WRITE);
  if (!f) return false;
  serializeJsonPretty(doc, f);
  f.close();
  return true;
}

}  // namespace NoteStorage
