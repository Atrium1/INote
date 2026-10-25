#include "DisplayUi.h"

#include "../../include/board_pins.h"
#include "../i18n/Strings.h"
#include "../storage/NoteStorage.h"

#include <SPI.h>
#include <time.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>

static GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
    GxEPD2_154_D67(PIN_EPD_CS, PIN_EPD_DC, PIN_EPD_RST, PIN_EPD_BUSY));

namespace {
UiModel current;
UiModel lastDrawn;
bool ready = false;
uint32_t lastFullMs = 0;

constexpr int kMargin = 10;
constexpr int kRuleY = 32;
constexpr int kChipR = 5;
constexpr int kChipH = 28;
constexpr int kChipGap = 4;
constexpr int kChipStartY = 40;
constexpr int kTagChipH = 24;
constexpr int kTagChipGap = 2;
constexpr int kMaxChars = 16;

void drawWifiArc(int cx, int cy, int r) {
  display.drawCircle(cx, cy, r, GxEPD_BLACK);
  display.fillRect(cx - r - 1, cy + 1, 2 * r + 3, r + 2, GxEPD_WHITE);
}

void drawWifiIcon(int x, int y, bool connected) {
  const int cx = x + 9;
  const int cy = y + 11;
  display.fillCircle(cx, cy, 1, GxEPD_BLACK);
  if (connected) {
    drawWifiArc(cx, cy, 4);
    drawWifiArc(cx, cy, 7);
    drawWifiArc(cx, cy, 10);
  } else {
    drawWifiArc(cx, cy, 7);
    display.drawLine(cx - 5, cy - 8, cx + 5, cy - 2, GxEPD_BLACK);
  }
}

void drawBatteryIcon(int x, int y, int pct) {
  display.drawRoundRect(x, y, 24, 12, 2, GxEPD_BLACK);
  display.fillRoundRect(x + 24, y + 3, 3, 6, 1, GxEPD_BLACK);
  if (pct < 0) return;
  const int level = constrain(pct, 0, 100);
  int fillW = level * 20 / 100;
  if (fillW <= 0 && level > 0) fillW = 1;
  if (fillW > 0) display.fillRoundRect(x + 2, y + 2, fillW, 8, 1, GxEPD_BLACK);
  if (level > 0 && level < 15 && fillW < 4) {
    display.drawLine(x + 4, y + 10, x + 18, y + 1, GxEPD_BLACK);
  }
}

void drawStatusChrome() {
  drawWifiIcon(136, 3, current.wifiConnected);
  drawBatteryIcon(160, 4, current.batteryPercent);
}

void headerBar(const char* titleLower) {
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSansBold9pt7b);
  display.setCursor(kMargin, 22);
  display.print(titleLower);
  display.drawFastHLine(kMargin, kRuleY, 200 - 2 * kMargin, GxEPD_BLACK);
  drawStatusChrome();
}

void headerBrand() {
  headerBar("INote");
}

void printTrunc(int x, int y, const String& t, int maxChars = kMaxChars) {
  display.setCursor(x, y);
  display.print(t.substring(0, maxChars));
}

