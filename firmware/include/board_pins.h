#pragma once

// Waveshare ESP32-S3-ePaper-1.54 — pins from vendor repo
// https://github.com/waveshareteam/ESP32-S3-ePaper-1.54
//   02_Example/Arduino/*/user_config.h
//   02_Example/Arduino/08_Audio_Test/src/codec_board/board_cfg.h (S3_ePaper_1_54)
//   02_Example/Arduino/04_SD_Card/sdcard_bsp.cpp (SDMMC 1-bit)

// E-paper SPI
static const int PIN_EPD_PWR = 6;   // active LOW (EPD_PWR_PIN)
static const int PIN_EPD_BUSY = 8;
static const int PIN_EPD_RST = 9;
static const int PIN_EPD_DC = 10;
static const int PIN_EPD_CS = 11;
static const int PIN_EPD_SCK = 12;
static const int PIN_EPD_MOSI = 13;

// I2C (ES8311 0x18, SHTC3, PCF85063)
static const int PIN_I2C_SDA = 47;
static const int PIN_I2C_SCL = 48;

// ES8311 I2S — board_cfg: bclk 15, ws 38, dout 45, din 16, mclk 14
static const int PIN_I2S_MCLK = 14;
static const int PIN_I2S_SCLK = 15;
static const int PIN_I2S_ASDOUT = 16;  // din (mic)
static const int PIN_I2S_LRCK = 38;
static const int PIN_I2S_DSDIN = 45;   // dout (DAC / speaker)
// Waveshare: Audio_PWR_PIN=42, codec pa=46
static const int PIN_PA_EN = 42;      // audio rail / PA power
static const int PIN_PA_CTRL = 46;    // amp enable

// microSD — Waveshare demo uses SDMMC 1-bit (no CS): CLK=39, CMD=41, D0=40.
// Firmware uses Arduino SD SPI on the same CLK/MOSI/MISO; CS=21 is SPI chip-select
// for the Arduino host (not listed in Waveshare SDMMC example — verify on bring-up).
static const int PIN_SD_CLK = 39;
static const int PIN_SD_MISO = 40;  // SDMMC D0
static const int PIN_SD_MOSI = 41;  // SDMMC CMD
static const int PIN_SD_CS = 21;

// Power / buttons
static const int PIN_BOOT_BTN = 0;
static const int PIN_PWR_BTN = 18;
static const int PIN_BAT_LATCH = 17;  // VBAT_PWR_PIN
static const int PIN_BAT_ADC = 4;

// Audio capture defaults
static const int INOTE_SAMPLE_RATE = 16000;
static const int INOTE_SAMPLE_BITS = 16;
static const int INOTE_CHANNELS = 1;
