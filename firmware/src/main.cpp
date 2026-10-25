#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <esp_sntp.h>

#include <board_pins.h>
#include "config/InoteConfig.h"
#include "storage/NoteStorage.h"
#include "queue/JobQueue.h"
#include "display/DisplayUi.h"
#include "power/Power.h"
#include "wifi_setup/WifiSetup.h"
#include "web/PortalServer.h"
#include "audio/AudioCapture.h"

namespace {
InoteConfig gCfg;
SPIClass sdSpi(HSPI);
bool sdOk = false;
bool recording = false;
String activeNoteId;
uint32_t lastUiMs = 0;
bool portalStarted = false;

void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

void refreshUi(bool force = false) {
  UiModel m;
  m.language = gCfg.uiLanguage;
  m.wifiConnected = WifiSetup::isConnected();
  m.batteryPercent = Power::batteryPercent();
  m.noteCount = 0;
  m.pendingCount = NoteStorage::countByStatus(NoteStatus::Pending);
  std::vector<IndexEntry> idx;
  if (NoteStorage::loadIndex(idx)) m.noteCount = (int)idx.size();

  if (WifiSetup::isSetupMode()) {
    m.screen = UiScreen::Setup;
  } else if (recording) {
    m.screen = UiScreen::Recording;
    m.recordingMs = AudioCapture::recordingElapsedMs();
  } else if (JobQueue::isBusy()) {
    m.screen = UiScreen::Syncing;
  } else {
    m.screen = UiScreen::Idle;
  }
  DisplayUi::setModel(m);
  DisplayUi::render(force);
}

bool startNote() {
  if (!sdOk || recording) return false;
  activeNoteId = NoteStorage::newNoteId();
  const String dir = NoteStorage::noteDir(activeNoteId);
  if (!SD.exists("/inote/notes")) SD.mkdir("/inote/notes");
  if (!SD.mkdir(dir)) return false;

  NoteMeta meta;
  meta.id = activeNoteId;
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
  recording = true;
  return true;
}

bool stopNote() {
  if (!recording) return false;
  uint32_t dur = 0;
  AudioCapture::stopRecording(dur);
  recording = false;

  NoteMeta meta;
  if (!NoteStorage::readMeta(activeNoteId, meta)) return false;
  meta.durationMs = dur;
  meta.updatedAt = NoteStorage::utcNow();
  meta.status = NoteStatus::Pending;
  meta.error = "";
  NoteStorage::writeMeta(meta);
  activeNoteId = "";
  return true;
}

void enterSetup() {
  if (recording) stopNote();
  WifiSetup::startSetupPortal(gCfg);
  refreshUi(true);
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nINote boot");

  Power::begin();
  InoteConfigStore::begin();

  sdSpi.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  sdOk = SD.begin(PIN_SD_CS, sdSpi);
  Serial.printf("[sd] %s\n", sdOk ? "ok" : "FAIL");
  if (sdOk) {
    NoteStorage::begin();
  }

  gCfg = InoteConfigStore::load();
  DisplayUi::begin();
  AudioCapture::begin();
  WifiSetup::begin();
  JobQueue::begin();

  if (!gCfg.hasWifi()) {
    enterSetup();
  } else if (WifiSetup::connectSta(gCfg)) {
    Serial.printf("[wifi] %s\n", WifiSetup::staIp().c_str());
    syncTime();
    PortalServer::setConfig(&gCfg);
    PortalServer::begin();
    portalStarted = true;
  } else {
    Serial.println("[wifi] connect failed — long-press BOOT for setup");
  }

  refreshUi(true);
}

void loop() {
  Power::updateButtons();
  WifiSetup::loop();

  if (portalStarted) PortalServer::loop();

  if (Power::bootLongPress()) {
    enterSetup();
  }

  if (Power::bootShortClick()) {
    if (WifiSetup::isSetupMode()) {
      // ignore capture while in setup
    } else if (recording) {
      stopNote();
      refreshUi(true);
    } else {
      if (startNote()) refreshUi(true);
    }
  }

  if (Power::pwrShortClick() && !recording) {
    // Light sleep placeholder — full power-off needs BAT latch dance on battery
    Serial.println("[pwr] short press (sleep TODO)");
  }

  if (recording) {
    AudioCapture::poll();
  }

  if (!WifiSetup::isSetupMode() && WifiSetup::isConnected()) {
    JobQueue::tick(gCfg, true);
    if (!portalStarted) {
      PortalServer::setConfig(&gCfg);
      PortalServer::begin();
      portalStarted = true;
      syncTime();
    }
  }

  if (millis() - lastUiMs > 1000) {
    lastUiMs = millis();
    refreshUi(false);
  }
}
