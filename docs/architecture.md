# Architecture

## Overview

INote is an offline-first voice capture device. Audio and metadata live on microSD. When Wi-Fi and an API key are available, pending notes are transcribed via a pluggable STT client (OpenAI `gpt-transcribe` in v0.1). A LAN HTTP server exposes a searchable portal.

```text
BOOT short press → record WAV → SD + queue(pending)
Wi-Fi + API key  → STT → transcript + status ready
Phone on LAN     → GET /api/notes + audio playback
```

## Modules (firmware)

| Module | Responsibility |
|--------|----------------|
| `config` | Merge NVS + optional SD `/inote/config.json`; never log secrets |
| `storage` | SD paths, `index.json`, `meta.json`, WAV/transcript files |
| `queue` | Pending → transcribing → ready / error; retry on reconnect |
| `audio` | ES8311 I2S capture → 16 kHz mono 16-bit WAV |
| `display` | E-paper UI state machine (idle / recording / syncing / error) |
| `wifi_setup` | SoftAP `INote-Setup` captive portal; STA with up to 5 SSIDs |
| `stt` | HTTPS multipart upload to OpenAI; write transcript |
| `web` | Static UI + REST API from SD / LittleFS |
| `power` | BAT latch (GPIO17), PWR button, light sleep hooks |

## Note lifecycle

```text
recording → pending → transcribing → ready
                              ↘ error (retry later)
```

## On-disk layout (microSD, FAT32)

```text
/inote/
  config.json                 # optional override of NVS
  index.json                  # ordered note ids + status summary
  notes/<id>/meta.json
  notes/<id>/audio.wav
  notes/<id>/transcript.txt
```

### `meta.json`

| Field | Type | Notes |
|-------|------|--------|
| `id` | string | UUID-like / time-based id |
| `createdAt` | string | ISO-8601 UTC |
| `updatedAt` | string | ISO-8601 UTC |
| `audioPath` | string | Relative path under `/inote` |
| `transcript` | string | Full text (also mirrored in `transcript.txt`) |
| `status` | string | `recording` \| `pending` \| `transcribing` \| `ready` \| `error` |
| `tags` | string[] | Empty in v0.1 unless organize enabled |
| `type` | string \| null | `note` \| `task` \| `appointment` \| null |
| `error` | string \| null | Last failure message |
| `durationMs` | number | Recording length |

### `index.json`

```json
{
  "version": 1,
  "notes": [
    { "id": "20260325T120000Z", "status": "ready", "createdAt": "..." }
  ]
}
```

## Config merge order

1. Built-in defaults  
2. NVS (from SoftAP setup)  
3. SD `/inote/config.json` overrides (if present)

## Secrets

- API key and Wi-Fi passwords: NVS and/or SD config — **never** committed  
- Use [`config.example.json`](../config.example.json) as the template  

## Web API (v0.1)

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/health` | Device status |
| GET | `/api/notes` | List from index (+ meta summaries) |
| GET | `/api/notes/:id` | Full meta |
| GET | `/api/notes/:id/audio` | WAV file |
| GET | `/` | Static portal |

## i18n

String table with `tr` (primary) and `en`. UI language from `config.ui.language`.
