#pragma once

#include <Arduino.h>

namespace Power {
void begin();
void holdBatteryLatch(bool on);
int batteryPercent();
bool pwrPressed();
bool bootPressed();
void updateButtons();
bool bootShortClick();
bool bootLongPress();  // ~3s held
bool pwrShortClick();
bool pwrLongPress();   // ~2s held while awake

void noteActivity();
uint32_t idleMs();
bool shouldIdleSleep(uint32_t timeoutSec);

void enterDeepSleep();  // wake on BOOT + PWR (active low)
void softPowerOff();    // BAT latch low
}  // namespace Power
