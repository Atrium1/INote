# INote

Offline-first pocket voice notes for **Waveshare ESP32-S3-ePaper-1.54 V2**. Transcribe and optionally organize on Wi-Fi; portal with tasks/calendar; text ask/briefings; idle e-ink sleep.

## Status

v0.3: calendar + tasks UI, recording markers, date/time formats + `de`, text ask/briefing APIs (TTS later).

## Quick start (no board)

```bash
cd web && python -m http.server 8080
```

```bash
cd scripts
pip install -r requirements.txt
python organize_mock.py --text "Yarın market"
python ask_mock.py --question "Dişçi ne zaman?" --notes ../web/fixtures/notes.json
python briefing_mock.py --range day --notes ../web/fixtures/notes.json
```

```bash
cd firmware && pio run
```

## Config

See [`config.example.json`](config.example.json):

- `features.organize` / `ask` / `briefing`
- `ui.language` (`tr`|`en`|`de`), `dateFormat` (`DMY`|`YMD`|`MDY`), `timeFormat` (`H24`|`H12`)
- `power.idleSleepSeconds`

## Gestures

| Gesture | Action |
|---------|--------|
| BOOT short | Start/stop recording |
| BOOT hold ~1s (recording) | Add marker |
| BOOT hold ~3s (idle) | Wi-Fi setup AP |
| PWR short | Deep sleep |
| PWR long | Soft power off (latch) |

Copy `web/` to SD `/inote/web/` on the device.

## License

MIT — [LICENSE](LICENSE).
