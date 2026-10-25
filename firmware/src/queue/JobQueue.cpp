#include "JobQueue.h"

#include "../storage/NoteStorage.h"
#include "../stt/OpenAiStt.h"
#include "../stt/OpenAiOrganize.h"

namespace {
bool busy = false;
uint32_t lastTryMs = 0;

void runOrganize(NoteMeta& meta, const InoteConfig& cfg) {
  if (!cfg.organize) return;
  if (meta.organizedAt.length()) return;
  if (!meta.transcript.length()) return;

  OpenAiOrganize::Result r;
  String err;
  const bool ok = OpenAiOrganize::classify(meta.transcript, NoteStorage::utcNow(), cfg.openaiApiKey,
                                           cfg.organizeModel, r, err);
  meta.updatedAt = NoteStorage::utcNow();
  if (ok) {
    meta.type = r.type;
    meta.title = r.title;
    meta.dueAt = r.dueAt;
    meta.done = r.done;
    meta.organizedAt = NoteStorage::utcNow();
    meta.organizeError = "";
  } else {
    meta.organizeError = err;
  }
  NoteStorage::writeMeta(meta);
}
}  // namespace

namespace JobQueue {

void begin() {
  busy = false;
  lastTryMs = 0;
}

bool isBusy() { return busy; }

void tick(const InoteConfig& cfg, bool wifiConnected) {
  if (!wifiConnected || !cfg.hasApiKey()) return;
  if (busy) return;
  if (millis() - lastTryMs < 5000) return;
  lastTryMs = millis();

  // Prefer STT pending first
  if (cfg.autoTranscribe) {
    std::vector<String> pending;
    if (NoteStorage::listPending(pending) && !pending.empty()) {
      const String id = pending.back();
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
        NoteStorage::writeMeta(meta);
        runOrganize(meta, cfg);
      } else {
        meta.status = NoteStatus::Error;
        meta.error = err;
        NoteStorage::writeMeta(meta);
      }
      busy = false;
      return;
    }
  }

  // Backfill organize for ready notes
  if (!cfg.organize) return;
  std::vector<String> need;
  if (!NoteStorage::listNeedingOrganize(need) || need.empty()) return;

  const String id = need.back();
  NoteMeta meta;
  if (!NoteStorage::readMeta(id, meta)) return;
  busy = true;
  runOrganize(meta, cfg);
  busy = false;
}

}  // namespace JobQueue
