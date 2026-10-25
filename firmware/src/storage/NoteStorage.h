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

struct NoteMeta {
  String id;
  String createdAt;
  String updatedAt;
  String audioPath;
  String transcript;
  NoteStatus status = NoteStatus::Pending;
  String error;
  uint32_t durationMs = 0;
  String type;  // note|task|appointment; empty => null in JSON
  String title;
  String dueAt;
  bool done = false;
  String organizedAt;
  String organizeError;
};

struct IndexEntry {
  String id;
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
String noteDir(const String& id);
String metaPath(const String& id);
String audioPath(const String& id);
String transcriptPath(const String& id);

bool writeMeta(const NoteMeta& meta);
bool readMeta(const String& id, NoteMeta& out);
bool updateIndex(const NoteMeta& meta);
bool loadIndex(std::vector<IndexEntry>& out);
bool listPending(std::vector<String>& ids);
bool listNeedingOrganize(std::vector<String>& ids);
int countByStatus(NoteStatus status);
int countOpenTasks();
String nextUpcomingDueAt();
bool hasPendingWork(bool organizeEnabled);
}  // namespace NoteStorage
