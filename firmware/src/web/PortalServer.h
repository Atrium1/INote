#pragma once

#include "../config/InoteConfig.h"

namespace PortalServer {
void begin();
void loop();
void setConfig(const InoteConfig* cfg);
}  // namespace PortalServer
