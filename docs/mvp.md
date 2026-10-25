# MVP scope

## v0.1 (shipped base)

- BOOT short press: start/stop recording  
- 16 kHz mono 16-bit WAV on SD  
- Queue `pending` → OpenAI STT → `ready`  
- SoftAP `INote-Setup`, up to 5 Wi-Fi networks  
- LAN portal: list, search, play/download  
- TR/EN UI strings  

## v0.2 (this pass)

- Optional organize (`features.organize`): classify `note` / `task` / `appointment`  
- Meta: `title`, `dueAt`, `done`, `organizedAt`, `organizeError`  
- Portal: type filters, task done via `POST /api/notes/:id/done`  
- E-paper idle: open task count + next `dueAt`  
- Idle sleep: sticky “INote” face → EPD off → deep sleep (BOOT/PWR wake)  
- Short PWR → sleep; long PWR → BAT latch soft-off  

## Still out of scope

- TTS / spoken briefings / Q&A  
- Recording markers  
- Calendar month UI  
- Cloud sync / BLE  
