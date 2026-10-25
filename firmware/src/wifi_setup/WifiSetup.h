#pragma once

#include "../config/InoteConfig.h"

namespace WifiSetup {
void begin();
void startSetupPortal(InoteConfig& cfg);
void stopSetupPortal();
bool isSetupMode();
bool connectSta(const InoteConfig& cfg);
bool isConnected();
void disconnectSta();  // leave STA mode, drop association
void radioOff();
void loop();  // handle SoftAP web clients
String staIp();
String staSsid();
}  // namespace WifiSetup
