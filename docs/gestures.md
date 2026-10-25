# INote gestures

Buttons: **BOOT** = GPIO0, **PWR** = GPIO18.

Wake from deep sleep (PWR or BOOT) always lands on **Home** (does not auto-start recording).

| Context | Gesture | Action |
|---------|---------|--------|
| Sleep | PWR or BOOT wake | Boot to **Home** |
| Home | BOOT short | Start **Record** |
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
| Notes list | PWR short | Play / pause via speaker |
| Notes list | BOOT hold ~1s | Delete confirm; PWR short confirms; BOOT short cancels; hold again → Menu |
| Tags browse | BOOT short | Next note, then next tag filter |
| Tags browse | PWR short | Play selected |
| Tags | BOOT hold ~1s | Back to Menu |
| Sync | PWR short | Wi-Fi connect / disconnect (portal starts when connected) |
| Sync | BOOT hold ~1s | Back to Menu |
| Settings | BOOT short | Cycle Sound / Transfer / Device |
| Settings | PWR short | Toggle or stub action |
| Settings | BOOT hold ~1s | Back to Menu |

Idle timeout (`power.idleSleepSeconds`) also deep-sleeps from **Home** when idle.
