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
  bool autoTranscribe = true;
  String uiLanguage = "tr";

  bool hasWifi() const { return !wifi.empty() && wifi[0].ssid.length() > 0; }
  bool hasApiKey() const { return openaiApiKey.length() > 8; }
};

namespace InoteConfigStore {
bool begin();
InoteConfig load();
bool save(const InoteConfig& cfg);
bool loadSdOverride(InoteConfig& cfg);
}  // namespace InoteConfigStore
