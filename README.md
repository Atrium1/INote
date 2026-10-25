# INote

Offline-first pocket voice notes for **Waveshare ESP32-S3-ePaper-1.54 V2**. Record on device, tag manually, sync over Wi-Fi; LAN library portal with playback and delete.

Hardware pin reference: [waveshareteam/ESP32-S3-ePaper-1.54](https://github.com/waveshareteam/ESP32-S3-ePaper-1.54).

## Status

**v0.4.1** (v1-rc software): real Settings persistence, Notes back/delete gestures, legacy `#NNN`/tag migration, setup script, error/empty messaging. Hardware bring-up still required for product v1.

## Quick start (no board)

```bash
cd web && python -m http.server 8080
```

```bash
python scripts/check_fixtures.py
python scripts/prepare_sd.py --out ./sd_image
```

```bash
cd firmware && pio run
```

## Prepare SD

1. Format microSD **FAT32**.
2. `python scripts/prepare_sd.py --out ./sd_image` then copy `sd_image/*` to the card root so `/inote/web/` and `/inote/config.json` exist.
3. Edit `inote/config.json` (Wi-Fi + `openaiApiKey`) or use SoftAP **INote-Setup**.
4. On device: Menu → **Sync** → PWR to connect; portal at `http://<device-ip>/`.

## Config

See [`config.example.json`](config.example.json):

- `audio.soundEnabled` — on-device speaker playback
- `features.organize` — API `type` (separate from manual **tag**)
- `ui.language` (`tr`|`en`|`de`), date/time formats
- `power.idleSleepSeconds` (`0` = off)

### TLS note

OpenAI HTTPS clients use `WiFiClientSecure::setInsecure()` for v0.4.x bring-up (MITM risk on untrusted networks). Certificate pinning is planned after hardware validation.

## Gestures

Full map: [`docs/gestures.md`](docs/gestures.md). Notes: BOOT hold → Menu; **PWR long** → delete confirm.

## Docs

- [`docs/architecture.md`](docs/architecture.md)
- [`docs/bringup.md`](docs/bringup.md)
- [`docs/test_plan_v1rc.md`](docs/test_plan_v1rc.md)
- [`hardware/pins.md`](hardware/pins.md)

## License

MIT — [LICENSE](LICENSE).
