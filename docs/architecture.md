# Architecture

## On-device UX

```text
Sleep → Home (wake)
Home → Record (BOOT short) → Tag (stop) → Home (PWR confirm)
Home → Menu (BOOT hold 1s) → Notes | Tags | Sync | Settings
Sync → Wi-Fi connect/disconnect; portal when STA connected
Home → SoftAP (BOOT hold 3s) | deep sleep (PWR short)
```

See [`docs/gestures.md`](gestures.md).

## Overview

```text
Manual tag (note|work|idea|buy|private) on Tag screen
displayId #NNN from NVS noteSeq
Optional API organize → type task|appointment|note (separate from tag)
STT queue when Wi-Fi up
Portal: library (#NNN, tag, play, delete) + secondary tasks/calendar/ask
```

## Modules

| Module | Responsibility |
|--------|----------------|
| `ui/AppUi` | Home/Record/Tag/Menu/* state machine |
| `display` | E-paper screens |
| `config` | NVS + SD; ui language/date/time; feature flags |
| `storage` | notes, `displayId`, `tag`, markers, ask/briefing |
| `audio` | ES8311 record + WAV playback (Waveshare pin map) |
| `queue` | STT + organize |
| `stt` | Whisper-class STT, organize JSON, ask/briefing |
| `web` | Portal + REST |
| `wifi_setup` | SoftAP + STA connect/disconnect |
| `power` | Buttons, latch, deep sleep |

## Data model

- **`id`**: internal folder id (time + random)
- **`displayId`**: `#001`… human id (NVS `noteSeq`, also in `meta.json` / index)
- **`tag`**: manual enum `note|work|idea|buy|private` (primary on-device organize)
- **`type` / title / dueAt / done**: from optional `features.organize` API — **not** the same as `tag`
- **Legacy migration:** on `NoteStorage::begin()`, notes missing `displayId` or valid `tag` get `#NNN` + `tag=note` and are rewritten once

## TLS

OpenAI HTTPS uses `setInsecure()` in v0.4.x (bring-up). Prefer trusted LAN; pin CA after hardware validation.

## meta.json (fields)

- `displayId`, `tag`
- `markers`: `[{ "tMs", "label" }]`
- organize: `type`, `title`, `dueAt`, `done`, `organizedAt`, `organizeError`

## SD layout

```text
/inote/notes/<id>/{meta.json,audio.wav,transcript.txt}
/inote/index.json
/inote/asks/<id>.json
/inote/briefings/<key>.json
/inote/web/…
```

## Web API

| Method | Path | Notes |
|--------|------|--------|
| GET | `/api/health` | ok, ip, busy |
| GET | `/api/config` | Non-secret ui + features |
| GET | `/api/notes` | List (includes `displayId`, `tag`) |
| GET | `/api/notes/:id` | One note |
| GET | `/api/notes/:id/audio` | WAV |
| DELETE | `/api/notes/:id` | Remove folder + index entry |
| POST | `/api/notes/:id/done` | `{ done }` for organize tasks |
| POST | `/api/ask` | `{ question }` → `{ answer, text, id }` |
| POST | `/api/briefing` | `{ range: day\|week }` |

## Hardware reference

[Waveshare ESP32-S3-ePaper-1.54](https://github.com/waveshareteam/ESP32-S3-ePaper-1.54) — see [`hardware/pins.md`](../hardware/pins.md).

## Power

Idle timeout / short PWR from Home → sleep face → EPD off → deep sleep (BOOT+PWR wake → Home). Long PWR → latch off. No sleep while recording, Tag, SoftAP, playback, or portal ask/briefing busy.
