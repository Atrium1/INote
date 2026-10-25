#pragma once

#include <Arduino.h>

struct UiStrings {
  const char* idle;
  const char* recording;
  const char* syncing;
  const char* error;
  const char* setupAp;
  const char* notes;
  const char* wifiOk;
  const char* wifiNo;
};

inline UiStrings stringsFor(const String& lang) {
  if (lang == "en") {
    return {"INote", "Recording…", "Syncing…", "Error", "Setup Wi-Fi", "notes", "Wi-Fi", "No Wi-Fi"};
  }
  return {"INote", "Kaydediliyor…", "Senkron…", "Hata", "Wi-Fi kur", "not", "Wi-Fi", "Wi-Fi yok"};
}
