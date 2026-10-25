# MVP scope

## v0.1

Capture, STT queue, SoftAP, LAN list/search/playback, TR/EN strings.

## v0.2

Organize (optional), portal type filters + task done, idle deep sleep with sticky INote face.

## v0.3

Portal calendar/tasks, recording markers, date/time formats + `de`, text ask/briefing.

## v0.4 (device UX + portal)

- On-device Home / Record / Tag / Menu (Notes, Tags, Sync, Settings)
- Manual tags `note|work|idea|buy|private` and `#NNN` display ids
- ES8311 speaker playback + delete on device and `DELETE /api/notes/:id`
- Sync screen Wi-Fi connect/disconnect; portal when connected
- Library-first LAN portal (tag chips, player, delete)
- Pins reconciled with Waveshare GitHub examples

## v0.4.1 (v1-rc software)

- Real Settings: Sound / Language / Idle / Organize (NVS persist)
- Notes: BOOT hold → Menu; PWR long → delete
- Legacy note migration (`displayId` + `tag`); error/empty UX; SD prepare + fixture smoke
- TLS documented as `setInsecure` for bring-up

## Still out of scope

- Native phone app / BLE
- Cloud sync
- Perfect audio EQ / TTS voice-out (roadmap)
