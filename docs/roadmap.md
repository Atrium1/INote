# Roadmap

## v0.1 — MVP (current prep)

Capture, SD storage, SoftAP Wi-Fi, OpenAI transcription, LAN portal, e-paper status. See [mvp.md](mvp.md).

## v0.2 — Light organization

- Enable `features.organize`  
- After transcription, one GPT call classifies `type`: `note` | `task` | `appointment`  
- Optional due/when fields in `meta.json`  
- E-paper home: open tasks + next appointment snippet  
- Portal: filter by type; mark task done  

## v0.3 — Recall + voice out

- Hold-to-ask style query over local transcripts (RAG-lite: recent notes context)  
- Daily / weekly briefing as **text** first  
- Then TTS + ES8311 speaker path  
- Usage caps / confirm-before-expensive calls  

## Later ideas (unscheduled)

- Multi-language auto-detect  
- BLE provisioning alternative  
- Case STL / 3D print files  
- Battery life profiling and deep-sleep schedule  
