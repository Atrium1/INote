# INote

Offline-first pocket voice note device for the **Waveshare ESP32-S3-ePaper-1.54 V2** (8 MB flash / 8 MB PSRAM). Record a thought without unlocking a phone; sync and transcribe when Wi-Fi is available.

Original project (MIT). Pala Note and similar products are inspiration only — no proprietary firmware or assets.

## Status

Pre-hardware prep: docs, PC mock STT pipeline, static web portal, and PlatformIO firmware skeleton targeting the V2 board.

## Repo layout

| Path | Purpose |
|------|---------|
| [`firmware/`](firmware/) | PlatformIO ESP32-S3 firmware |
| [`web/`](web/) | Local portal UI (PC prototype + device assets) |
| [`scripts/`](scripts/) | PC mock: WAV → OpenAI → note JSON |
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

### Web portal prototype

Serve [`web/`](web/) with any static server and open `index.html`. It loads [`web/fixtures/notes.json`](web/fixtures/notes.json).

```bash
cd web
python -m http.server 8080
# open http://localhost:8080
```

### Firmware build (board optional)

```bash
cd firmware
pio run
```

Upload waits until the board is connected (`pio run -t upload`).

## Hardware (order carefully)

- **V2 only:** ESP32-S3-ePaper-1.54 with **8 MB Flash + 8 MB PSRAM** (not V1 4/2 MB)
- Non-touch variant for v0.1
- FAT32 microSD, 3.7 V LiPo (MX1.25), USB-C **data** cable

See [`hardware/bom.md`](hardware/bom.md) and [`docs/bringup.md`](docs/bringup.md).

## License

MIT — see [LICENSE](LICENSE).
