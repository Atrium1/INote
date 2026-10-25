#!/usr/bin/env python3
"""PC mock: ask a question over sample/ready note transcripts."""

from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path

import requests

API_URL = "https://api.openai.com/v1/chat/completions"


def load_config(path: Path | None) -> dict:
    for p in [path, Path(__file__).parent / "config.json", Path(__file__).parent.parent / "config.json"]:
        if p and Path(p).is_file():
            return json.loads(Path(p).read_text(encoding="utf-8"))
    return {}


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--question", required=True)
    ap.add_argument("--notes", type=Path, help="JSON array or {notes:[...]} fixture")
    ap.add_argument("--config", type=Path)
    ap.add_argument("--model", default=None)
    args = ap.parse_args()

    cfg = load_config(args.config)
    key = os.environ.get("OPENAI_API_KEY") or cfg.get("openaiApiKey") or ""
    if not key or key.startswith("sk-..."):
        raise SystemExit("Set OPENAI_API_KEY or openaiApiKey")
    model = args.model or (cfg.get("ask") or {}).get("model") or "gpt-4o-mini"

    notes = []
    if args.notes and args.notes.is_file():
        raw = json.loads(args.notes.read_text(encoding="utf-8"))
        notes = raw.get("notes", raw) if isinstance(raw, dict) else raw
    ctx_lines = []
    for n in notes[:20]:
        if n.get("status") and n.get("status") != "ready":
            continue
        ctx_lines.append(
            f"- [{n.get('id')}] type={n.get('type') or 'note'} title={n.get('title')} "
            f"due={n.get('dueAt')} done={n.get('done')}\n  {n.get('transcript') or ''}"
        )
    ctx = "\n".join(ctx_lines)[:12000]

    body = {
        "model": model,
        "temperature": 0.2,
        "messages": [
            {
                "role": "system",
                "content": "Answer using only the provided personal notes context. "
                "If unknown, say you do not know. Reply in the user's language. Plain text only.",
            },
            {"role": "user", "content": f"Context:\n{ctx}\n\nQuestion:\n{args.question}"},
        ],
    }
    resp = requests.post(
        API_URL,
        headers={"Authorization": f"Bearer {key}", "Content-Type": "application/json"},
        json=body,
        timeout=90,
    )
    if resp.status_code >= 400:
        raise SystemExit(f"{resp.status_code}: {resp.text}")
    answer = resp.json()["choices"][0]["message"]["content"].strip()
    print(json.dumps({"answer": answer, "text": answer}, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
