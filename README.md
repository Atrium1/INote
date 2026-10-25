# INote

Offline-first pocket voice notes for **Waveshare ESP32-S3-ePaper-1.54 V2**. Record on device, tag manually, sync over Wi-Fi; LAN library portal with playback and delete.

Hardware pin reference: [waveshareteam/ESP32-S3-ePaper-1.54](https://github.com/waveshareteam/ESP32-S3-ePaper-1.54).

## Status

v0.4: Home/Record/Tag/Menu UX, `#NNN` + tags, speaker play/delete, Sync Wi-Fi, redesigned portal.

## Quick start (no board)

```bash
cd web && python -m http.server 8080
```

```bash
cd scripts
pip install -r requirements.txt
python organize_mock.py --text "Yarın market"
python ask_mock.py --question "Dişçi ne zaman?" --notes ../web/fixtures/notes.json
```

```bash
cd firmware && pio run
```

## Config

See [`config.example.json`](config.example.json):

- `features.organize` — API `type` task/appointment (separate from manual **tag**)
- `features.ask` / `briefing`
- `ui.language` (`tr`|`en`|`de`), date/time formats
- `power.idleSleepSeconds`

## Gestures

Full map: [`docs/gestures.md`](docs/gestures.md).

| Gesture | Action |
|---------|--------|
| BOOT short (Home) | Start recording |
| BOOT short (Record) | Stop → tag picker |
| BOOT hold ~1s (Home) | Menu |
| BOOT hold ~3s | SoftAP setup |
| PWR short (Home) | Deep sleep |
| Menu → Sync / PWR | Wi-Fi connect/disconnect |

Copy `web/` to SD `/inote/web/` on the device.

## Docs

- [`docs/architecture.md`](docs/architecture.md)
- [`docs/bringup.md`](docs/bringup.md)
- [`hardware/pins.md`](hardware/pins.md)

## License

MIT — [LICENSE](LICENSE).
