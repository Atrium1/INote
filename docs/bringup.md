# Hardware bring-up

Do this after the **V2** board arrives. Firmware should already compile via `pio run`.

## Checklist before power

1. Confirm board is **ESP32-S3-ePaper-1.54 V2** (8 MB flash / 8 MB PSRAM). V1 will not fit the partition scheme.  
2. Format microSD as **FAT32**; insert firmly.  
3. Use a USB-C **data** cable (not charge-only).  
4. Optional: copy `config.example.json` → SD `/inote/config.json` with Wi-Fi + API key (or use SoftAP later). Set `"features": { "organize": true }` to enable classification.  
5. Optional LiPo on **MX1.25** 2-pin (polarity correct).  

## Flash

```bash
cd firmware
pio run -t upload
pio device monitor -b 115200
```

If upload fails: hold **BOOT**, reset/power-cycle into download mode, retry.

PlatformIO settings (see `platformio.ini`):

- Board: `esp32-s3-devkitc-1`  
- Flash 8 MB, OPI PSRAM (`qio_opi`)  
- `BOARD_HAS_PSRAM`, USB CDC on boot  

## First boot expectations

1. BAT latch GPIO17 driven high while running (battery power).  
2. E-paper power: **GPIO6 LOW** to enable panel — if blank, check `EPD_PWR`.  
3. Boot lands on **Home** (note count, Wi-Fi / battery). See [`docs/gestures.md`](gestures.md).  
4. No Wi-Fi config → SoftAP SSID **`INote-Setup`**; or use Menu → Sync later.  
5. Home BOOT short → Record; BOOT short → Tag picker; PWR confirms tag → Home (`#NNN` + `tag` in meta).  
6. During recording, hold BOOT ~1s → marker; Menu → Notes → PWR plays via speaker.  
7. Menu → Sync → PWR toggles STA; portal starts when connected.  
8. Copy full `web/` to `/inote/web/`. Enable `features.ask` / `briefing` for secondary Ask UI.  

## Sleep / power (validate on battery)

| Action | Expected |
|--------|----------|
| No buttons for `power.idleSleepSeconds` (default 45) | Draw **INote**, cut `EPD_PWR` (image stays), deep sleep |
| Short **PWR** | Same sleep sequence |
| Long **PWR** (~2s) | Sleep face → latch GPIO17 LOW (soft off). On USB, may fall through to deep sleep |
| **BOOT** or **PWR** while sleeping | Wake to **Home** (does **not** auto-start recording) |

Wake uses `esp_sleep_enable_ext1_wakeup` on GPIO0 + GPIO18, `ANY_LOW`.

**USB vs battery:** Latch-off only fully cuts power on LiPo. With USB, expect deep sleep instead. Measure current later with a meter on battery.

Do not sleep while recording, SoftAP setup, or JobQueue busy.

## Portal files on SD

Copy the repo `web/` directory to the card as `/inote/web/` (`index.html`, `app.js`, `styles.css`). If missing, the device serves a minimal fallback page; `/api/notes` still works.

Task done: `POST /api/notes/:id/done` `{ "done": true }`. Delete: `DELETE /api/notes/:id`.

## Audio bring-up (ES8311)

Vendor reference: [Waveshare ESP32-S3-ePaper-1.54](https://github.com/waveshareteam/ESP32-S3-ePaper-1.54) `08_Audio_Test` / `board_cfg.h` (`S3_ePaper_1_54`).

Pins: MCLK 14, BCLK 15, DIN 16, WS 38, DOUT 45; **Audio_PWR 42** on for codec; **PA 46** on for playback.

Firmware: stronger ES8311 ADC+DAC init + I2S duplex; record path + WAV playback. If codec does not ACK, record still writes silent WAV for pipeline tests.

1. Confirm I2C ES8311 at `0x18` (SDA 47 / SCL 48).  
2. Record 5 s; file size ≈ `16000 * 2 * seconds` + 44.  
3. Menu → Notes → PWR to play on speaker; or portal player / PC.  
4. Tune ADC gain / I2S slot if quiet or empty.  
5. SD: Waveshare demo is SDMMC 1-bit (39/41/40); firmware tries Arduino SPI + CS 21 — if mount fails, migrate to SDMMC.  

## Common failures

| Symptom | Likely cause |
|---------|----------------|
| Boot loop / brownout after flash | V1 board or wrong flash/PSRAM settings |
| No COM port | Charge-only cable; driver; hold BOOT for download |
| Blank e-paper | `EPD_PWR` high; wrong BUSY/RST/DC/CS |
| SD mount fail | Not FAT32; bad card; CS/SPI pins |
| HTTPS / STT fail | No PSRAM; clock/TLS; bad API key; no STA |
| Empty / noisy audio | ES8311 init; I2S pin mix-up; gain |
| Won’t wake | Wrong wake pins; button stuck; latch already off |
| Sleep face never drawn | Sleep during recording/queue/setup (blocked) |

## TLS (OpenAI)

Firmware calls `WiFiClientSecure::setInsecure()` for STT / organize / ask. Acceptable for early bring-up on a trusted network; **MITM risk** on hostile Wi-Fi. Certificate pinning is not required for hardware validation; add before wider deployment.

## API cost control

Set a monthly usage limit in the OpenAI dashboard (e.g. $5–10). Disable `features.autoTranscribe` to queue locally only. Organize adds one small chat call per note when enabled.

## PC test plan (no board)

1. `cd web && python -m http.server 8080` — library `#NNN` + tag chips; delete in fixture mode.  
2. `python scripts/check_fixtures.py` and `python scripts/prepare_sd.py --out ./sd_image`.  
3. `python scripts/organize_mock.py --text "Yarın sabah market alışverişi yap"` — expect `type: task` JSON (API organize ≠ manual `tag`).  
4. `pio run` in `firmware/` — build succeeds. See [`docs/test_plan_v1rc.md`](test_plan_v1rc.md).  
