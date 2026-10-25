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

void drawBattery(int x, int y, int pct) {
  display.drawRect(x, y, 22, 10, GxEPD_BLACK);
  display.fillRect(x + 22, y + 3, 2, 4, GxEPD_BLACK);
  if (pct >= 0) {
    int w = constrain(pct, 0, 100) * 18 / 100;
    display.fillRect(x + 2, y + 2, w, 6, GxEPD_BLACK);
  }
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

    if (current.screen == UiScreen::Recording) {
      display.setCursor(8, 28);
      display.print(s.recording);
      display.setFont(&FreeMono9pt7b);
      display.setCursor(8, 60);
      char buf[16];
      snprintf(buf, sizeof(buf), "%lu s", (unsigned long)(current.recordingMs / 1000));
      display.print(buf);
      display.setCursor(8, 90);
      display.printf("marks: %d", current.markerCount);
      if (current.markFlash) {
        display.setCursor(8, 120);
        display.print(s.mark);
      }
    } else if (current.screen == UiScreen::Syncing) {
      display.setCursor(8, 28);
      display.print(s.syncing);
      display.setFont(&FreeMono9pt7b);
      display.setCursor(8, 60);
      display.printf("%d pending", current.pendingCount);
    } else if (current.screen == UiScreen::Error) {
      display.setCursor(8, 28);
      display.print(s.error);
      display.setFont(&FreeMono9pt7b);
      display.setCursor(8, 56);
      display.print(current.errorMessage.substring(0, 40));
    } else if (current.screen == UiScreen::Setup) {
      display.setCursor(8, 28);
      display.print(s.setupAp);
      display.setFont(&FreeMono9pt7b);
      display.setCursor(8, 60);
      display.print("INote-Setup");
    } else {
      display.setCursor(8, 28);
      display.print(s.idle);
      display.setFont(&FreeMono9pt7b);
      display.setCursor(8, 60);
      display.printf("%d %s", current.noteCount, s.notes);
      display.setCursor(8, 90);
      display.print(current.wifiConnected ? s.wifiOk : s.wifiNo);
      int y = 120;
      if (current.pendingCount > 0) {
        display.setCursor(8, y);
        display.printf("pending: %d", current.pendingCount);
        y += 22;
      }
      if (current.openTasks > 0) {
        display.setCursor(8, y);
        display.printf("%s: %d", s.tasks, current.openTasks);
        y += 22;
      }
      if (current.nextDueDisplay.length()) {
        display.setCursor(8, y);
        display.print(current.nextDueDisplay.substring(0, 18));
        y += 22;
      }
      if (current.briefingHint.length()) {
        display.setCursor(8, y);
        display.print(current.briefingHint.substring(0, 18));
      }
    }

    if (current.batteryPercent >= 0) drawBattery(170, 8, current.batteryPercent);
    display.drawCircle(160, 12, 3, GxEPD_BLACK);
    if (current.wifiConnected) display.drawCircle(160, 12, 6, GxEPD_BLACK);
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
      current.batteryPercent != lastDrawn.batteryPercent ||
      (current.screen == UiScreen::Recording &&
       (current.recordingMs / 1000) != (lastDrawn.recordingMs / 1000)) ||
      current.errorMessage != lastDrawn.errorMessage;

  if (!changed) return;
  if (!force && millis() - lastFullMs < 800 && current.screen != UiScreen::Recording) return;
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
