# LittleFS / flash data (optional)

v0.1 serves the portal from microSD at `/inote/web/` (copy the repo `web/` folder there) or falls back to a minimal embedded HTML page.

To embed assets in flash later:

```bash
pio run -t buildfs
pio run -t uploadfs
```

Not required for the pre-hardware prep phase.
