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
}  // namespace Power
