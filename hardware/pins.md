# Pin map — ESP32-S3-ePaper-1.54 (B/W)

Source of truth: Waveshare docs / vendor `user_config.h`. **Verify on bring-up.** Do not use the 1.54G colour board pin table blindly if anything conflicts with the B/W repo.

## E-paper (SPI)

| Signal | GPIO | Notes |
|--------|------|--------|
| EPD_PWR | 6 | **Active LOW** — drive LOW to power panel |
| BUSY | 8 | |
| RST | 9 | |
| DC | 10 | |
| CS | 11 | |
| SCK | 12 | |
| MOSI | 13 | |
| Panel | — | 200×200 B/W (SSD1681 / GDEY0154D67 class via GxEPD2) |

## I2C (shared)

| Signal | GPIO | Devices |
|--------|------|---------|
| SDA | 47 | ES8311 `0x18`, SHTC3 `0x70`, PCF85063 `0x51` |
| SCL | 48 | |

## Audio (ES8311 / I2S)

| Signal | GPIO | Notes |
|--------|------|--------|
| MCLK | 14 | |
| SCLK | 15 | |
| ASDOUT | 16 | Mic data in |
| LRCK | 38 | |
| DSDIN | 45 | DAC out (unused in v0.1) |
| PA_EN | 42 | Keep off / inactive in v0.1 |
| PA_CTRL | 46 | Keep off in v0.1 |

## microSD

| Signal | GPIO | Notes |
|--------|------|--------|
| CLK | 39 | |
| MISO | 40 | |
| MOSI | 41 | |
| CS | 21 | Used in firmware `board_pins.h`; confirm against Waveshare schematic on bring-up |

## Power / buttons

| Signal | GPIO | Notes |
|--------|------|--------|
| BOOT | 0 | Capture gestures; hold for download mode |
| PWR / BAT_KEY | 18 | Sleep / power button |
| BAT_Control | 17 | Hold HIGH to stay on under battery |
| BAT_ADC | 4 | Voltage divider; Vbat ≈ Vad × 2 |

## USB

| Signal | GPIO |
|--------|------|
| USB_N | 19 |
| USB_P | 20 |

## Expansion header

GPIO1, GPIO2, GPIO3 plus power / I2C / UART — unused in v0.1.
