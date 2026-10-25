#include "AudioCapture.h"

#include "WavWriter.h"
#include "../../include/board_pins.h"

#include <Wire.h>
#include <driver/i2s_std.h>
#include <string.h>

// Hardware bring-up notes (see docs/bringup.md):
// - Confirm ES8311 ACK at 0x18 on SDA/SCL 47/48
// - PA pins stay off in v0.1
// - If codec init fails, INOTE_AUDIO_STUB path writes silence so the rest of the pipeline can be tested

namespace {
bool recording = false;
uint32_t startedMs = 0;
WavWriter wav;
i2s_chan_handle_t rxChan = nullptr;
bool i2sReady = false;
bool codecReady = false;

static const uint8_t ES8311_ADDR = 0x18;

bool es8311Write(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ES8311_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool es8311Probe() {
  Wire.beginTransmission(ES8311_ADDR);
  return Wire.endTransmission() == 0;
}

// Minimal register poke — replace/expand with Waveshare ES8311 sequence on bring-up
bool es8311MinimalInit() {
  if (!es8311Probe()) return false;
  // Reset / clock / ADC path — conservative placeholders from common ES8311 bring-up patterns
  es8311Write(0x00, 0x80);
  delay(10);
  es8311Write(0x00, 0x00);
  es8311Write(0x01, 0x30);
  es8311Write(0x02, 0x10);
  es8311Write(0x16, 0x24);
  es8311Write(0x04, 0x10);
  es8311Write(0x05, 0x00);
  es8311Write(0x06, 0x00);
  es8311Write(0x07, 0x00);
  es8311Write(0x08, 0xFF);
  es8311Write(0x09, 0x0C);
  es8311Write(0x0A, 0x0C);
  es8311Write(0x0B, 0x00);
  es8311Write(0x0C, 0x00);
  es8311Write(0x10, 0x03);
  es8311Write(0x11, 0x7F);
  es8311Write(0x12, 0x00);
  es8311Write(0x13, 0x10);
  es8311Write(0x14, 0x10);
  es8311Write(0x17, 0xC8);
  return true;
}

bool initI2sRx() {
  if (rxChan) return true;
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;
  if (i2s_new_channel(&chan_cfg, nullptr, &rxChan) != ESP_OK) return false;

  i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(INOTE_SAMPLE_RATE),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
      .gpio_cfg =
          {
              .mclk = (gpio_num_t)PIN_I2S_MCLK,
              .bclk = (gpio_num_t)PIN_I2S_SCLK,
              .ws = (gpio_num_t)PIN_I2S_LRCK,
              .dout = I2S_GPIO_UNUSED,
              .din = (gpio_num_t)PIN_I2S_ASDOUT,
              .invert_flags = {.mclk_inv = false, .bclk_inv = false, .ws_inv = false},
          },
  };
  std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;
  if (i2s_channel_init_std_mode(rxChan, &std_cfg) != ESP_OK) return false;
  i2sReady = true;
  return true;
}
}  // namespace

namespace AudioCapture {

bool begin() {
  pinMode(PIN_PA_EN, OUTPUT);
  pinMode(PIN_PA_CTRL, OUTPUT);
  setPaEnabled(false);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  codecReady = es8311MinimalInit();
  i2sReady = initI2sRx();
  Serial.printf("[audio] codec=%d i2s=%d\n", (int)codecReady, (int)i2sReady);
  return true;  // always succeed so device boots; recording may be stub
}

void setPaEnabled(bool on) {
  // Active levels depend on board; keep both low/off for v0.1
  digitalWrite(PIN_PA_EN, on ? HIGH : LOW);
  digitalWrite(PIN_PA_CTRL, LOW);
}

bool startRecording(const String& wavPath) {
  if (recording) return false;
  if (!wav.open(wavPath, INOTE_SAMPLE_RATE, INOTE_SAMPLE_BITS, INOTE_CHANNELS)) {
    return false;
  }
  if (i2sReady && rxChan) {
    i2s_channel_enable(rxChan);
  }
  recording = true;
  startedMs = millis();
  return true;
}

bool isRecording() { return recording; }

uint32_t recordingElapsedMs() {
  if (!recording) return 0;
  return millis() - startedMs;
}

void poll() {
  if (!recording) return;
  int16_t buf[256];
  size_t bytesRead = 0;
  if (i2sReady && rxChan) {
    if (i2s_channel_read(rxChan, buf, sizeof(buf), &bytesRead, 50) == ESP_OK && bytesRead > 0) {
      wav.writeSamples((uint8_t*)buf, bytesRead);
      return;
    }
  }
  // Stub: write short silence chunk so file grows when codec/I2S not ready yet
  memset(buf, 0, sizeof(buf));
  wav.writeSamples((uint8_t*)buf, sizeof(buf));
  delay(10);
}

bool stopRecording(uint32_t& outDurationMs) {
  outDurationMs = 0;
  if (!recording) return false;
  recording = false;
  outDurationMs = millis() - startedMs;
  if (i2sReady && rxChan) {
    i2s_channel_disable(rxChan);
  }
  return wav.close();
}

}  // namespace AudioCapture
