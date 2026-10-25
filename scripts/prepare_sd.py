#!/usr/bin/env python3
"""Build an SD card image tree: inote/web + optional config.json template."""

from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Prepare INote SD card folder layout")
    parser.add_argument(
        "--out",
        type=Path,
        default=root / "sd_image",
        help="Output directory (default: ./sd_image)",
    )
    parser.add_argument(
        "--force-config",
        action="store_true",
        help="Overwrite inote/config.json from config.example.json",
    )
    args = parser.parse_args()

    web_src = root / "web"
    example = root / "config.example.json"
    if not web_src.is_dir():
        print(f"Missing web/: {web_src}", file=sys.stderr)
        return 1
    if not example.is_file():
        print(f"Missing {example}", file=sys.stderr)
        return 1

    out = args.out.resolve()
    inote = out / "inote"
    web_dst = inote / "web"
    web_dst.mkdir(parents=True, exist_ok=True)
    for name in ("index.html", "app.js", "styles.css", "format.js"):
        src = web_src / name
        if src.is_file():
            shutil.copy2(src, web_dst / name)
    fixtures = web_src / "fixtures"
    if fixtures.is_dir():
        dst_fx = web_dst / "fixtures"
        if dst_fx.exists():
            shutil.rmtree(dst_fx)
        shutil.copytree(fixtures, dst_fx)

    cfg_dst = inote / "config.json"
    if args.force_config or not cfg_dst.exists():
        shutil.copy2(example, cfg_dst)
        print(f"Wrote {cfg_dst} (edit Wi-Fi + openaiApiKey)")
    else:
        print(f"Kept existing {cfg_dst}")

    (inote / "notes").mkdir(exist_ok=True)
    (inote / "asks").mkdir(exist_ok=True)
    (inote / "briefings").mkdir(exist_ok=True)

    print(f"SD tree ready at: {out}")
    print("Copy the contents of this folder onto a FAT32 microSD so that /inote/ exists at the card root.")
    print("Then: SoftAP INote-Setup, or Menu > Sync > connect; portal at http://<device-ip>/")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
