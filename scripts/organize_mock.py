#!/usr/bin/env python3
"""Classify a transcript or note meta.json via OpenAI (INote organize).

Usage:
  set OPENAI_API_KEY=sk-...
  python organize_mock.py --text "Yarın dişçi randevusu saat 10"
  python organize_mock.py --meta path/to/meta.json [--force]
"""

from __future__ import annotations

import argparse
import json
import os
import sys
from datetime import datetime, timezone
from pathlib import Path

import requests

API_URL = "https://api.openai.com/v1/chat/completions"
DEFAULT_MODEL = "gpt-4o-mini"


def utc_now() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def load_config(path: Path | None) -> dict:
    candidates = []
    if path:
        candidates.append(path)
    candidates.extend(
        [
            Path(__file__).resolve().parent / "config.json",
            Path(__file__).resolve().parent.parent / "config.json",
        ]
    )
    for p in candidates:
        if p.is_file():
            return json.loads(p.read_text(encoding="utf-8"))
    return {}


def classify(transcript: str, api_key: str, model: str, now: str) -> dict:
    system = (
        "Classify a personal voice note. Reply JSON only with keys: "
        "type (note|task|appointment), title (short), dueAt (ISO-8601 or null), done (false), tags ([]). "
        "Turkish or English input. If unclear use type=note and dueAt=null. "
        "Resolve relative dates using the provided now UTC. Do not invent dates without cues."
    )
    body = {
        "model": model,
        "temperature": 0,
        "response_format": {"type": "json_object"},
        "messages": [
            {"role": "system", "content": system},
            {"role": "user", "content": f"nowUtc={now}\ntranscript:\n{transcript}"},
        ],
    }
    resp = requests.post(
        API_URL,
        headers={"Authorization": f"Bearer {api_key}", "Content-Type": "application/json"},
        json=body,
        timeout=60,
    )
    if resp.status_code >= 400:
        raise SystemExit(f"OpenAI error {resp.status_code}: {resp.text}")
    content = resp.json()["choices"][0]["message"]["content"]
    return json.loads(content)


def apply_to_meta(meta: dict, result: dict, force: bool) -> dict:
    if meta.get("organizedAt") and not force:
        raise SystemExit("Already organized (pass --force)")
    typ = result.get("type") or "note"
    if typ not in ("note", "task", "appointment"):
        typ = "note"
    meta["type"] = typ
    meta["title"] = result.get("title") or None
    meta["dueAt"] = result.get("dueAt")
    meta["done"] = bool(result.get("done")) if typ == "task" else False
    meta["tags"] = result.get("tags") or []
    meta["organizedAt"] = utc_now()
    meta["organizeError"] = None
    meta["updatedAt"] = utc_now()
    return meta


def main() -> None:
    parser = argparse.ArgumentParser(description="INote organize mock")
    parser.add_argument("--text", help="Transcript text")
    parser.add_argument("--meta", type=Path, help="Path to meta.json to update")
    parser.add_argument("--config", type=Path, default=None)
    parser.add_argument("--model", default=None)
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    cfg = load_config(args.config)
    api_key = os.environ.get("OPENAI_API_KEY") or cfg.get("openaiApiKey") or ""
    if not api_key or api_key.startswith("sk-..."):
        raise SystemExit("Set OPENAI_API_KEY or openaiApiKey in config.json")
    model = args.model or (cfg.get("organize") or {}).get("model") or DEFAULT_MODEL

    meta = {}
    if args.meta:
        meta = json.loads(args.meta.read_text(encoding="utf-8"))
        text = meta.get("transcript") or ""
    else:
        text = args.text or ""
    if not text:
        raise SystemExit("Provide --text or --meta with transcript")

    now = utc_now()
    print(f"Classifying with {model}…", file=sys.stderr)
    result = classify(text, api_key, model, now)
    if args.meta:
        meta = apply_to_meta(meta, result, args.force)
        args.meta.write_text(json.dumps(meta, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(meta, ensure_ascii=False, indent=2))
    else:
        print(json.dumps(result, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
