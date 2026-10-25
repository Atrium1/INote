# INote

**INote** is an offline-first pocket voice notebook for the [Waveshare ESP32-S3-ePaper-1.54 V2](https://www.waveshare.com/esp32-s3-epaper-1.54.htm). Capture short notes on device, assign a manual tag, then sync over Wi‑Fi to a LAN library portal for playback, search, and delete—without a phone app or cloud account.

| | |
|---|---|
| **Hardware** | ESP32-S3 · 1.54″ e-paper (200×200) · ES8311 audio · microSD · LiPo |
| **On device** | Record → Tag → Menu (Notes / Tags / Sync / Settings) · deep sleep |
| **On LAN** | Browser portal at `http://<device-ip>/` |
| **Status** | **v0.4.1** software (v1-rc) — product v1 still needs hardware bring-up |

Pin map and vendor examples: [waveshareteam/ESP32-S3-ePaper-1.54](https://github.com/waveshareteam/ESP32-S3-ePaper-1.54).

---

## Why INote

- **Capture first.** BOOT starts recording; stop lands on a tag picker—no typing on a 200×200 panel.
- **Organize by hand.** Tags are `note | work | idea | buy | private`, separate from optional API “organize” types.
- **Human ids.** Notes show as `#001`, `#002`, … (`displayId`), not opaque folder names.
- **Stay offline until you choose.** SoftAP setup and STA Sync are explicit; idle Wi‑Fi can stay off.
- **Paper-like UI.** FreeSans chrome (headers, chips, metrics) with `tr` / `en` / `de` device strings.

---

## Features (v0.4.1)

**Device**
- Home hero (localized TODAY) with NOTES / PENDING / TASKS
- Record with markers; Tag as selectable chips; Menu / Settings as rounded rows
- Notes & Tags browse, speaker playback (when Sound ON), delete confirm
- Sync: connect / disconnect Wi‑Fi; health lines (SD / API key) on Sync
- Settings: Sound · Language · Idle sleep · Organize (NVS persist)
- SoftAP `INote-Setup`, deep sleep with sticky INote face, battery latch soft-off

**Portal & pipeline**
- Library-first web UI (tag chips, play, delete)
- STT queue when Wi‑Fi is up (OpenAI-compatible); optional organize JSON
- Calendar / tasks / ask / briefing APIs (secondary to the library)

**Tooling**
- Browser e-ink flow simulator (BOOT / PWR gestures)
- SD card image helper and fixture smoke checks

Out of scope for v1: native mobile app, BLE, cloud sync. See [`docs/mvp.md`](docs/mvp.md) and [`docs/roadmap.md`](docs/roadmap.md).

---

## Repository layout

```text
firmware/          PlatformIO firmware (Arduino / ESP32-S3)
web/               LAN portal static assets (also copied to SD)
tools/epaper-sim/  PC mock of the 200×200 device UI
scripts/           prepare_sd.py, check_fixtures.py
docs/              architecture, gestures, bring-up, test plan
hardware/          BOM, pins
config.example.json
```

---

## Hardware

| Item | Requirement |
|------|-------------|
| Board | Waveshare **ESP32-S3-ePaper-1.54 V2** (8 MB flash + 8 MB PSRAM). Do **not** use V1. |
| Storage | microSD, **FAT32**, Class 10+ |
| Battery | Single-cell **3.7 V LiPo**, **MX1.25** 2-pin (~300–1000 mAh typical) |
| Cable | USB-C **data** cable for flash / serial |

Full BOM and “do not buy” notes: [`hardware/bom.md`](hardware/bom.md). Pins: [`hardware/pins.md`](hardware/pins.md). Bring-up: [`docs/bringup.md`](docs/bringup.md).

---

## Quick start (no hardware)

### Portal preview

```bash
cd web
python -m http.server 8080
```

Open http://127.0.0.1:8080/

### E-ink flow simulator

Mocks Home → Record → Tag → Menu and gestures on a 200×200 canvas (IBM Plex Sans approximating device FreeSans).

```bash
cd tools/epaper-sim
python -m http.server 8090
```

Open http://127.0.0.1:8090/ — details in [`tools/epaper-sim/README.md`](tools/epaper-sim/README.md).

### Fixtures & SD image

```bash
python scripts/check_fixtures.py
python scripts/prepare_sd.py --out ./sd_image
```

### Firmware build

Requires [PlatformIO](https://platformio.org/) (CLI or IDE).

```bash
cd firmware
pio run
# with board connected:
pio run -t upload
pio device monitor
```

Environment: `inote-s3-epaper` in [`firmware/platformio.ini`](firmware/platformio.ini).

---

## Prepare the microSD

1. Format the card **FAT32**.
2. Generate a tree and copy it to the card root:

   ```bash
   python scripts/prepare_sd.py --out ./sd_image
   ```

   After copy you should have `/inote/web/` and `/inote/config.json` on the card.

3. Edit `inote/config.json` (Wi‑Fi + `openaiApiKey`), or use SoftAP **INote-Setup** on first boot.
4. On device: **Menu → Sync → PWR** to connect; open `http://<device-ip>/`.

SD layout and REST surface: [`docs/architecture.md`](docs/architecture.md).

---

## Configuration

Start from [`config.example.json`](config.example.json). Important keys:

| Area | Keys | Notes |
|------|------|--------|
| Audio | `audio.soundEnabled` | On-device speaker playback |
| Features | `features.organize` | API `type` (not the same as manual `tag`) |
| UI | `ui.language` | `tr` \| `en` \| `de`; date/time formats |
| Power | `power.idleSleepSeconds` | `0` = idle sleep off; sleep face text |
| Network / API | Wi‑Fi + `openaiApiKey` | Prefer SoftAP or SD edit; secrets stay off the public API |

Settings changed on the device (Sound / Language / Idle / Organize) persist to NVS immediately.

### TLS

OpenAI HTTPS clients currently use `WiFiClientSecure::setInsecure()` for v0.4.x bring-up. That is convenient on a trusted LAN and **unsafe on untrusted networks** (MITM). Certificate pinning is planned after hardware validation.

---

## Gestures (summary)

Buttons: **BOOT** (GPIO0) · **PWR** (GPIO18). Wake from sleep always lands on **Home**.

| From | Gesture | Result |
|------|---------|--------|
| Home | BOOT short | Record |
| Home | BOOT hold ~1s | Menu |
| Home | BOOT hold ~3s | SoftAP setup |
| Home | PWR short / idle | Deep sleep |
| Record | BOOT short | Tag |
| Notes | PWR long | Delete confirm |
| Notes / lists | BOOT hold | Back to Menu |
| Sync | PWR short | Wi‑Fi on/off |

Full table: [`docs/gestures.md`](docs/gestures.md).

---

## Documentation

| Doc | Contents |
|-----|----------|
| [`docs/architecture.md`](docs/architecture.md) | UX flow, modules, data model, API |
| [`docs/gestures.md`](docs/gestures.md) | Complete button map |
| [`docs/bringup.md`](docs/bringup.md) | First power-on checklist |
| [`docs/test_plan_v1rc.md`](docs/test_plan_v1rc.md) | v1-rc software checks |
| [`docs/mvp.md`](docs/mvp.md) | Version scope |
| [`docs/roadmap.md`](docs/roadmap.md) | Next steps |
| [`hardware/bom.md`](hardware/bom.md) | Parts list |
| [`hardware/pins.md`](hardware/pins.md) | GPIO map |

---

## License

MIT — see [LICENSE](LICENSE).
