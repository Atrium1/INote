# Manual test plan — v0.4.1 / v1-rc software

## PC (no board)

1. `cd web && python -m http.server 8080` — open library; filter by tag chips; search; open player; delete in fixture mode; confirm list updates.
2. `python scripts/check_fixtures.py` — exit 0.
3. `python scripts/prepare_sd.py --out ./sd_image` — tree has `inote/web/` + `config.json`.
4. `cd firmware && pio run` — build SUCCESS.

## On device (when hardware available)

1. Flash; confirm Home shows note count (or `SD fail` if card missing).
2. Settings: cycle Sound / Language / Idle / Organize; reboot; values persist.
3. Record → Tag → confirm; note has `#NNN` + tag.
4. Notes: BOOT hold → Menu; PWR long → delete confirm; cancel / confirm.
5. Sync: connect fail shows `Wi-Fi fail` or `No SSID`; success shows IP; portal loads.
6. Legacy card without displayId: reboot once; notes gain `#NNN` / `tag=note`.
7. Pending STT without API key: Home shows `No API key`.
