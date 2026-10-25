#include "Power.h"

#include "../../include/board_pins.h"

namespace {
bool bootDown = false;
bool pwrDown = false;
uint32_t bootDownAt = 0;
uint32_t pwrDownAt = 0;
bool bootShort = false;
bool bootLong = false;
bool pwrShort = false;
bool bootLongFired = false;
}

namespace Power {

void begin() {
  pinMode(PIN_BAT_LATCH, OUTPUT);
  holdBatteryLatch(true);
  pinMode(PIN_BOOT_BTN, INPUT_PULLUP);
  pinMode(PIN_PWR_BTN, INPUT_PULLUP);
  analogReadResolution(12);
}

void holdBatteryLatch(bool on) { digitalWrite(PIN_BAT_LATCH, on ? HIGH : LOW); }

int batteryPercent() {
  // Divider ~1:2 — rough mapping; calibrate on hardware
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
  bootLong = false;
  pwrShort = false;

  const bool b = bootPressed();
  if (b && !bootDown) {
    bootDown = true;
    bootDownAt = millis();
    bootLongFired = false;
  } else if (b && bootDown && !bootLongFired && millis() - bootDownAt > 3000) {
    bootLong = true;
    bootLongFired = true;
  } else if (!b && bootDown) {
    if (!bootLongFired && millis() - bootDownAt < 3000 && millis() - bootDownAt > 40) {
      bootShort = true;
    }
    bootDown = false;
  }

  const bool p = pwrPressed();
  if (p && !pwrDown) {
    pwrDown = true;
    pwrDownAt = millis();
  } else if (!p && pwrDown) {
    if (millis() - pwrDownAt > 40 && millis() - pwrDownAt < 2000) {
      pwrShort = true;
    }
    pwrDown = false;
  }
}

bool bootShortClick() { return bootShort; }
bool bootLongPress() { return bootLong; }
bool pwrShortClick() { return pwrShort; }

}  // namespace Power
