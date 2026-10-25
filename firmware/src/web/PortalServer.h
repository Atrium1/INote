#pragma once

#include "../config/InoteConfig.h"

namespace PortalServer {
void begin();
void loop();
void setConfig(const InoteConfig* cfg);
void setSdOk(bool ok);
bool isBusy();  // ask/briefing in flight
String lastBriefingHint();
void clearBriefingHint();
}  // namespace PortalServer
