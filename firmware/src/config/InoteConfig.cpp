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
  if (!doc["features"]["ask"].isNull()) {
    cfg.ask = doc["features"]["ask"].as<bool>();
  }
  if (!doc["features"]["briefing"].isNull()) {
    cfg.briefing = doc["features"]["briefing"].as<bool>();
  }
  if (doc["organize"]["model"].is<const char*>()) {
    cfg.organizeModel = doc["organize"]["model"].as<String>();
  }
  if (doc["ask"]["model"].is<const char*>()) {
    cfg.askModel = doc["ask"]["model"].as<String>();
  }
  if (doc["briefing"]["model"].is<const char*>()) {
    cfg.briefingModel = doc["briefing"]["model"].as<String>();
  }
  if (doc["ui"]["language"].is<const char*>()) {
    cfg.uiLanguage = doc["ui"]["language"].as<String>();
  }
  if (doc["ui"]["dateFormat"].is<const char*>()) {
    cfg.dateFormat = doc["ui"]["dateFormat"].as<String>();
  }
  if (doc["ui"]["timeFormat"].is<const char*>()) {
    cfg.timeFormat = doc["ui"]["timeFormat"].as<String>();
  }
  if (!doc["power"]["idleSleepSeconds"].isNull()) {
    cfg.idleSleepSeconds = doc["power"]["idleSleepSeconds"].as<uint32_t>();
  }
  if (doc["power"]["sleepFace"].is<const char*>()) {
    cfg.sleepFace = doc["power"]["sleepFace"].as<String>();
  }
  if (!doc["power"]["wifiOffWhenIdle"].isNull()) {
    cfg.wifiOffWhenIdle = doc["power"]["wifiOffWhenIdle"].as<bool>();
  }
  if (!doc["power"]["shortPwrSleeps"].isNull()) {
    cfg.shortPwrSleeps = doc["power"]["shortPwrSleeps"].as<bool>();
  }
  if (!doc["audio"]["soundEnabled"].isNull()) {
    cfg.soundEnabled = doc["audio"]["soundEnabled"].as<bool>();
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
  cfg.dateFormat = prefs.getString("dateFmt", cfg.dateFormat);
  cfg.timeFormat = prefs.getString("timeFmt", cfg.timeFormat);
  cfg.organize = prefs.getBool("organize", false);
  cfg.organizeModel = prefs.getString("orgModel", cfg.organizeModel);
  cfg.autoTranscribe = prefs.getBool("autoStt", true);
  cfg.ask = prefs.getBool("featAsk", false);
  cfg.briefing = prefs.getBool("featBrief", false);
  cfg.askModel = prefs.getString("askModel", cfg.askModel);
  cfg.briefingModel = prefs.getString("briefModel", cfg.briefingModel);
  cfg.idleSleepSeconds = prefs.getUInt("idleSleep", cfg.idleSleepSeconds);
  cfg.sleepFace = prefs.getString("sleepFace", cfg.sleepFace);
  cfg.wifiOffWhenIdle = prefs.getBool("wifiOffIdle", true);
  cfg.shortPwrSleeps = prefs.getBool("shortPwrSlp", true);
  cfg.soundEnabled = prefs.getBool("soundEn", true);

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
  prefs.putString("dateFmt", cfg.dateFormat);
  prefs.putString("timeFmt", cfg.timeFormat);
  prefs.putBool("organize", cfg.organize);
  prefs.putString("orgModel", cfg.organizeModel);
  prefs.putBool("autoStt", cfg.autoTranscribe);
  prefs.putBool("featAsk", cfg.ask);
  prefs.putBool("featBrief", cfg.briefing);
  prefs.putString("askModel", cfg.askModel);
  prefs.putString("briefModel", cfg.briefingModel);
  prefs.putUInt("idleSleep", cfg.idleSleepSeconds);
  prefs.putString("sleepFace", cfg.sleepFace);
  prefs.putBool("wifiOffIdle", cfg.wifiOffWhenIdle);
  prefs.putBool("shortPwrSlp", cfg.shortPwrSleeps);
  prefs.putBool("soundEn", cfg.soundEnabled);

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
