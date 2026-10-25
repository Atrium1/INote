#include "WifiSetup.h"

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

namespace {
bool setupMode = false;
WebServer setupServer(80);
DNSServer dns;
InoteConfig* cfgPtr = nullptr;

const char* kSetupHtml = R"HTML(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>INote Setup</title>
<style>body{font-family:sans-serif;max-width:28rem;margin:1.5rem auto;padding:0 1rem}
label{display:block;margin-top:.75rem}input{width:100%;padding:.5rem}button{margin-top:1rem;padding:.6rem 1rem}</style>
</head><body>
<h1>INote</h1>
<p>Wi-Fi ve OpenAI API anahtarını kaydedin.</p>
<form method="POST" action="/save">
<label>SSID <input name="ssid" required></label>
<label>Password <input name="password" type="password"></label>
<label>OpenAI API key <input name="apikey" required></label>
<label>Language <input name="lang" value="tr"></label>
<button type="submit">Save</button>
</form>
</body></html>
)HTML";

void handleRoot() { setupServer.send(200, "text/html", kSetupHtml); }

void handleSave() {
  if (!cfgPtr) {
    setupServer.send(500, "text/plain", "no config");
    return;
  }
  WifiCredential c;
  c.ssid = setupServer.arg("ssid");
  c.password = setupServer.arg("password");
  cfgPtr->openaiApiKey = setupServer.arg("apikey");
  const String lang = setupServer.arg("lang");
  if (lang.length()) cfgPtr->uiLanguage = lang;
  cfgPtr->sttLanguage = cfgPtr->uiLanguage;
  cfgPtr->wifi.clear();
  if (c.ssid.length()) cfgPtr->wifi.push_back(c);
  InoteConfigStore::save(*cfgPtr);
  setupServer.send(200, "text/html",
                   "<html><body><h1>Saved</h1><p>Rebooting…</p></body></html>");
  delay(500);
  ESP.restart();
}

void handleCaptive() { setupServer.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true); setupServer.send(302, "text/plain", ""); }
}  // namespace

namespace WifiSetup {

void begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("inote");
}

void startSetupPortal(InoteConfig& cfg) {
  cfgPtr = &cfg;
  setupMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("INote-Setup");
  dns.start(53, "*", WiFi.softAPIP());
  setupServer.on("/", HTTP_GET, handleRoot);
  setupServer.on("/save", HTTP_POST, handleSave);
  setupServer.onNotFound(handleCaptive);
  setupServer.begin();
}

void stopSetupPortal() {
  if (!setupMode) return;
  setupServer.stop();
  dns.stop();
  WiFi.softAPdisconnect(true);
  setupMode = false;
}

bool isSetupMode() { return setupMode; }

bool connectSta(const InoteConfig& cfg) {
  if (!cfg.hasWifi()) return false;
  WiFi.mode(WIFI_STA);
  for (const auto& c : cfg.wifi) {
    WiFi.begin(c.ssid.c_str(), c.password.c_str());
    const uint32_t start = millis();
    while (millis() - start < 15000) {
      if (WiFi.status() == WL_CONNECTED) return true;
      delay(200);
    }
    WiFi.disconnect(true, true);
    delay(200);
  }
  return false;
}

bool isConnected() { return WiFi.status() == WL_CONNECTED; }

void radioOff() {
  if (setupMode) stopSetupPortal();
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  delay(50);
}

void loop() {
  if (!setupMode) return;
  dns.processNextRequest();
  setupServer.handleClient();
}

String staIp() {
  if (!isConnected()) return "";
  return WiFi.localIP().toString();
}

}  // namespace WifiSetup
