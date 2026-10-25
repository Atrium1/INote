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
3. Idle UI shows note count, open tasks, next due, Wi-Fi / battery.  
4. No Wi-Fi config → SoftAP SSID **`INote-Setup`**; open captive portal, save networks + API key.  
5. Short BOOT → recording; short BOOT again → stop → `pending`.  
6. With `wifiOffWhenIdle`, STA connects when there is pending STT/organize work.  

## Sleep / power (validate on battery)

| Action | Expected |
|--------|----------|
| No buttons for `power.idleSleepSeconds` (default 45) | Draw **INote**, cut `EPD_PWR` (image stays), deep sleep |
| Short **PWR** | Same sleep sequence |
| Long **PWR** (~2s) | Sleep face → latch GPIO17 LOW (soft off). On USB, may fall through to deep sleep |
| **BOOT** or **PWR** while sleeping | Wake to idle (does **not** auto-start recording) |

Wake uses `esp_sleep_enable_ext1_wakeup` on GPIO0 + GPIO18, `ANY_LOW`.

**USB vs battery:** Latch-off only fully cuts power on LiPo. With USB, expect deep sleep instead. Measure current later with a meter on battery.

Do not sleep while recording, SoftAP setup, or JobQueue busy.

## Portal files on SD

Copy the repo `web/` directory to the card as `/inote/web/` (`index.html`, `app.js`, `styles.css`). If missing, the device serves a minimal fallback page; `/api/notes` still works.

Task done toggle: `POST /api/notes/:id/done` with JSON `{ "done": true }`.

## Audio bring-up (ES8311)

Pins: MCLK 14, SCLK 15, ASDOUT 16, LRCK 38, DSDIN 45; keep PA (42/46) **off** until TTS.

Firmware ships a **minimal ES8311 init + I2S RX**. If the codec does not ACK, `AudioCapture` still writes a silent WAV so storage/queue/STT can be tested — replace the register sequence with Waveshare’s full example once the board is on the desk.

1. Confirm I2C sees ES8311 at `0x18` (bus SDA 47 / SCL 48).  
2. Record 5 s WAV; verify file size ≈ `16000 * 2 * seconds` + 44-byte header.  
3. Play WAV on PC; then run `scripts/transcribe_mock.py` / `organize_mock.py` or device workers.  
4. Tune ADC gain / I2S slot (left/right) if audio is quiet or empty.  

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

## API cost control

Set a monthly usage limit in the OpenAI dashboard (e.g. $5–10). Disable `features.autoTranscribe` to queue locally only. Organize adds one small chat call per note when enabled.

## PC test plan (no board)

1. `cd web && python -m http.server 8080` — filter by Notes/Tasks/Appointments; fixture done checkbox updates locally.  
2. `python scripts/organize_mock.py --text "Yarın sabah market alışverişi yap"` — expect `type: task` JSON.  
3. `pio run` in `firmware/` — build succeeds.  
