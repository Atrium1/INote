# Pin map — ESP32-S3-ePaper-1.54 (B/W)

Source of truth: [waveshareteam/ESP32-S3-ePaper-1.54](https://github.com/waveshareteam/ESP32-S3-ePaper-1.54)

- `02_Example/Arduino/*/user_config.h` — EPD, I2C, power, buttons
- `02_Example/Arduino/08_Audio_Test/src/codec_board/board_cfg.h` — `S3_ePaper_1_54` I2S / ES8311 / PA
- `02_Example/Arduino/04_SD_Card/sdcard_bsp.cpp` — SDMMC 1-bit

**Verify on bring-up.** Do not use conflicting DeepWiki / colour-board tables for SD CS vs I2S SCLK.

## E-paper (SPI)

| Signal | GPIO | Notes |
|--------|------|--------|
| EPD_PWR | 6 | **Active LOW** |
| BUSY | 8 | |
| RST | 9 | |
| DC | 10 | |
| CS | 11 | |
| SCK | 12 | |
| MOSI | 13 | |
| Panel | — | 200×200 B/W (GxEPD2 `GxEPD2_154_D67`) |

## I2C

| Signal | GPIO | Devices |
|--------|------|---------|
| SDA | 47 | ES8311 `0x18`, SHTC3 `0x70`, PCF85063 `0x51` |
| SCL | 48 | |

## Audio (ES8311 / I2S)

| Signal | GPIO | Notes |
|--------|------|--------|
| MCLK | 14 | |
| SCLK / BCLK | 15 | |
| ASDOUT / DIN | 16 | Mic |
| LRCK / WS | 38 | |
| DSDIN / DOUT | 45 | DAC → speaker |
| Audio_PWR / PA_EN | 42 | Codec / amp rail (`user_config` Audio_PWR_PIN) |
| PA | 46 | Amp enable (`board_cfg` `pa: 46`) |

## microSD

Waveshare demo uses **SDMMC 1-bit** (no CS):

| Signal | GPIO |
|--------|------|
| CLK | 39 |
| CMD | 41 |
| D0 | 40 |

Firmware currently mounts via **Arduino SD SPI** on the same CLK/MOSI/MISO with `CS=21` (SPI host select; not in the Waveshare SDMMC example). Confirm on bring-up; migrate to SDMMC if SPI fails.

## Power / buttons

| Signal | GPIO | Notes |
|--------|------|--------|
| BOOT | 0 | Gestures; download mode |
| PWR | 18 | Sleep / power |
| VBAT / latch | 17 | Hold HIGH on battery |
| BAT_ADC | 4 | Vbat ≈ Vad × 2 |
