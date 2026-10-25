#pragma once

#include <Arduino.h>

enum class UiScreen {
  Home,
  Record,
  Tag,
  Menu,
  Notes,
  Tags,
  Sync,
  Settings,
  Setup,
  SleepFace,
};

struct UiModel {
  UiScreen screen = UiScreen::Home;
  int noteCount = 0;
  int pendingCount = 0;
  int openTasks = 0;
  int errorCount = 0;
  int markerCount = 0;
  bool markFlash = false;
  String nextDueAt;
  String nextDueDisplay;
  String briefingHint;
  String statusLine;  // Sync short status / health
  bool wifiConnected = false;
  String wifiSsid;
  String portalIp;
  int batteryPercent = -1;
  uint32_t recordingMs = 0;
  String errorMessage;
  String language = "tr";
  String tagChoice = "note";
  int menuIndex = 0;      // 0 Notes 1 Tags 2 Sync 3 Settings
  int settingsIndex = 0;  // 0 Sound 1 Language 2 Idle 3 Organize
  String settingsValue;   // legacy; chips use settingsLabels
  String settingsLabels[4];
  String listTitle;
  String listLine1;
  String listLine2;
  String listHint;
  bool confirmDelete = false;
  bool playing = false;
  bool sdOk = true;
};

namespace DisplayUi {
bool begin();
void panelPower(bool on);
void setModel(const UiModel& model);
void render(bool force = false);
void drawSleepFace(const char* text);
}  // namespace DisplayUi
