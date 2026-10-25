#include "DisplayUi.h"

#include "../../include/board_pins.h"
#include "../i18n/Strings.h"

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

static GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
    GxEPD2_154_D67(PIN_EPD_CS, PIN_EPD_DC, PIN_EPD_RST, PIN_EPD_BUSY));

namespace {
UiModel current;
UiModel lastDrawn;
bool ready = false;
uint32_t lastFullMs = 0;

const char* menuLabels[] = {"Notes", "Tags", "Sync", "Settings"};
const char* settingsLabels[] = {"Sound", "Transfer", "Device"};

void drawBattery(int x, int y, int pct) {
  display.drawRect(x, y, 22, 10, GxEPD_BLACK);
  display.fillRect(x + 22, y + 3, 2, 4, GxEPD_BLACK);
  if (pct >= 0) {
    int w = constrain(pct, 0, 100) * 18 / 100;
    display.fillRect(x + 2, y + 2, w, 6, GxEPD_BLACK);
  }
}

void drawStatusChrome() {
  if (current.batteryPercent >= 0) drawBattery(170, 8, current.batteryPercent);
  display.drawCircle(160, 12, 3, GxEPD_BLACK);
  if (current.wifiConnected) display.drawCircle(160, 12, 6, GxEPD_BLACK);
}

void line(int y, const String& t) {
  display.setCursor(8, y);
  display.print(t.substring(0, 22));
}

void paint() {
  const UiStrings s = stringsFor(current.language);
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);

    switch (current.screen) {
      case UiScreen::Record: {
        display.setCursor(8, 28);
        display.print(s.recording);
        display.setFont(&FreeMono9pt7b);
        char buf[16];
        snprintf(buf, sizeof(buf), "%lu s", (unsigned long)(current.recordingMs / 1000));
        line(60, buf);
        snprintf(buf, sizeof(buf), "marks: %d", current.markerCount);
        line(90, buf);
        if (current.markFlash) line(120, s.mark);
        line(180, "BOOT stop");
        break;
      }
      case UiScreen::Tag: {
        display.setCursor(8, 28);
        display.print("Tag");
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(8, 90);
        display.print(current.tagChoice);
        display.setFont(&FreeMono9pt7b);
        line(140, "BOOT cycle");
        line(165, "PWR confirm");
        break;
      }
      case UiScreen::Menu: {
        display.setCursor(8, 28);
        display.print("Menu");
        display.setFont(&FreeMono9pt7b);
        for (int i = 0; i < 4; ++i) {
          String row = (i == current.menuIndex ? "> " : "  ");
          row += menuLabels[i];
          line(56 + i * 22, row);
        }
        line(175, "PWR enter");
        break;
      }
      case UiScreen::Notes:
      case UiScreen::Tags: {
        display.setCursor(8, 28);
        display.print(current.listTitle.length() ? current.listTitle : "Notes");
        display.setFont(&FreeMono9pt7b);
        line(56, current.listLine1);
        line(80, current.listLine2);
        if (current.confirmDelete) {
          line(120, "Delete?");
          line(145, "PWR yes");
        } else {
          line(120, current.playing ? "Playing…" : current.listHint);
          line(160, "PWR play");
        }
        break;
      }
      case UiScreen::Sync: {
        display.setCursor(8, 28);
        display.print("Sync");
        display.setFont(&FreeMono9pt7b);
        if (current.wifiConnected) {
          line(60, "Wi-Fi ON");
          line(85, current.wifiSsid.substring(0, 18));
          line(110, current.portalIp);
          line(145, "PWR disconnect");
        } else {
          line(60, "Wi-Fi OFF");
          line(85, "PWR connect");
          if (current.pendingCount > 0) {
            char buf[24];
            snprintf(buf, sizeof(buf), "pending %d", current.pendingCount);
            line(120, buf);
          }
        }
        break;
      }
      case UiScreen::Settings: {
        display.setCursor(8, 28);
        display.print("Settings");
        display.setFont(&FreeMono9pt7b);
        for (int i = 0; i < 3; ++i) {
          String row = (i == current.settingsIndex ? "> " : "  ");
          row += settingsLabels[i];
          line(56 + i * 22, row);
        }
        line(140, current.listHint.length() ? current.listHint : "PWR toggle");
        break;
      }
      case UiScreen::Setup: {
        display.setCursor(8, 28);
        display.print(s.setupAp);
        display.setFont(&FreeMono9pt7b);
        line(60, "INote-Setup");
        break;
      }
      case UiScreen::Home:
      default: {
        display.setCursor(8, 28);
        display.print(s.idle);
        display.setFont(&FreeMono9pt7b);
        char buf[24];
        snprintf(buf, sizeof(buf), "%d %s", current.noteCount, s.notes);
        line(60, buf);
        line(85, current.wifiConnected ? s.wifiOk : s.wifiNo);
        int y = 110;
        if (current.pendingCount > 0) {
          snprintf(buf, sizeof(buf), "pending: %d", current.pendingCount);
          line(y, buf);
          y += 22;
        }
        if (current.openTasks > 0) {
          snprintf(buf, sizeof(buf), "%s: %d", s.tasks, current.openTasks);
          line(y, buf);
          y += 22;
        }
        if (current.nextDueDisplay.length()) {
          line(y, current.nextDueDisplay.substring(0, 18));
        }
        line(180, "BOOT rec");
        break;
      }
    }

    drawStatusChrome();
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
  const bool changed =
      force || current.screen != lastDrawn.screen || current.noteCount != lastDrawn.noteCount ||
      current.pendingCount != lastDrawn.pendingCount || current.openTasks != lastDrawn.openTasks ||
      current.markerCount != lastDrawn.markerCount || current.markFlash != lastDrawn.markFlash ||
      current.nextDueDisplay != lastDrawn.nextDueDisplay ||
      current.briefingHint != lastDrawn.briefingHint ||
      current.wifiConnected != lastDrawn.wifiConnected ||
      current.portalIp != lastDrawn.portalIp || current.tagChoice != lastDrawn.tagChoice ||
      current.menuIndex != lastDrawn.menuIndex || current.settingsIndex != lastDrawn.settingsIndex ||
      current.listLine1 != lastDrawn.listLine1 || current.listLine2 != lastDrawn.listLine2 ||
      current.listHint != lastDrawn.listHint || current.confirmDelete != lastDrawn.confirmDelete ||
      current.playing != lastDrawn.playing || current.batteryPercent != lastDrawn.batteryPercent ||
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
    display.setFont(&FreeMonoBold18pt7b);
    const char* label = text && text[0] ? text : "INote";
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((200 - (int)w) / 2, 110);
    display.print(label);
  } while (display.nextPage());
  delay(50);
}

}  // namespace DisplayUi
