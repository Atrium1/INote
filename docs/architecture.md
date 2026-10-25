# Architecture

## Overview

INote is an offline-first voice capture device. Audio and metadata live on microSD. When Wi-Fi and an API key are available, pending notes are transcribed (and optionally organized) via OpenAI. A LAN HTTP server exposes a searchable portal. Idle deep sleep keeps battery life high while the e-ink face stays visible.

```text
BOOT short press → record WAV → SD + queue(pending)
Wi-Fi + API key  → STT → ready → (optional) organize
Phone on LAN     → GET /api/notes + audio + task done
Idle timeout     → draw "INote" → EPD off → deep sleep (BOOT/PWR wake)
```

## Modules (firmware)

| Module | Responsibility |
|--------|----------------|
| `config` | Merge NVS + optional SD `/inote/config.json`; never log secrets |
| `storage` | SD paths, `index.json`, `meta.json`, WAV/transcript files |
| `queue` | Pending → STT → ready; optional organize; retry on reconnect |
| `audio` | ES8311 I2S capture → 16 kHz mono 16-bit WAV |
| `display` | E-paper UI + sleep face; `EPD_PWR` control |
| `wifi_setup` | SoftAP `INote-Setup`; STA with up to 5 SSIDs |
| `stt` | OpenAI transcription + JSON organize classify |
| `web` | Static UI + REST API |
| `power` | BAT latch, buttons, activity idle timer, deep sleep |

## Note lifecycle

```text
recording → pending → transcribing → ready
                              ↘ error (retry later)
ready + features.organize → classify → type/title/dueAt (or organizeError)
```

## On-disk layout (microSD, FAT32)

```text
/inote/
  config.json
  index.json
  notes/<id>/meta.json
  notes/<id>/audio.wav
  notes/<id>/transcript.txt
  web/                 # optional copy of portal assets
```

### `meta.json`

| Field | Type | Notes |
|-------|------|--------|
| `id` | string | Time-based id |
| `createdAt` / `updatedAt` | string | ISO-8601 UTC |
| `audioPath` | string | Relative under `/inote` |
| `transcript` | string | Also in `transcript.txt` |
| `status` | string | `recording` \| `pending` \| `transcribing` \| `ready` \| `error` |
| `tags` | string[] | From organize when present |
| `type` | string \| null | `note` \| `task` \| `appointment` |
| `title` | string \| null | Short summary |
| `dueAt` | string \| null | ISO-8601 |
| `done` | bool | Tasks only |
| `organizedAt` | string \| null | Set on classify success |
| `organizeError` | string \| null | Non-fatal; status stays `ready` |
| `error` | string \| null | STT failure |
| `durationMs` | number | Recording length |

### `index.json`

Includes `type`, `done`, `dueAt` summaries for cheap listing.

## Power state machine

```text
awake (idle/recording/sync/setup)
  → idleSleepSeconds with no activity (blocked if recording/busy/setup)
  → draw sleepFace ("INote") → EPD_PWR HIGH → Wi-Fi off → deep sleep
  → wake BOOT or PWR → idle UI (no auto-record)

PWR short (awake): enter sleep sequence (if shortPwrSleeps)
PWR long ~2s: sleep face → EPD off → BAT latch LOW (soft power-off on battery)
```

Wake pins: GPIO0 (BOOT), GPIO18 (PWR), `ESP_EXT1_WAKEUP_ANY_LOW`.

With `wifiOffWhenIdle`, STA reconnects when there is pending STT/organize work or the user is actively using the device (portal started after connect).

## Config merge order

1. Built-in defaults  
2. NVS  
3. SD `/inote/config.json` overrides  

## Web API

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/health` | Device status |
| GET | `/api/notes` | List + meta |
| GET | `/api/notes/:id` | Full meta |
| GET | `/api/notes/:id/audio` | WAV |
| POST | `/api/notes/:id/done` | Body `{ "done": true\|false }` |
| GET | `/` | Static portal |

## i18n

String table `tr` / `en` via `config.ui.language`.
