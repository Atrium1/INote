#include "DisplayUi.h"

#include "../../include/board_pins.h"
#include "../i18n/Strings.h"

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

// GxEPD2_154_D67 — 200x200 SSD1681 / GDEY0154D67 class
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
      if (current.pendingCount > 0) {
        display.setCursor(8, 120);
        display.printf("pending: %d", current.pendingCount);
      }
    }

    if (current.batteryPercent >= 0) {
      drawBattery(170, 8, current.batteryPercent);
    }
    // Wi-Fi glyph
    display.drawCircle(160, 12, 3, GxEPD_BLACK);
    if (current.wifiConnected) {
      display.drawCircle(160, 12, 6, GxEPD_BLACK);
    }
  } while (display.nextPage());
  lastDrawn = current;
  lastFullMs = millis();
}
}  // namespace

namespace DisplayUi {

bool begin() {
  pinMode(PIN_EPD_PWR, OUTPUT);
  digitalWrite(PIN_EPD_PWR, LOW);  // enable panel
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
      current.pendingCount != lastDrawn.pendingCount || current.wifiConnected != lastDrawn.wifiConnected ||
      current.batteryPercent != lastDrawn.batteryPercent ||
      (current.screen == UiScreen::Recording && (current.recordingMs / 1000) != (lastDrawn.recordingMs / 1000)) ||
      current.errorMessage != lastDrawn.errorMessage;

  if (!changed) return;
  // Avoid hammering full refresh; recording updates at most ~1 Hz via second change
  if (!force && millis() - lastFullMs < 800 && current.screen != UiScreen::Recording) return;
  paint();
}

}  // namespace DisplayUi
