# INote gestures

Buttons: **BOOT** = GPIO0, **PWR** = GPIO18.

Wake from deep sleep (PWR or BOOT) always lands on **Home** (does not auto-start recording).

| Context | Gesture | Action |
|---------|---------|--------|
| Sleep | PWR or BOOT wake | Boot to **Home** |
| Home | BOOT short | Start **Record** (needs SD) |
| Home | BOOT hold ~1s | Open **Menu** |
| Home | BOOT hold ~3s | SoftAP setup (`INote-Setup`) |
| Home | PWR short | Deep sleep (INote face) |
| Home | PWR long ~2s | Soft power-off (battery latch) |
| Record | BOOT short | Stop → **Tag** picker |
| Record | BOOT hold ~1s | Add marker |
| Tag | BOOT short | Cycle `note → work → idea → buy → private` |
| Tag | PWR short | Confirm tag → Home |
| Menu | BOOT short | Next item (Notes / Tags / Sync / Settings) |
| Menu | PWR short | Enter item |
| Menu | BOOT hold ~1s | Back to Home |
| Notes list | BOOT short | Next note |
| Notes list | PWR short | Play / pause (if Sound ON) |
| Notes list | BOOT hold ~1s | **Back to Menu** (stop playback) |
| Notes list | PWR long ~2s | **Delete confirm** |
| Notes (confirm) | PWR short | Confirm delete |
| Notes (confirm) | BOOT short | Cancel confirm |
| Notes (confirm) | BOOT hold ~1s | Cancel → Menu |
| Tags browse | BOOT short | Next note, then next tag filter |
| Tags browse | PWR short | Play selected |
| Tags | BOOT hold ~1s | Back to Menu |
| Sync | PWR short | Wi-Fi connect / disconnect (portal when connected) |
| Sync | BOOT hold ~1s | Back to Menu |
| Settings | BOOT short | Cycle Sound / Language / Idle / Organize |
| Settings | PWR short | Toggle or cycle value; **persist immediately** |
| Settings | BOOT hold ~1s | Back to Menu |

**Settings values:** Sound ON/OFF · Language tr/en/de · Idle 30/45/90/OFF · Organize ON/OFF.

Idle timeout (`power.idleSleepSeconds`, 0 = off) also deep-sleeps from **Home** when idle.

Note: Notes delete used to be BOOT hold; v0.4.1 uses **PWR long** so BOOT hold matches other lists (back to Menu).
