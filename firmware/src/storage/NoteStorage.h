#pragma once

#include <Arduino.h>
#include <vector>

enum class NoteStatus {
  Recording,
  Pending,
  Transcribing,
  Ready,
  Error,
};

inline const char* noteStatusToString(NoteStatus s) {
  switch (s) {
    case NoteStatus::Recording: return "recording";
    case NoteStatus::Pending: return "pending";
    case NoteStatus::Transcribing: return "transcribing";
    case NoteStatus::Ready: return "ready";
    case NoteStatus::Error: return "error";
  }
  return "error";
}

inline NoteStatus noteStatusFromString(const String& s) {
  if (s == "recording") return NoteStatus::Recording;
  if (s == "pending") return NoteStatus::Pending;
  if (s == "transcribing") return NoteStatus::Transcribing;
  if (s == "ready") return NoteStatus::Ready;
  return NoteStatus::Error;
}

// Manual on-device tags (primary organize). Separate from API features.organize type.
inline bool isValidTag(const String& t) {
  return t == "note" || t == "work" || t == "idea" || t == "buy" || t == "private";
}

inline const char* const kTagCycle[] = {"note", "work", "idea", "buy", "private"};
inline constexpr int kTagCycleCount = 5;

inline String nextTag(const String& current) {
  int idx = 0;
  for (int i = 0; i < kTagCycleCount; ++i) {
    if (current == kTagCycle[i]) {
      idx = (i + 1) % kTagCycleCount;
      break;
    }
  }
  return String(kTagCycle[idx]);
}

struct NoteMarker {
  uint32_t tMs = 0;
  String label;
};

struct NoteMeta {
  String id;
  String displayId;  // "#001"
  String tag;        // note|work|idea|buy|private
  String createdAt;
  String updatedAt;
  String audioPath;
  String transcript;
  NoteStatus status = NoteStatus::Pending;
  String error;
  uint32_t durationMs = 0;
  String type;  // API organize: task|appointment|note (optional)
  String title;
  String dueAt;
  bool done = false;
  String organizedAt;
  String organizeError;
  std::vector<NoteMarker> markers;
};

struct IndexEntry {
  String id;
  String displayId;
  String tag;
  NoteStatus status;
  String createdAt;
  String type;
  bool done = false;
  String dueAt;
};

namespace NoteStorage {
bool begin();
bool ensureLayout();
String newNoteId();
String utcNow();
String nextDisplayId();  // "#NNN" from NVS noteSeq
String noteDir(const String& id);
String metaPath(const String& id);
String audioPath(const String& id);
String transcriptPath(const String& id);

bool writeMeta(const NoteMeta& meta);
bool readMeta(const String& id, NoteMeta& out);
bool updateIndex(const NoteMeta& meta);
bool removeFromIndex(const String& id);
bool deleteNote(const String& id);
bool loadIndex(std::vector<IndexEntry>& out);
bool listPending(std::vector<String>& ids);
bool listNeedingOrganize(std::vector<String>& ids);
bool listReadyNotes(std::vector<NoteMeta>& out, int limit);
bool listByTag(const String& tag, std::vector<NoteMeta>& out, int limit);
int countByStatus(NoteStatus status);
int countOpenTasks();
String nextUpcomingDueAt();
bool hasPendingWork(bool organizeEnabled);
bool saveAskLog(const String& id, const String& question, const String& answer);
bool saveBriefingCache(const String& key, const String& text, const String& range);
}  // namespace NoteStorage
