# INote

Offline-first pocket voice note device for the **Waveshare ESP32-S3-ePaper-1.54 V2** (8 MB flash / 8 MB PSRAM). Record a thought without unlocking a phone; sync, transcribe, and optionally organize when Wi-Fi is available. Idle deep sleep keeps a sticky e-ink **INote** face for long battery life.

Original project (MIT). Pala Note and similar products are inspiration only — no proprietary firmware or assets.

## Status

v0.2 prep: organization (optional GPT classify), LAN portal task filters, idle deep sleep, PC mocks, PlatformIO firmware targeting the V2 board.

## Repo layout

| Path | Purpose |
|------|---------|
| [`firmware/`](firmware/) | PlatformIO ESP32-S3 firmware |
| [`web/`](web/) | Local portal UI (PC prototype + device assets) |
| [`scripts/`](scripts/) | PC mocks: STT + organize |
| [`docs/`](docs/) | Architecture, MVP, roadmap, bring-up |
| [`hardware/`](hardware/) | BOM and pin map |
| [`config.example.json`](config.example.json) | Sample config (never commit real secrets) |

## Quick start (no board)

### Mock transcription

```bash
cd scripts
python -m venv .venv
# Windows: .venv\Scripts\activate
pip install -r requirements.txt
copy ..\config.example.json .\config.json   # edit API key
python transcribe_mock.py path\to\note.wav
```

### Mock organize

```bash
python organize_mock.py --text "Yarın dişçi randevusu saat 10"
python organize_mock.py --meta path\to\meta.json --force
```

### Web portal prototype

```bash
cd web
python -m http.server 8080
# open http://localhost:8080 — fixtures include tasks/appointments
```

### Firmware build

```bash
cd firmware
pio run
```

## Config highlights

See [`config.example.json`](config.example.json):

- `features.organize` — classify after STT (`organize.model`, default `gpt-4o-mini`)
- `power.idleSleepSeconds` — default 45; sticky sleep face `INote`
- `power.wifiOffWhenIdle` — reconnect when pending work exists

## Hardware

- **V2 only:** 8 MB Flash + 8 MB PSRAM (not V1)
- Non-touch board, FAT32 microSD, 3.7 V LiPo (MX1.25), USB-C **data** cable

See [`hardware/bom.md`](hardware/bom.md) and [`docs/bringup.md`](docs/bringup.md).

## License

MIT — see [LICENSE](LICENSE).
