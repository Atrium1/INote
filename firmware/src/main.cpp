#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include <board_pins.h>
#include "config/InoteConfig.h"
#include "storage/NoteStorage.h"
#include "display/DisplayUi.h"
#include "power/Power.h"
#include "wifi_setup/WifiSetup.h"
#include "audio/AudioCapture.h"
#include "ui/AppUi.h"

namespace {
InoteConfig gCfg;
SPIClass sdSpi(HSPI);
bool sdOk = false;
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nINote boot");

  Power::begin();
  InoteConfigStore::begin();
  sdSpi.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  sdOk = SD.begin(PIN_SD_CS, sdSpi);
  Serial.printf("[sd] %s (SPI CS=%d; Waveshare demo uses SDMMC CLK/CMD/D0)\n", sdOk ? "ok" : "FAIL",
                PIN_SD_CS);
  if (sdOk) NoteStorage::begin();

  gCfg = InoteConfigStore::load();
  DisplayUi::begin();
  AudioCapture::begin();
  WifiSetup::begin();
  AppUi::begin(gCfg, sdOk);
}

void loop() { AppUi::loop(); }
