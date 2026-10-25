# MVP v0.1 (frozen)

## In scope

- BOOT short press: start/stop recording  
- Save **16 kHz mono 16-bit WAV** to SD under `/inote/notes/<id>/`  
- Queue status `pending` when recording ends  
- E-paper: idle (note count, Wi-Fi/battery icons), recording (timer), syncing, error  
- SoftAP captive portal `INote-Setup` when no valid Wi-Fi config  
- Persist up to **5** Wi-Fi networks + OpenAI API key (NVS; SD override optional)  
- Background worker: transcribe pending notes when STA connected and key present  
- LAN web UI: list, search transcripts, play/download audio, show status  
- UI strings: Turkish primary, English secondary  
- `features.organize` flag present but **default off** (no auto task/appointment in v0.1)

## Out of scope

- TTS / speaker playback  
- Daily/weekly briefings  
- Q&A over notes  
- Calendar UI  
- BLE  
- Cloud sync  
- Touch-panel board support  
- Realtime streaming transcription  

## Acceptance checks (with hardware)

1. Record offline → WAV on SD, e-paper shows recording then idle  
2. Insert Wi-Fi + key → pending note becomes `ready` with Turkish transcript  
3. Phone on same LAN opens portal, searches text, plays audio  
4. Long-press BOOT enters setup AP when needed  
