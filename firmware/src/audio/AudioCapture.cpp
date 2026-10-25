#include "AudioCapture.h"

#include "WavWriter.h"
#include "../../include/board_pins.h"

#include <SD.h>
#include <Wire.h>
#include <driver/i2s_std.h>
#include <string.h>

// Codec / I2S patterns adapted from Waveshare ESP32-S3-ePaper-1.54
// 02_Example/Arduino/08_Audio_Test (board_cfg S3_ePaper_1_54 + ES8311).
// Register sequence is a compact ADC+DAC slave+MCLK bring-up (esp_codec_dev-style).

namespace {
bool recording = false;
bool playing = false;
uint32_t startedMs = 0;
WavWriter wav;
File playFile;
uint32_t playDataBytes = 0;
uint32_t playDataPos = 0;
uint16_t playBits = 16;
uint16_t playChannels = 1;
uint32_t playRate = INOTE_SAMPLE_RATE;

i2s_chan_handle_t rxChan = nullptr;
i2s_chan_handle_t txChan = nullptr;
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

// Stronger init: reset, clocks for 16 kHz + MCLK×256, ADC+DAC paths, unmute.
bool es8311Init() {
  if (!es8311Probe()) return false;

  es8311Write(0x00, 0x1F);
  delay(20);
  es8311Write(0x00, 0x00);
  delay(10);

  // Clock manager — MCLK from pin, slave mode
  es8311Write(0x01, 0x30);
  es8311Write(0x02, 0x10);
  es8311Write(0x03, 0x10);
  es8311Write(0x16, 0x24);  // SDP: I2S 16-bit
  es8311Write(0x04, 0x10);
  es8311Write(0x05, 0x00);
  // 16 kHz @ MCLK 256×fs → common divider set from Waveshare/esp_codec tables
  es8311Write(0x06, 0x03);  // pre-div / mult for 12.288 MHz-class / 16k
  es8311Write(0x07, 0x00);
  es8311Write(0x08, 0xFF);
  es8311Write(0x09, 0x0C);
  es8311Write(0x0A, 0x0C);

  // System / analog power
  es8311Write(0x0B, 0x00);
  es8311Write(0x0C, 0x00);
  es8311Write(0x0D, 0x01);  // power up analog
  es8311Write(0x0E, 0x02);
  es8311Write(0x0F, 0x44);
  es8311Write(0x10, 0x08);  // VMID
  es8311Write(0x11, 0x7F);

  // ADC
  es8311Write(0x12, 0x28);  // ADC gain
  es8311Write(0x13, 0x00);
  es8311Write(0x14, 0x1A);  // ADC volume-ish
  es8311Write(0x15, 0x40);
  es8311Write(0x17, 0xC8);  // ADC enable / HPF
  es8311Write(0x1C, 0x0F);
  es8311Write(0x1D, 0x0F);

  // DAC / HP (speaker path via PA)
  es8311Write(0x31, 0x00);
  es8311Write(0x32, 0x00);
  es8311Write(0x37, 0x08);
  es8311Write(0x44, 0x00);  // DAC volume
  es8311Write(0x45, 0x00);
  es8311Write(0x18, 0x00);  // unmute DAC path helpers
  es8311Write(0x19, 0x00);
  es8311Write(0x1A, 0x00);
  es8311Write(0x1B, 0x00);

  return true;
}

bool initI2sDuplex() {
  if (rxChan && txChan) return true;
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;
  if (i2s_new_channel(&chan_cfg, &txChan, &rxChan) != ESP_OK) return false;

  i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(INOTE_SAMPLE_RATE),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
      .gpio_cfg =
          {
              .mclk = (gpio_num_t)PIN_I2S_MCLK,
              .bclk = (gpio_num_t)PIN_I2S_SCLK,
              .ws = (gpio_num_t)PIN_I2S_LRCK,
              .dout = (gpio_num_t)PIN_I2S_DSDIN,
              .din = (gpio_num_t)PIN_I2S_ASDOUT,
              .invert_flags = {.mclk_inv = false, .bclk_inv = false, .ws_inv = false},
          },
  };
  std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;
  if (i2s_channel_init_std_mode(rxChan, &std_cfg) != ESP_OK) return false;
  if (i2s_channel_init_std_mode(txChan, &std_cfg) != ESP_OK) return false;
  i2sReady = true;
  return true;
}

