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
  const char* tasks;
  const char* mark;
  const char* briefingOk;
};

inline UiStrings stringsFor(const String& lang) {
  if (lang == "en") {
    return {"INote", "Recording…", "Syncing…", "Error", "Setup Wi-Fi", "notes", "Wi-Fi", "No Wi-Fi",
            "tasks", "+mark", "Briefing ready"};
  }
  if (lang == "de") {
    return {"INote", "Aufnahme…", "Sync…", "Fehler", "WLAN einrichten", "Notizen", "WLAN", "Kein WLAN",
            "Aufgaben", "+Mark", "Briefing bereit"};
  }
  return {"INote", "Kaydediliyor…", "Senkron…", "Hata", "Wi-Fi kur", "not", "Wi-Fi", "Wi-Fi yok",
          "görev", "+işaret", "Briefing hazır"};
}

inline String formatDateTimeDisplay(const String& iso, const String& dateFmt, const String& timeFmt) {
  if (iso.length() < 16) return iso;
  // ISO: YYYY-MM-DDTHH:MM:SSZ
  const String y = iso.substring(0, 4);
  const String mo = iso.substring(5, 7);
  const String d = iso.substring(8, 10);
  int hour = iso.substring(11, 13).toInt();
  const String mi = iso.substring(14, 16);
  String datePart;
  if (dateFmt == "YMD") datePart = y + "-" + mo + "-" + d;
  else if (dateFmt == "MDY") datePart = mo + "/" + d + "/" + y;
  else datePart = d + "." + mo + "." + y;

  String timePart;
  if (timeFmt == "H12") {
    const char* ampm = hour >= 12 ? "PM" : "AM";
    int h12 = hour % 12;
    if (h12 == 0) h12 = 12;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%s %s", h12, mi.c_str(), ampm);
    timePart = buf;
  } else {
    timePart = iso.substring(11, 16);
  }
  return datePart + " " + timePart;
}
