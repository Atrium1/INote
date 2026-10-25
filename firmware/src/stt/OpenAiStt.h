#pragma once

#include <Arduino.h>

namespace OpenAiStt {
bool transcribeFile(const String& wavPath, const String& apiKey, const String& model,
                    const String& language, String& outText, String& outError);
}
