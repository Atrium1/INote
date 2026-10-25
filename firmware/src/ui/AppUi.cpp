#include "AppUi.h"

#include <SD.h>
#include <vector>
#include <time.h>

#include <board_pins.h>
#include "../storage/NoteStorage.h"
#include "../queue/JobQueue.h"
#include "../display/DisplayUi.h"
#include "../power/Power.h"
#include "../wifi_setup/WifiSetup.h"
#include "../web/PortalServer.h"
#include "../audio/AudioCapture.h"
#include "../i18n/Strings.h"

namespace {
InoteConfig* cfg = nullptr;
bool sdReady = false;
bool portalStarted = false;

enum class State {
  Home,
  Record,
  Tag,
  Menu,
  Notes,
  Tags,
  Sync,
  Settings,
  Setup,
};

State state = State::Home;
String activeNoteId;
String pendingTag = "note";
int activeMarkerCount = 0;
bool markFlash = false;
uint32_t markFlashUntil = 0;
uint32_t lastUiMs = 0;

int menuIndex = 0;
int settingsIndex = 0;
int notesIndex = 0;
int tagsFilterIndex = 0;
int tagsNoteIndex = 0;
bool confirmDelete = false;
String syncHint;
uint32_t syncHintUntil = 0;

std::vector<IndexEntry> noteList;
std::vector<NoteMeta> tagNotes;

void syncTime() { configTime(0, 0, "pool.ntp.org", "time.nist.gov"); }

void persistConfig() {
  if (cfg) InoteConfigStore::save(*cfg);
}

void ensurePortal() {
  if (portalStarted) return;
  PortalServer::setConfig(cfg);
  PortalServer::setSdOk(sdReady);
  PortalServer::begin();
  portalStarted = true;
}

bool tryConnectWifi(bool force) {
  if (!cfg) return false;
  if (!cfg->hasWifi()) {
    if (force) {
      syncHint = "No SSID";
      syncHintUntil = millis() + 4000;
    }
    return false;
  }
  if (WifiSetup::isConnected()) {
    ensurePortal();
    return true;
  }
  if (!force && cfg->wifiOffWhenIdle && !NoteStorage::hasPendingWork(cfg->organize)) return false;
  if (WifiSetup::connectSta(*cfg)) {
    Serial.printf("[wifi] %s\n", WifiSetup::staIp().c_str());
    syncHint = "";
    syncTime();
    ensurePortal();
    return true;
  }
  if (force) {
    syncHint = "Wi-Fi fail";
    syncHintUntil = millis() + 4000;
  }
  return false;
}

void reloadNoteList() {
  noteList.clear();
  NoteStorage::loadIndex(noteList);
  if (notesIndex >= (int)noteList.size()) notesIndex = 0;
  if (notesIndex < 0) notesIndex = 0;
}

void reloadTagNotes() {
  tagNotes.clear();
  String tag;
  if (tagsFilterIndex > 0 && tagsFilterIndex <= kTagCycleCount) {
    tag = kTagCycle[tagsFilterIndex - 1];
  }
  NoteStorage::listByTag(tag, tagNotes, 64);
  if (tagsNoteIndex >= (int)tagNotes.size()) tagsNoteIndex = 0;
  if (tagsNoteIndex < 0) tagsNoteIndex = 0;
}

String settingsValueText() {
  if (!cfg) return "";
  switch (settingsIndex) {
    case 0:
      return cfg->soundEnabled ? "Sound ON" : "Sound OFF";
    case 1:
      return String("Lang ") + cfg->uiLanguage;
    case 2:
      if (cfg->idleSleepSeconds == 0) return "Idle OFF";
      return String("Idle ") + String(cfg->idleSleepSeconds) + "s";
    case 3:
      return cfg->organize ? "Org ON" : "Org OFF";
    default:
      return "";
  }
}

void cycleLanguage() {
  if (cfg->uiLanguage == "tr") cfg->uiLanguage = "en";
  else if (cfg->uiLanguage == "en") cfg->uiLanguage = "de";
  else cfg->uiLanguage = "tr";
  cfg->sttLanguage = cfg->uiLanguage;
}

void cycleIdleSleep() {
  const uint32_t cur = cfg->idleSleepSeconds;
  if (cur == 30) cfg->idleSleepSeconds = 45;
  else if (cur == 45) cfg->idleSleepSeconds = 90;
  else if (cur == 90) cfg->idleSleepSeconds = 0;
  else cfg->idleSleepSeconds = 30;
}

bool startNote() {
  if (!sdReady || state == State::Record) return false;
  Power::noteActivity();
  activeNoteId = NoteStorage::newNoteId();
  activeMarkerCount = 0;
  if (!SD.exists("/inote/notes")) SD.mkdir("/inote/notes");
  if (!SD.mkdir(NoteStorage::noteDir(activeNoteId))) return false;

  NoteMeta meta;
  meta.id = activeNoteId;
  meta.displayId = NoteStorage::nextDisplayId();
  meta.tag = "note";
  meta.createdAt = NoteStorage::utcNow();
  meta.updatedAt = meta.createdAt;
  meta.audioPath = "notes/" + activeNoteId + "/audio.wav";
  meta.status = NoteStatus::Recording;
  NoteStorage::writeMeta(meta);

  if (!AudioCapture::startRecording(NoteStorage::audioPath(activeNoteId))) {
    meta.status = NoteStatus::Error;
    meta.error = "audio start failed";
    meta.updatedAt = NoteStorage::utcNow();
    NoteStorage::writeMeta(meta);
    return false;
  }
  state = State::Record;
  return true;
}

bool stopNoteToTag() {
  if (state != State::Record) return false;
  uint32_t dur = 0;
  AudioCapture::stopRecording(dur);
  Power::noteActivity();
  NoteMeta meta;
  if (!NoteStorage::readMeta(activeNoteId, meta)) return false;
  meta.durationMs = dur;
  meta.updatedAt = NoteStorage::utcNow();
  meta.status = NoteStatus::Pending;
  meta.error = "";
  NoteStorage::writeMeta(meta);
  pendingTag = meta.tag.length() ? meta.tag : "note";
  state = State::Tag;
  activeMarkerCount = 0;
  return true;
}

bool confirmTag() {
  if (state != State::Tag || !activeNoteId.length()) return false;
  NoteMeta meta;
  if (!NoteStorage::readMeta(activeNoteId, meta)) return false;
  meta.tag = pendingTag;
  meta.updatedAt = NoteStorage::utcNow();
  NoteStorage::writeMeta(meta);
  activeNoteId = "";
  state = State::Home;
  tryConnectWifi(true);
  return true;
}

bool addMarker() {
  if (state != State::Record || !activeNoteId.length()) return false;
  NoteMeta meta;
  if (!NoteStorage::readMeta(activeNoteId, meta)) return false;
  NoteMarker m;
  m.tMs = AudioCapture::recordingElapsedMs();
  meta.markers.push_back(m);
  meta.updatedAt = NoteStorage::utcNow();
  NoteStorage::writeMeta(meta);
  activeMarkerCount = (int)meta.markers.size();
  markFlash = true;
  markFlashUntil = millis() + 1500;
  Power::noteActivity();
  return true;
}

void enterSetup() {
  if (state == State::Record) {
    uint32_t dur = 0;
    AudioCapture::stopRecording(dur);
  }
  AudioCapture::stopPlayback();
  Power::noteActivity();
  WifiSetup::startSetupPortal(*cfg);
  state = State::Setup;
}

bool canSleep() {
  if (state == State::Record || state == State::Tag) return false;
  if (AudioCapture::isPlaying()) return false;
  if (JobQueue::isBusy() || PortalServer::isBusy()) return false;
  if (WifiSetup::isSetupMode() || state == State::Setup) return false;
  return true;
}

void enterSleepSequence(bool latchOff) {
  if (!canSleep()) return;
  if (cfg->wifiOffWhenIdle || latchOff) WifiSetup::radioOff();
  AudioCapture::stopPlayback();
  AudioCapture::setPaEnabled(false);
  DisplayUi::drawSleepFace(cfg->sleepFace.c_str());
  DisplayUi::panelPower(false);
  delay(100);
  if (latchOff) Power::softPowerOff();
  else Power::enterDeepSleep();
}

String homeStatusLine() {
  if (!sdReady) return "SD fail";
  if (!cfg->hasApiKey() && NoteStorage::countByStatus(NoteStatus::Pending) > 0) return "No API key";
  if (!cfg->hasApiKey()) return "No API key";
  return "";
}

void refreshUi(bool force = false) {
  UiModel m;
  m.language = cfg->uiLanguage;
  m.wifiConnected = WifiSetup::isConnected();
  m.wifiSsid = WifiSetup::staSsid();
  m.portalIp = WifiSetup::staIp();
  m.batteryPercent = Power::batteryPercent();
  m.pendingCount = NoteStorage::countByStatus(NoteStatus::Pending);
  m.errorCount = NoteStorage::countByStatus(NoteStatus::Error);
  m.openTasks = NoteStorage::countOpenTasks();
  m.nextDueAt = NoteStorage::nextUpcomingDueAt();
  m.nextDueDisplay =
      formatDateTimeDisplay(m.nextDueAt, cfg->dateFormat, cfg->timeFormat);
  m.briefingHint = PortalServer::lastBriefingHint();
  m.markerCount = activeMarkerCount;
  m.markFlash = markFlash && millis() < markFlashUntil;
  if (!m.markFlash) markFlash = false;
  m.menuIndex = menuIndex;
  m.settingsIndex = settingsIndex;
  m.settingsValue = settingsValueText();
  m.tagChoice = pendingTag;
  m.confirmDelete = confirmDelete;
  m.playing = AudioCapture::isPlaying();
  m.sdOk = sdReady;

  if (syncHint.length() && millis() > syncHintUntil) syncHint = "";

  std::vector<IndexEntry> idx;
  if (NoteStorage::loadIndex(idx)) m.noteCount = (int)idx.size();

  if (WifiSetup::isSetupMode() || state == State::Setup) {
    m.screen = UiScreen::Setup;
  } else {
    switch (state) {
      case State::Record:
        m.screen = UiScreen::Record;
        m.recordingMs = AudioCapture::recordingElapsedMs();
        break;
      case State::Tag:
        m.screen = UiScreen::Tag;
        break;
      case State::Menu:
        m.screen = UiScreen::Menu;
        break;
      case State::Notes: {
        m.screen = UiScreen::Notes;
        m.listTitle = "Notes";
        if (noteList.empty()) {
          m.listLine1 = "(empty)";
          m.listLine2 = "";
          m.listHint = "hold BOOT menu";
        } else {
          const auto& e = noteList[notesIndex];
          m.listLine1 = e.displayId.length() ? e.displayId : e.id.substring(0, 12);
          m.listLine2 = e.tag + " · " + noteStatusToString(e.status);
          if (e.status == NoteStatus::Error) {
            NoteMeta meta;
            if (NoteStorage::readMeta(e.id, meta) && meta.error.length()) {
              m.listLine2 = meta.error.substring(0, 20);
            }
          }
          m.listHint = "hold BOOT menu";
        }
        break;
      }
      case State::Tags: {
        m.screen = UiScreen::Tags;
        if (tagsFilterIndex == 0) m.listTitle = "Tags: all";
        else m.listTitle = String("Tags: ") + kTagCycle[tagsFilterIndex - 1];
        if (tagNotes.empty()) {
          m.listLine1 = "(empty)";
          m.listLine2 = "";
          m.listHint = "BOOT next";
        } else {
          const auto& n = tagNotes[tagsNoteIndex];
          m.listLine1 = n.displayId.length() ? n.displayId : n.id.substring(0, 12);
          m.listLine2 = n.tag;
          if (n.status == NoteStatus::Error && n.error.length()) {
            m.listLine2 = n.error.substring(0, 20);
          }
          m.listHint = "PWR play";
        }
        break;
      }
      case State::Sync:
        m.screen = UiScreen::Sync;
        m.statusLine = syncHint;
        break;
      case State::Settings:
        m.screen = UiScreen::Settings;
        break;
      case State::Home:
      default:
        m.screen = UiScreen::Home;
        m.statusLine = homeStatusLine();
        break;
    }
  }

  DisplayUi::setModel(m);
  DisplayUi::render(force);
}

void playSelectedNote(const String& id) {
  if (!cfg->soundEnabled) {
    refreshUi(true);
    return;
  }
  if (AudioCapture::isPlaying()) {
    AudioCapture::stopPlayback();
    refreshUi(true);
    return;
  }
  if (!id.length()) return;
  if (AudioCapture::startPlayback(NoteStorage::audioPath(id))) {
    refreshUi(true);
  }
}

void handleHome() {
  if (Power::bootShortClick()) {
    if (!sdReady) {
      refreshUi(true);
      return;
    }
    if (startNote()) refreshUi(true);
  }
  if (Power::bootMarkPress()) {
    state = State::Menu;
    menuIndex = 0;
    refreshUi(true);
  }
  if (Power::bootLongPress()) enterSetup();
  if (Power::pwrShortClick()) {
    if (cfg->shortPwrSleeps && canSleep()) enterSleepSequence(false);
  }
  if (Power::pwrLongPress() && canSleep()) enterSleepSequence(true);
}

void handleRecord() {
  if (Power::bootShortClick()) {
    stopNoteToTag();
    refreshUi(true);
  }
  if (Power::bootMarkPress()) {
    addMarker();
    refreshUi(true);
  }
}

void handleTag() {
  if (Power::bootShortClick()) {
    pendingTag = nextTag(pendingTag);
    refreshUi(true);
  }
  if (Power::pwrShortClick()) {
    confirmTag();
    refreshUi(true);
  }
}

void handleMenu() {
  if (Power::bootShortClick()) {
    menuIndex = (menuIndex + 1) % 4;
    refreshUi(true);
  }
  if (Power::pwrShortClick()) {
    if (menuIndex == 0) {
      state = State::Notes;
      reloadNoteList();
      notesIndex = 0;
      confirmDelete = false;
    } else if (menuIndex == 1) {
      state = State::Tags;
      tagsFilterIndex = 0;
      tagsNoteIndex = 0;
      reloadTagNotes();
    } else if (menuIndex == 2) {
      state = State::Sync;
      syncHint = "";
    } else {
      state = State::Settings;
      settingsIndex = 0;
    }
    refreshUi(true);
  }
  if (Power::bootMarkPress()) {
    state = State::Home;
    refreshUi(true);
  }
}

void handleNotes() {
  if (confirmDelete) {
    if (Power::pwrShortClick() && !noteList.empty()) {
      NoteStorage::deleteNote(noteList[notesIndex].id);
      AudioCapture::stopPlayback();
      confirmDelete = false;
      reloadNoteList();
      refreshUi(true);
    }
    if (Power::bootShortClick()) {
      confirmDelete = false;
      refreshUi(true);
    }
    if (Power::bootMarkPress()) {
      confirmDelete = false;
      AudioCapture::stopPlayback();
      state = State::Menu;
      refreshUi(true);
    }
    return;
  }
  if (Power::bootShortClick()) {
    if (!noteList.empty()) {
      notesIndex = (notesIndex + 1) % (int)noteList.size();
      refreshUi(true);
    }
  }
  if (Power::pwrShortClick() && !noteList.empty()) {
    playSelectedNote(noteList[notesIndex].id);
  }
  if (Power::pwrLongPress() && !noteList.empty()) {
    confirmDelete = true;
    refreshUi(true);
  }
  if (Power::bootMarkPress()) {
    AudioCapture::stopPlayback();
    confirmDelete = false;
    state = State::Menu;
    refreshUi(true);
  }
}

void handleTags() {
  if (Power::bootShortClick()) {
    if (tagNotes.empty() || tagsNoteIndex + 1 >= (int)tagNotes.size()) {
      tagsFilterIndex = (tagsFilterIndex + 1) % (kTagCycleCount + 1);
      tagsNoteIndex = 0;
      reloadTagNotes();
    } else {
      tagsNoteIndex++;
    }
    refreshUi(true);
  }
  if (Power::pwrShortClick() && !tagNotes.empty()) {
    playSelectedNote(tagNotes[tagsNoteIndex].id);
  }
  if (Power::bootMarkPress()) {
    AudioCapture::stopPlayback();
    state = State::Menu;
    refreshUi(true);
  }
}

void handleSync() {
  if (Power::pwrShortClick()) {
    if (WifiSetup::isConnected()) {
      WifiSetup::disconnectSta();
      if (cfg->wifiOffWhenIdle) WifiSetup::radioOff();
      syncHint = "";
    } else {
      tryConnectWifi(true);
    }
    refreshUi(true);
  }
  if (Power::bootMarkPress()) {
    state = State::Menu;
    refreshUi(true);
  }
}

void handleSettings() {
  if (Power::bootShortClick()) {
    settingsIndex = (settingsIndex + 1) % 4;
    refreshUi(true);
  }
  if (Power::pwrShortClick()) {
    if (settingsIndex == 0) {
      cfg->soundEnabled = !cfg->soundEnabled;
      if (!cfg->soundEnabled) AudioCapture::stopPlayback();
    } else if (settingsIndex == 1) {
      cycleLanguage();
    } else if (settingsIndex == 2) {
      cycleIdleSleep();
    } else if (settingsIndex == 3) {
      cfg->organize = !cfg->organize;
    }
    persistConfig();
    refreshUi(true);
  }
  if (Power::bootMarkPress()) {
    state = State::Menu;
    refreshUi(true);
  }
}

void handleSetup() {}
}  // namespace

