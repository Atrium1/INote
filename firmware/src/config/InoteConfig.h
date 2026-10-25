#pragma once

#include <Arduino.h>
#include <vector>

struct WifiCredential {
  String ssid;
  String password;
};

struct InoteConfig {
  std::vector<WifiCredential> wifi;
  String openaiApiKey;
  String sttProvider = "openai";
  String sttModel = "gpt-transcribe";
  String sttLanguage = "tr";
  bool organize = false;
  String organizeModel = "gpt-4o-mini";
  bool autoTranscribe = true;
  bool ask = false;
  bool briefing = false;
  String askModel = "gpt-4o-mini";
  String briefingModel = "gpt-4o-mini";

  String uiLanguage = "tr";
  String dateFormat = "DMY";  // DMY | YMD | MDY
  String timeFormat = "H24";  // H24 | H12

  uint32_t idleSleepSeconds = 45;
  String sleepFace = "INote";
  bool wifiOffWhenIdle = true;
  bool shortPwrSleeps = true;

  bool soundEnabled = true;  // gates on-device WAV playback / PA

  bool hasWifi() const { return !wifi.empty() && wifi[0].ssid.length() > 0; }
  bool hasApiKey() const { return openaiApiKey.length() > 8; }
};

namespace InoteConfigStore {
bool begin();
InoteConfig load();
bool save(const InoteConfig& cfg);
bool loadSdOverride(InoteConfig& cfg);
}  // namespace InoteConfigStore
