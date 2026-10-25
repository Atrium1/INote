#pragma once

// Waveshare ESP32-S3-ePaper-1.54 (B/W). Verify against vendor schematic on bring-up.

// E-paper SPI
static const int PIN_EPD_PWR = 6;   // active LOW
static const int PIN_EPD_BUSY = 8;
static const int PIN_EPD_RST = 9;
static const int PIN_EPD_DC = 10;
static const int PIN_EPD_CS = 11;
static const int PIN_EPD_SCK = 12;
static const int PIN_EPD_MOSI = 13;

// I2C
static const int PIN_I2C_SDA = 47;
static const int PIN_I2C_SCL = 48;

// ES8311 I2S
static const int PIN_I2S_MCLK = 14;
static const int PIN_I2S_SCLK = 15;
static const int PIN_I2S_ASDOUT = 16;
static const int PIN_I2S_LRCK = 38;
static const int PIN_I2S_DSDIN = 45;
static const int PIN_PA_EN = 42;
static const int PIN_PA_CTRL = 46;

// microSD (SPI) — CS confirm on schematic / Waveshare SD example
static const int PIN_SD_CLK = 39;
static const int PIN_SD_MISO = 40;
static const int PIN_SD_MOSI = 41;
static const int PIN_SD_CS = 21;

// Power / buttons
static const int PIN_BOOT_BTN = 0;
static const int PIN_PWR_BTN = 18;
static const int PIN_BAT_LATCH = 17;
static const int PIN_BAT_ADC = 4;

// Audio capture defaults
static const int INOTE_SAMPLE_RATE = 16000;
static const int INOTE_SAMPLE_BITS = 16;
static const int INOTE_CHANNELS = 1;