bool parseWavHeader(File& f, uint32_t& dataOffset, uint32_t& dataBytes, uint32_t& rate,
                    uint16_t& bits, uint16_t& ch) {
  if (f.size() < 44) return false;
  f.seek(0);
  char riff[12];
  if (f.readBytes(riff, 12) != 12) return false;
  if (memcmp(riff, "RIFF", 4) != 0 || memcmp(riff + 8, "WAVE", 4) != 0) return false;

  dataOffset = 0;
  dataBytes = 0;
  rate = INOTE_SAMPLE_RATE;
  bits = 16;
  ch = 1;

  while (f.available()) {
    char id[4];
    if (f.readBytes(id, 4) != 4) break;
    uint8_t szb[4];
    if (f.readBytes((char*)szb, 4) != 4) break;
    uint32_t sz = (uint32_t)szb[0] | ((uint32_t)szb[1] << 8) | ((uint32_t)szb[2] << 16) |
                  ((uint32_t)szb[3] << 24);
    if (memcmp(id, "fmt ", 4) == 0) {
      uint8_t fmt[16];
      size_t n = f.readBytes((char*)fmt, min((uint32_t)16, sz));
      if (n >= 16) {
        ch = (uint16_t)fmt[2] | ((uint16_t)fmt[3] << 8);
        rate = (uint32_t)fmt[4] | ((uint32_t)fmt[5] << 8) | ((uint32_t)fmt[6] << 16) |
               ((uint32_t)fmt[7] << 24);
        bits = (uint16_t)fmt[14] | ((uint16_t)fmt[15] << 8);
      }
      if (sz > n) f.seek(f.position() + (sz - n));
    } else if (memcmp(id, "data", 4) == 0) {
      dataOffset = f.position();
      dataBytes = sz;
      return true;
    } else {
      f.seek(f.position() + sz);
    }
  }
  return false;
}
}  // namespace

namespace AudioCapture {

bool begin() {
  pinMode(PIN_PA_EN, OUTPUT);
  pinMode(PIN_PA_CTRL, OUTPUT);
  setAudioPower(true);  // rail on for codec probe
  setPaEnabled(false);
  delay(20);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  codecReady = es8311Init();
  i2sReady = initI2sDuplex();
  Serial.printf("[audio] codec=%d i2s=%d (Waveshare ES8311)\n", (int)codecReady, (int)i2sReady);
  return true;
}

void setAudioPower(bool on) { digitalWrite(PIN_PA_EN, on ? HIGH : LOW); }

void setPaEnabled(bool on) {
  // Waveshare board_cfg pa=46; Audio_PWR stays on while using codec
  digitalWrite(PIN_PA_CTRL, on ? HIGH : LOW);
  if (on) setAudioPower(true);
}

bool startRecording(const String& wavPath) {
  if (recording || playing) return false;
  setAudioPower(true);
  setPaEnabled(false);
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

bool startPlayback(const String& wavPath) {
  if (recording || playing) return false;
  if (!SD.exists(wavPath)) return false;
  playFile = SD.open(wavPath, FILE_READ);
  if (!playFile) return false;

  uint32_t dataOff = 0;
  if (!parseWavHeader(playFile, dataOff, playDataBytes, playRate, playBits, playChannels)) {
    playFile.close();
    return false;
  }
  playFile.seek(dataOff);
  playDataPos = 0;

  setAudioPower(true);
  setPaEnabled(true);
  if (i2sReady && txChan) {
    i2s_channel_enable(txChan);
  }
  playing = true;
  return true;
}

bool isPlaying() { return playing; }

void pollPlayback() {
  if (!playing) return;
  uint8_t buf[512];
  size_t want = min((uint32_t)sizeof(buf), playDataBytes - playDataPos);
  if (want == 0) {
    stopPlayback();
    return;
  }
  size_t n = playFile.read(buf, want);
  if (n == 0) {
    stopPlayback();
    return;
  }
  playDataPos += n;
  size_t written = 0;
  if (i2sReady && txChan) {
    i2s_channel_write(txChan, buf, n, &written, 100);
  } else {
    delay(5);
  }
}

void stopPlayback() {
  if (!playing) return;
  playing = false;
  if (i2sReady && txChan) {
    i2s_channel_disable(txChan);
  }
  if (playFile) playFile.close();
  setPaEnabled(false);
  playDataBytes = 0;
  playDataPos = 0;
}

}  // namespace AudioCapture
