#pragma once

#include <Arduino.h>
#include "../storage/NoteStorage.h"

namespace OpenAiOrganize {
struct Result {
  String type;
  String title;
  String dueAt;
  bool done = false;
};

bool classify(const String& transcript, const String& nowUtc, const String& apiKey,
              const String& model, Result& out, String& outError);
}  // namespace OpenAiOrganize
