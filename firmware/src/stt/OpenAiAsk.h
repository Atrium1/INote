#pragma once

#include <Arduino.h>
#include <vector>
#include "../storage/NoteStorage.h"

namespace OpenAiAsk {
bool ask(const String& question, const std::vector<NoteMeta>& context, const String& apiKey,
         const String& model, String& outAnswer, String& outError);

bool briefing(const String& range, const std::vector<NoteMeta>& context, const String& apiKey,
              const String& model, String& outText, String& outError);
}  // namespace OpenAiAsk
