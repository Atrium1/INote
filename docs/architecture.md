# Architecture

## Overview

```text
BOOT short → start/stop record
BOOT hold ~1s while recording → marker {tMs}
BOOT hold ~3s idle → SoftAP setup
STT → optional organize
Portal: notes / tasks / calendar / ask+briefing
Idle sleep → sticky "INote" e-ink face
```

## Modules

| Module | Responsibility |
|--------|----------------|
| `config` | NVS + SD; ui language/date/time; feature flags |
| `storage` | notes, markers, ask logs, briefing cache |
| `queue` | STT + organize |
| `stt` | Whisper-class STT, organize JSON, ask/briefing chat |
| `web` | Portal + REST |
| `display` / `power` | UI + deep sleep |

## meta.json (extra fields)

- `markers`: `[{ "tMs", "label" }]`
- organize fields: `type`, `title`, `dueAt`, `done`, `organizedAt`, `organizeError`

## SD extras

```text
/inote/asks/<id>.json          # { question, answer, text }
/inote/briefings/<key>.json    # { range, text, generatedAt }
```

## Web API

| Method | Path | Notes |
|--------|------|--------|
| GET | `/api/config` | Non-secret ui + features |
| GET | `/api/notes` | List |
| POST | `/api/notes/:id/done` | `{ done }` |
| POST | `/api/ask` | `{ question }` → `{ answer, text, id }` |
| POST | `/api/briefing` | `{ range: day\|week }` → `{ text, … }` |

Ask/briefing use last 20 ready notes (~12k chars). Flags: `features.ask`, `features.briefing`.

## UI config

- `ui.language`: `tr` \| `en` \| `de`
- `ui.dateFormat`: `DMY` \| `YMD` \| `MDY`
- `ui.timeFormat`: `H24` \| `H12`

## Power

Idle timeout / short PWR → sleep face → EPD off → deep sleep (BOOT+PWR wake). Long PWR → latch off. No sleep while recording, queue, or portal ask/briefing busy.
