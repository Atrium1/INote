# INote e-paper flow simulator

PC mock of the **200×200** on-device UI. Flow only — not GxEPD2, audio, SD, or Wi-Fi.

Source of truth: [`docs/gestures.md`](../../docs/gestures.md). Layout cues from `DisplayUi.cpp`.

**Type:** On device, screens use Adafruit GFX **FreeSans** / FreeSansBold (not FreeMono). The panel approximates Pala-like INote chrome (lowercase headers, hairlines, round menu chips, Home TODAY hero); device uses FreeSans, sim uses **IBM Plex Sans**. Page chrome outside the canvas is unchanged.

## Run

```bash
cd tools/epaper-sim
python -m http.server 8090
```

Open http://127.0.0.1:8090/

## Try this path

1. **BOOT Short** → Record (timer ticks)  
2. **BOOT Hold ~1s** → marker flash  
3. **BOOT Short** → Tag → cycle with BOOT Short → **PWR Short** confirm → Home  
4. **BOOT Hold ~1s** → Menu → **PWR Short** Notes  
5. Notes: **PWR Short** play · **PWR Hold ~2s** delete confirm · **BOOT Hold ~1s** back Menu  
6. Menu → Sync / Settings; Home **PWR Short** → sleep face → **Wake**

Demo checkboxes: **SD OK** / **Has API key** affect Home status lines.

## Gaps vs firmware

- Sync connect always succeeds (no real STA); device may show `Wi-Fi fail` / `No SSID`.
- Soft power-off and deep sleep both show the same sleep face here.
- SoftAP **Setup** is a static screen; no captive portal.