void chipAt(int y, int h, const char* label, bool selected) {
  const int w = 200 - 2 * kMargin;
  if (selected) {
    display.fillRoundRect(kMargin, y, w, h, kChipR, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
  } else {
    display.drawRoundRect(kMargin, y, w, h, kChipR, GxEPD_BLACK);
    display.setTextColor(GxEPD_BLACK);
  }
  display.setFont(&FreeSansBold9pt7b);
  display.setCursor(kMargin + 10, y + h - 7);
  display.print(String(label).substring(0, kMaxChars));
  display.setTextColor(GxEPD_BLACK);
}

void menuChip(int index, const char* label, bool selected) {
  chipAt(kChipStartY + index * (kChipH + kChipGap), kChipH, label, selected);
}

void tagChip(int index, const char* label, bool selected) {
  chipAt(kChipStartY + index * (kTagChipH + kTagChipGap), kTagChipH, label, selected);
}

void metricRow(int y, const char* label, const String& value) {
  display.drawFastHLine(kMargin, y, 200 - 2 * kMargin, GxEPD_BLACK);
  display.setFont(&FreeSansBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(kMargin, y + 16);
  display.print(label);
  int16_t x1, y1;
  uint16_t tw, th;
  display.getTextBounds(value.c_str(), 0, 0, &x1, &y1, &tw, &th);
  display.setCursor(200 - kMargin - (int)tw, y + 16);
  display.print(value);
}

void contentBox(int y, int h) {
  display.drawRoundRect(kMargin, y, 200 - 2 * kMargin, h, kChipR, GxEPD_BLACK);
}

const DeviceUi& dui() { return deviceUi(current.language); }

bool clockOk(struct tm& t) {
  time_t now = time(nullptr);
  if (now < 100000) return false;
  gmtime_r(&now, &t);
  return true;
}

String shortDateLine() {
  struct tm t;
  if (!clockOk(t)) return "";
  const DeviceUi& d = dui();
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d %s", t.tm_mday, d.mon[constrain(t.tm_mon, 0, 11)]);
  return String(buf);
}

const char* tagLabelUi(int index) {
  const DeviceUi& d = dui();
  if (index < 0 || index >= kTagCycleCount) return d.tag[0];
  return d.tag[index];
}

void bodyLineIn(int x, int y, const String& t) {
  display.setFont(&FreeSansBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  printTrunc(x, y, t);
}

void footerHint(const char* t) {
  display.setFont(&FreeSansBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  printTrunc(kMargin, 192, String(t), 22);
}

int tagIndexOf(const String& tag) {
  for (int i = 0; i < kTagCycleCount; ++i) {
    if (tag == kTagCycle[i]) return i;
  }
  return 0;
}

void paint() {
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    const DeviceUi& d = dui();

    switch (current.screen) {
      case UiScreen::Home: {
        headerBrand();
        const String date = shortDateLine();
        if (date.length()) {
          display.setFont(&FreeSansBold9pt7b);
          printTrunc(kMargin, 52, date);
        }
        display.setFont(&FreeSansBold18pt7b);
        printTrunc(kMargin, date.length() ? 82 : 70, String(d.today), 12);

        int y = 100;
        metricRow(y, d.metricNotes, String(current.noteCount));
        y += 28;
        metricRow(y, d.metricPending, String(current.pendingCount));
        y += 28;
        metricRow(y, d.metricTasks, String(current.openTasks));
        break;
      }
      case UiScreen::Menu: {
        headerBar(d.headerMenu);
        for (int i = 0; i < 4; ++i) menuChip(i, d.menu[i], i == current.menuIndex);
        break;
      }
      case UiScreen::Settings: {
        headerBar(d.headerSettings);
        for (int i = 0; i < 4; ++i) {
          const char* lab = current.settingsLabels[i].length() ? current.settingsLabels[i].c_str()
                                                              : "?";
          menuChip(i, lab, i == current.settingsIndex);
        }
        break;
      }
      case UiScreen::Record: {
        headerBar(d.headerRecord);
        char buf[16];
        snprintf(buf, sizeof(buf), "%lu %s", (unsigned long)(current.recordingMs / 1000), d.idleUnit);
        display.setFont(&FreeSansBold18pt7b);
        printTrunc(kMargin, 78, String(buf), 10);
        metricRow(100, d.metricMarks, String(current.markerCount));
        if (current.markFlash) metricRow(128, d.metricMarks, String("+"));
        footerHint(d.bootStop);
        break;
      }
      case UiScreen::Tag: {
        headerBar(d.headerTag);
        const int sel = tagIndexOf(current.tagChoice);
        for (int i = 0; i < kTagCycleCount; ++i) {
          tagChip(i, tagLabelUi(i), i == sel);
        }
        break;
      }
      case UiScreen::Notes:
      case UiScreen::Tags: {
        String ht = current.listTitle.length() ? current.listTitle : d.notesTitle;
        ht.toLowerCase();
        headerBar(ht.c_str());
        contentBox(40, 72);
        bodyLineIn(kMargin + 8, 62, current.listLine1.length() ? current.listLine1 : d.empty);
        if (current.listLine2.length()) bodyLineIn(kMargin + 8, 84, current.listLine2);
        if (current.confirmDelete) {
          contentBox(120, 56);
          bodyLineIn(kMargin + 8, 142, d.deleteQ);
          bodyLineIn(kMargin + 8, 164, d.deleteHint);
        } else {
          metricRow(124, d.metricHint, current.playing ? String(d.playing) : current.listHint);
          footerHint(current.screen == UiScreen::Notes ? d.holdPwrDel : d.pwrPlay);
        }
        break;
      }
      case UiScreen::Sync: {
        headerBar(d.headerSync);
        if (current.wifiConnected) {
          metricRow(48, d.metricWifi, String(d.on));
          metricRow(76, d.metricSsid, current.wifiSsid.length() ? current.wifiSsid : "-");
          metricRow(104, d.metricIp, current.portalIp.length() ? current.portalIp : "-");
          if (current.statusLine.length()) {
            metricRow(132, d.metricStatus, current.statusLine.substring(0, 12));
          }
          footerHint(d.wifiDisconnect);
        } else {
          metricRow(48, d.metricWifi, String(d.off));
          metricRow(76, d.metricAction, String(d.wifiConnect));
          if (current.statusLine.length()) {
            metricRow(104, d.metricStatus, current.statusLine.substring(0, 12));
          }
          if (current.pendingCount > 0) {
            metricRow(132, d.metricPending, String(current.pendingCount));
          }
        }
        break;
      }
      case UiScreen::Setup: {
        headerBar(d.headerSetup);
        contentBox(40, 80);
        bodyLineIn(kMargin + 8, 68, d.setupLine1);
        bodyLineIn(kMargin + 8, 92, d.setupLine2);
        bodyLineIn(kMargin + 8, 112, d.setupLine3);
        break;
      }
      default: {
        headerBrand();
        break;
      }
    }
  } while (display.nextPage());
  lastDrawn = current;
  lastFullMs = millis();
}
}  // namespace

namespace DisplayUi {

void panelPower(bool on) {
  pinMode(PIN_EPD_PWR, OUTPUT);
  digitalWrite(PIN_EPD_PWR, on ? LOW : HIGH);
}

bool begin() {
  panelPower(true);
  delay(50);
  SPI.begin(PIN_EPD_SCK, -1, PIN_EPD_MOSI, PIN_EPD_CS);
  display.init(115200, true, 50, false);
  ready = true;
  current = {};
  paint();
  return true;
}

void setModel(const UiModel& model) { current = model; }

void render(bool force) {
  if (!ready) return;
  const bool settingsChanged = current.settingsLabels[0] != lastDrawn.settingsLabels[0] ||
                               current.settingsLabels[1] != lastDrawn.settingsLabels[1] ||
                               current.settingsLabels[2] != lastDrawn.settingsLabels[2] ||
                               current.settingsLabels[3] != lastDrawn.settingsLabels[3];
  const bool changed =
      force || current.screen != lastDrawn.screen || current.language != lastDrawn.language ||
      current.noteCount != lastDrawn.noteCount || current.pendingCount != lastDrawn.pendingCount ||
      current.openTasks != lastDrawn.openTasks || current.errorCount != lastDrawn.errorCount ||
      current.markerCount != lastDrawn.markerCount || current.markFlash != lastDrawn.markFlash ||
      current.nextDueDisplay != lastDrawn.nextDueDisplay ||
      current.briefingHint != lastDrawn.briefingHint || current.statusLine != lastDrawn.statusLine ||
      current.wifiConnected != lastDrawn.wifiConnected || current.portalIp != lastDrawn.portalIp ||
      current.wifiSsid != lastDrawn.wifiSsid || current.tagChoice != lastDrawn.tagChoice ||
      current.menuIndex != lastDrawn.menuIndex || current.settingsIndex != lastDrawn.settingsIndex ||
      current.settingsValue != lastDrawn.settingsValue || settingsChanged ||
      current.listLine1 != lastDrawn.listLine1 || current.listLine2 != lastDrawn.listLine2 ||
      current.listHint != lastDrawn.listHint || current.listTitle != lastDrawn.listTitle ||
      current.confirmDelete != lastDrawn.confirmDelete || current.playing != lastDrawn.playing ||
      current.sdOk != lastDrawn.sdOk || current.batteryPercent != lastDrawn.batteryPercent ||
      (current.screen == UiScreen::Record &&
       (current.recordingMs / 1000) != (lastDrawn.recordingMs / 1000)) ||
      current.errorMessage != lastDrawn.errorMessage;

  if (!changed) return;
  if (!force && millis() - lastFullMs < 800 && current.screen != UiScreen::Record) return;
  paint();
}

void drawSleepFace(const char* text) {
  panelPower(true);
  delay(20);
  if (!ready) {
    SPI.begin(PIN_EPD_SCK, -1, PIN_EPD_MOSI, PIN_EPD_CS);
    display.init(115200, true, 50, false);
    ready = true;
  }
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSansBold18pt7b);
    const char* label = text && text[0] ? text : "INote";
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((200 - (int)w) / 2, 112);
    display.print(label);
  } while (display.nextPage());
  delay(50);
}

}  // namespace DisplayUi