namespace AppUi {

void begin(InoteConfig& c, bool sdOk) {
  cfg = &c;
  sdReady = sdOk;
  state = State::Home;
  JobQueue::begin();
  PortalServer::setSdOk(sdReady);
  Power::noteActivity();
  if (!cfg->hasWifi()) enterSetup();
  else tryConnectWifi(!cfg->wifiOffWhenIdle || NoteStorage::hasPendingWork(cfg->organize));
  refreshUi(true);
}

bool isBusy() {
  return state == State::Record || state == State::Tag || state == State::Setup ||
         AudioCapture::isPlaying() || WifiSetup::isSetupMode();
}

void loop() {
  Power::updateButtons();
  WifiSetup::loop();
  if (portalStarted) PortalServer::loop();

  if (AudioCapture::isPlaying()) AudioCapture::pollPlayback();
  if (state == State::Record) AudioCapture::poll();

  if (WifiSetup::isSetupMode()) {
    state = State::Setup;
  } else if (state == State::Setup) {
    state = State::Home;
  }

  switch (state) {
    case State::Home:
      handleHome();
      break;
    case State::Record:
      handleRecord();
      break;
    case State::Tag:
      handleTag();
      break;
    case State::Menu:
      handleMenu();
      break;
    case State::Notes:
      handleNotes();
      break;
    case State::Tags:
      handleTags();
      break;
    case State::Sync:
      handleSync();
      break;
    case State::Settings:
      handleSettings();
      break;
    case State::Setup:
      handleSetup();
      break;
  }

  if (state != State::Record && state != State::Setup && Power::bootLongPress()) {
    enterSetup();
    refreshUi(true);
  }

  if (!WifiSetup::isSetupMode() && state != State::Sync) {
    if (WifiSetup::isConnected()) {
      JobQueue::tick(*cfg, true);
      ensurePortal();
    } else if (NoteStorage::hasPendingWork(cfg->organize)) {
      tryConnectWifi(false);
    }
  } else if (state == State::Sync && WifiSetup::isConnected()) {
    JobQueue::tick(*cfg, true);
  }

  if (canSleep() && state == State::Home && Power::shouldIdleSleep(cfg->idleSleepSeconds)) {
    enterSleepSequence(false);
  }

  if (millis() - lastUiMs > 1000) {
    lastUiMs = millis();
    refreshUi(false);
  }
}

}  // namespace AppUi
