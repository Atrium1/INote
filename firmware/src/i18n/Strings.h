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

// Device e-ink chrome (ASCII-safe for FreeSans).
struct DeviceUi {
  const char* today;
  const char* mon[12];
  const char* menu[4];
  const char* headerMenu;
  const char* headerSettings;
  const char* headerRecord;
  const char* headerTag;
  const char* headerSync;
  const char* headerSetup;
  const char* notesTitle;
  const char* tagsAll;
  const char* tagsPref;  // "etiket: " / "tags: "
  const char* metricNotes;
  const char* metricPending;
  const char* metricTasks;
  const char* metricMarks;
  const char* metricHint;
  const char* metricWifi;
  const char* metricSsid;
  const char* metricIp;
  const char* metricStatus;
  const char* metricAction;
  const char* on;
  const char* off;
  const char* wifiConnect;
  const char* wifiDisconnect;
  const char* bootStop;
  const char* empty;
  const char* deleteQ;
  const char* deleteHint;
  const char* playing;
  const char* holdBootMenu;
  const char* holdPwrDel;
  const char* pwrPlay;
  const char* bootNext;
  const char* setupLine1;
  const char* setupLine2;
  const char* setupLine3;
  const char* soundOn;
  const char* soundOff;
  const char* langPref;  // "Dil " / "Lang "
  const char* idleOff;
  const char* idlePref;  // "Bekleme " / "Idle "
  const char* idleUnit;  // "sn" / "s"
  const char* orgOn;
  const char* orgOff;
  const char* sdFail;
  const char* noApiKey;
  const char* noSsid;
  const char* wifiFail;
  const char* tag[5];  // note work idea buy private display labels
  const char* stRecording;
  const char* stPending;
  const char* stTranscribing;
  const char* stReady;
  const char* stError;
};

inline const DeviceUi& deviceUi(const String& lang) {
  static const DeviceUi en = {
      "TODAY",
      {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"},
      {"Notes", "Tags", "Sync", "Settings"},
      "menu",
      "settings",
      "record",
      "tag",
      "sync",
      "setup",
      "notes",
      "tags: all",
      "tags: ",
      "NOTES",
      "PENDING",
      "TASKS",
      "MARKS",
      "HINT",
      "Wi-Fi",
      "SSID",
      "IP",
      "STATUS",
      "ACTION",
      "ON",
      "OFF",
      "PWR connect",
      "PWR disconnect",
      "BOOT stop",
      "(empty)",
      "Delete?",
      "PWR yes / BOOT no",
      "Playing...",
      "hold BOOT menu",
      "hold PWR del",
      "PWR play",
      "BOOT next",
      "INote-Setup",
      "Open captive",
      "portal",
      "Sound ON",
      "Sound OFF",
      "Lang ",
      "Idle OFF",
      "Idle ",
      "s",
      "Org ON",
      "Org OFF",
      "SD fail",
      "No API key",
      "No SSID",
      "Wi-Fi fail",
      {"note", "work", "idea", "buy", "private"},
      "recording",
      "pending",
      "transcribing",
      "ready",
      "error",
  };
  static const DeviceUi de = {
      "HEUTE",
      {"JAN", "FEB", "MAR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ"},
      {"Notizen", "Tags", "Sync", "Einstell."},
      "menu",
      "einstell.",
      "aufnahme",
      "tag",
      "sync",
      "setup",
      "notizen",
      "tags: alle",
      "tags: ",
      "NOTIZEN",
      "OFFEN",
      "AUFGABEN",
      "MARKEN",
      "HINWEIS",
      "WLAN",
      "SSID",
      "IP",
      "STATUS",
      "AKTION",
      "AN",
      "AUS",
      "PWR verbinden",
      "PWR trennen",
      "BOOT stop",
      "(leer)",
      "Loschen?",
      "PWR ja / BOOT nein",
      "Spielt...",
      "BOOT halten Menü",
      "PWR halten del",
      "PWR play",
      "BOOT weiter",
      "INote-Setup",
      "Portal im",
      "Browser",
      "Ton AN",
      "Ton AUS",
      "Sprache ",
      "Idle AUS",
      "Idle ",
      "s",
      "Org AN",
      "Org AUS",
      "SD Fehler",
      "Kein API-Key",
      "Kein SSID",
      "WLAN Fehler",
      {"notiz", "arbeit", "idee", "kauf", "privat"},
      "aufnahme",
      "offen",
      "schrift",
      "fertig",
      "fehler",
  };
  static const DeviceUi tr = {
      "BUGUN",
      {"OCA", "SUB", "MAR", "NIS", "MAY", "HAZ", "TEM", "AGU", "EYL", "EKI", "KAS", "ARA"},
      {"Notlar", "Etiketler", "Senkron", "Ayarlar"},
      "menu",
      "ayarlar",
      "kayit",
      "etiket",
      "senkron",
      "kurulum",
      "notlar",
      "etiket: tumu",
      "etiket: ",
      "NOTLAR",
      "BEKLEYEN",
      "GOREVLER",
      "ISARET",
      "IPUCU",
      "Wi-Fi",
      "SSID",
      "IP",
      "DURUM",
      "EYLEM",
      "ACIK",
      "KAPALI",
      "PWR baglan",
      "PWR baglantiyi kes",
      "BOOT durdur",
      "(bos)",
      "Silinsin mi?",
      "PWR evet / BOOT hayir",
      "Oynatiliyor...",
      "BOOT basili menu",
      "PWR basili sil",
      "PWR oynat",
      "BOOT sonraki",
      "INote-Setup",
      "Tarayicida",
      "portal ac",
      "Ses ACIK",
      "Ses KAPALI",
      "Dil ",
      "Bekleme KAPALI",
      "Bekleme ",
      "sn",
      "Duzen ACIK",
      "Duzen KAPALI",
      "SD hata",
      "API anahtari yok",
      "SSID yok",
      "Wi-Fi hata",
      {"not", "is", "fikir", "al", "ozel"},
      "kayit",
      "bekliyor",
      "yaziliyor",
      "hazir",
      "hata",
  };
  if (lang == "en") return en;
  if (lang == "de") return de;
  return tr;
}

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
