#pragma once

#include "../config/InoteConfig.h"

namespace JobQueue {
void begin();
void tick(const InoteConfig& cfg, bool wifiConnected);
bool isBusy();
}  // namespace JobQueue
