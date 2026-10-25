#include "Power.h"

#include "../../include/board_pins.h"

#include <esp_sleep.h>

namespace {
bool bootDown = false;
bool pwrDown = false;
uint32_t bootDownAt = 0;
uint32_t pwrDownAt = 0;
bool bootShort = false;
bool bootMark = false;
bool bootLong = false;
bool pwrShort = false;
bool pwrLong = false;
bool bootMarkFired = false;
bool bootLongFired = false;
bool pwrLongFired = false;
uint32_t lastActivityMs = 0;
}

namespace Power {

void begin() {
  pinMode(PIN_BAT_LATCH, OUTPUT);
  holdBatteryLatch(true);
  pinMode(PIN_BOOT_BTN, INPUT_PULLUP);
  pinMode(PIN_PWR_BTN, INPUT_PULLUP);
  analogReadResolution(12);
  lastActivityMs = millis();
}

void holdBatteryLatch(bool on) { digitalWrite(PIN_BAT_LATCH, on ? HIGH : LOW); }

void noteActivity() { lastActivityMs = millis(); }

uint32_t idleMs() { return millis() - lastActivityMs; }

bool shouldIdleSleep(uint32_t timeoutSec) {
  if (timeoutSec == 0) return false;
  return idleMs() >= timeoutSec * 1000UL;
}

int batteryPercent() {
  const int raw = analogRead(PIN_BAT_ADC);
  const float vAdc = (raw / 4095.0f) * 3.3f;
  const float vBat = vAdc * 2.0f;
  if (vBat < 3.2f) return 0;
  if (vBat > 4.15f) return 100;
  return (int)((vBat - 3.2f) / (4.15f - 3.2f) * 100.0f);
}

bool bootPressed() { return digitalRead(PIN_BOOT_BTN) == LOW; }
bool pwrPressed() { return digitalRead(PIN_PWR_BTN) == LOW; }

void updateButtons() {
  bootShort = false;
  bootMark = false;
  bootLong = false;
  pwrShort = false;
  pwrLong = false;

  const bool b = bootPressed();
  if (b && !bootDown) {
    bootDown = true;
    bootDownAt = millis();
    bootMarkFired = false;
    bootLongFired = false;
    noteActivity();
  } else if (b && bootDown) {
    const uint32_t held = millis() - bootDownAt;
    if (!bootMarkFired && held >= 1000) {
      bootMark = true;
      bootMarkFired = true;
      noteActivity();
    }
    if (!bootLongFired && held >= 3000) {
      bootLong = true;
      bootLongFired = true;
      noteActivity();
    }
  } else if (!b && bootDown) {
    if (!bootMarkFired && !bootLongFired && millis() - bootDownAt > 40) {
      bootShort = true;
      noteActivity();
    }
    bootDown = false;
  }

  const bool p = pwrPressed();
  if (p && !pwrDown) {
    pwrDown = true;
    pwrDownAt = millis();
    pwrLongFired = false;
    noteActivity();
  } else if (p && pwrDown && !pwrLongFired && millis() - pwrDownAt > 2000) {
    pwrLong = true;
    pwrLongFired = true;
    noteActivity();
  } else if (!p && pwrDown) {
    if (!pwrLongFired && millis() - pwrDownAt > 40 && millis() - pwrDownAt < 2000) {
      pwrShort = true;
      noteActivity();
    }
    pwrDown = false;
  }
}

bool bootShortClick() { return bootShort; }
bool bootMarkPress() { return bootMark; }
bool bootLongPress() { return bootLong; }
bool pwrShortClick() { return pwrShort; }
bool pwrLongPress() { return pwrLong; }

void enterDeepSleep() {
  const uint64_t mask = (1ULL << PIN_BOOT_BTN) | (1ULL << PIN_PWR_BTN);
  esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ANY_LOW);
  esp_deep_sleep_start();
}

void softPowerOff() {
  holdBatteryLatch(false);
  delay(100);
  enterDeepSleep();
}

}  // namespace Power
