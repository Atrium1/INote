#pragma once

#include <Arduino.h>

enum class UiScreen {
  Idle,
  Recording,
  Syncing,
  Error,
  Setup,
};

struct UiModel {
  UiScreen screen = UiScreen::Idle;
  int noteCount = 0;
  int pendingCount = 0;
  int openTasks = 0;
  String nextDueAt;
  bool wifiConnected = false;
  int batteryPercent = -1;
  uint32_t recordingMs = 0;
  String errorMessage;
  String language = "tr";
};

namespace DisplayUi {
bool begin();
void panelPower(bool on);  // false => EPD_PWR HIGH (off), image retained
void setModel(const UiModel& model);
void render(bool force = false);
void drawSleepFace(const char* text);
}  // namespace DisplayUi
