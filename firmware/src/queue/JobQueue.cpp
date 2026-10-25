#include "JobQueue.h"

#include "../storage/NoteStorage.h"
#include "../stt/OpenAiStt.h"

namespace {
bool busy = false;
uint32_t lastTryMs = 0;
}

namespace JobQueue {

void begin() {
  busy = false;
  lastTryMs = 0;
}

bool isBusy() { return busy; }

void tick(const InoteConfig& cfg, bool wifiConnected) {
  if (!cfg.autoTranscribe || !wifiConnected || !cfg.hasApiKey()) return;
  if (busy) return;
  if (millis() - lastTryMs < 5000) return;
  lastTryMs = millis();

  std::vector<String> ids;
  if (!NoteStorage::listPending(ids) || ids.empty()) return;

  const String id = ids.back();  // oldest-ish if index append order; we take last pending
  NoteMeta meta;
  if (!NoteStorage::readMeta(id, meta)) return;

  busy = true;
  meta.status = NoteStatus::Transcribing;
  meta.updatedAt = NoteStorage::utcNow();
  meta.error = "";
  NoteStorage::writeMeta(meta);

  String transcript;
  String err;
  const bool ok = OpenAiStt::transcribeFile(
      NoteStorage::audioPath(id), cfg.openaiApiKey, cfg.sttModel, cfg.sttLanguage, transcript, err);

  meta.updatedAt = NoteStorage::utcNow();
  if (ok) {
    meta.transcript = transcript;
    meta.status = NoteStatus::Ready;
    meta.error = "";
  } else {
    meta.status = NoteStatus::Error;
    meta.error = err;
  }
  NoteStorage::writeMeta(meta);
  busy = false;
}

}  // namespace JobQueue
