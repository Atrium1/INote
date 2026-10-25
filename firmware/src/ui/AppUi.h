#pragma once

#include "../config/InoteConfig.h"

namespace AppUi {
void begin(InoteConfig& cfg, bool sdOk);
void loop();
bool isBusy();  // recording / playing / setup — blocks idle sleep helpers
}  // namespace AppUi
