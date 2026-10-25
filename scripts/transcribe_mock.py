#!/usr/bin/env python3
"""PC mock: WAV → OpenAI transcription → INote meta.json layout.

Usage:
  set OPENAI_API_KEY=sk-...
  python transcribe_mock.py path/to/audio.wav [--out DIR] [--config path]

Or put openaiApiKey in config.json (see config.example.json).
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import uuid
import wave
from datetime import datetime, timezone
from pathlib import Path

import requests

DEFAULT_MODEL = "gpt-transcribe"
API_URL = "https://api.openai.com/v1/audio/transcriptions"


def utc_now() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def note_id() -> str:
    return datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "-" + uuid.uuid4().hex[:6]


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
            with p.open(encoding="utf-8") as f:
                return json.load(f)
    return {}


def wav_duration_ms(path: Path) -> int:
    try:
        with wave.open(str(path), "rb") as w:
            frames = w.getnframes()
            rate = w.getframerate() or 1
            return int(frames * 1000 / rate)
    except wave.Error:
        return 0


def transcribe(wav_path: Path, api_key: str, model: str, language: str) -> str:
    headers = {"Authorization": f"Bearer {api_key}"}
    data = {"model": model}
    if language:
        data["language"] = language
    with wav_path.open("rb") as audio:
        files = {"file": (wav_path.name, audio, "audio/wav")}
        resp = requests.post(API_URL, headers=headers, data=data, files=files, timeout=120)
    if resp.status_code >= 400:
        raise SystemExit(f"OpenAI error {resp.status_code}: {resp.text}")
    payload = resp.json()
    text = payload.get("text")
    if text is None:
        raise SystemExit(f"Unexpected response: {payload}")
    return text.strip()


def write_note(out_root: Path, nid: str, wav_src: Path, transcript: str, duration_ms: int) -> Path:
    note_dir = out_root / "inote" / "notes" / nid
    note_dir.mkdir(parents=True, exist_ok=True)

    audio_dest = note_dir / "audio.wav"
    if wav_src.resolve() != audio_dest.resolve():
        audio_dest.write_bytes(wav_src.read_bytes())

    (note_dir / "transcript.txt").write_text(transcript + "\n", encoding="utf-8")

    now = utc_now()
    meta = {
        "id": nid,
        "createdAt": now,
        "updatedAt": now,
        "audioPath": f"notes/{nid}/audio.wav",
        "transcript": transcript,
        "status": "ready",
        "tags": [],
        "type": None,
        "error": None,
        "durationMs": duration_ms,
    }
    (note_dir / "meta.json").write_text(json.dumps(meta, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    index_path = out_root / "inote" / "index.json"
    index = {"version": 1, "notes": []}
    if index_path.is_file():
        index = json.loads(index_path.read_text(encoding="utf-8"))
    index.setdefault("notes", [])
    index["notes"] = [n for n in index["notes"] if n.get("id") != nid]
    index["notes"].insert(0, {"id": nid, "status": "ready", "createdAt": now})
    index_path.parent.mkdir(parents=True, exist_ok=True)
    index_path.write_text(json.dumps(index, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return note_dir


def main() -> None:
    parser = argparse.ArgumentParser(description="INote mock STT pipeline")
    parser.add_argument("wav", type=Path, help="Input WAV file")
    parser.add_argument("--out", type=Path, default=Path(__file__).resolve().parent / "out")
    parser.add_argument("--config", type=Path, default=None)
    parser.add_argument("--model", default=None)
    parser.add_argument("--language", default=None)
    args = parser.parse_args()

    if not args.wav.is_file():
        raise SystemExit(f"WAV not found: {args.wav}")

    cfg = load_config(args.config)
    stt = cfg.get("stt") or {}
    api_key = os.environ.get("OPENAI_API_KEY") or cfg.get("openaiApiKey") or ""
    if not api_key or api_key.startswith("sk-..."):
        raise SystemExit("Set OPENAI_API_KEY or openaiApiKey in config.json")

    model = args.model or stt.get("model") or DEFAULT_MODEL
    language = args.language if args.language is not None else stt.get("language", "tr")

    print(f"Transcribing {args.wav} with {model} (lang={language})...", file=sys.stderr)
    text = transcribe(args.wav, api_key, model, language)
    nid = note_id()
    duration_ms = wav_duration_ms(args.wav)
    note_dir = write_note(args.out, nid, args.wav, text, duration_ms)
    print(json.dumps({"id": nid, "dir": str(note_dir), "transcript": text}, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
