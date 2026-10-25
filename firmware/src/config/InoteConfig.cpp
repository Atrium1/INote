#include "InoteConfig.h"

#include <Preferences.h>
#include <SD.h>
#include <ArduinoJson.h>

namespace {
Preferences prefs;
const char* kNs = "inote";

void applyJson(InoteConfig& cfg, JsonDocument& doc) {
  if (doc["openaiApiKey"].is<const char*>()) {
    cfg.openaiApiKey = doc["openaiApiKey"].as<String>();
  }
  if (doc["stt"]["provider"].is<const char*>()) {
    cfg.sttProvider = doc["stt"]["provider"].as<String>();
  }
  if (doc["stt"]["model"].is<const char*>()) {
    cfg.sttModel = doc["stt"]["model"].as<String>();
  }
  if (doc["stt"]["language"].is<const char*>()) {
    cfg.sttLanguage = doc["stt"]["language"].as<String>();
  }
  if (!doc["features"]["organize"].isNull()) {
    cfg.organize = doc["features"]["organize"].as<bool>();
  }
  if (!doc["features"]["autoTranscribe"].isNull()) {
    cfg.autoTranscribe = doc["features"]["autoTranscribe"].as<bool>();
  }
  if (doc["ui"]["language"].is<const char*>()) {
    cfg.uiLanguage = doc["ui"]["language"].as<String>();
  }
  if (doc["wifi"].is<JsonArray>()) {
    cfg.wifi.clear();
    for (JsonObject o : doc["wifi"].as<JsonArray>()) {
      WifiCredential c;
      c.ssid = o["ssid"] | "";
      c.password = o["password"] | "";
      if (c.ssid.length()) cfg.wifi.push_back(c);
      if (cfg.wifi.size() >= 5) break;
    }
  }
}
}  // namespace

namespace InoteConfigStore {

bool begin() {
  return prefs.begin(kNs, false);
}

InoteConfig load() {
  InoteConfig cfg;
  cfg.openaiApiKey = prefs.getString("apiKey", "");
  cfg.sttModel = prefs.getString("sttModel", cfg.sttModel);
  cfg.sttLanguage = prefs.getString("sttLang", cfg.sttLanguage);
  cfg.uiLanguage = prefs.getString("uiLang", cfg.uiLanguage);
  cfg.organize = prefs.getBool("organize", false);
  cfg.autoTranscribe = prefs.getBool("autoStt", true);

  const int n = prefs.getInt("wifiN", 0);
  for (int i = 0; i < n && i < 5; ++i) {
    WifiCredential c;
    c.ssid = prefs.getString(("ssid" + String(i)).c_str(), "");
    c.password = prefs.getString(("pass" + String(i)).c_str(), "");
    if (c.ssid.length()) cfg.wifi.push_back(c);
  }

  loadSdOverride(cfg);
  return cfg;
}

bool save(const InoteConfig& cfg) {
  prefs.putString("apiKey", cfg.openaiApiKey);
  prefs.putString("sttModel", cfg.sttModel);
  prefs.putString("sttLang", cfg.sttLanguage);
  prefs.putString("uiLang", cfg.uiLanguage);
  prefs.putBool("organize", cfg.organize);
  prefs.putBool("autoStt", cfg.autoTranscribe);

  const int n = min((int)cfg.wifi.size(), 5);
  prefs.putInt("wifiN", n);
  for (int i = 0; i < n; ++i) {
    prefs.putString(("ssid" + String(i)).c_str(), cfg.wifi[i].ssid);
    prefs.putString(("pass" + String(i)).c_str(), cfg.wifi[i].password);
  }
  return true;
}

bool loadSdOverride(InoteConfig& cfg) {
  if (!SD.exists("/inote/config.json")) return false;
  File f = SD.open("/inote/config.json", FILE_READ);
  if (!f) return false;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;
  applyJson(cfg, doc);
  return true;
}

}  // namespace InoteConfigStore
