#!/usr/bin/env python3
"""Smoke-check web/fixtures/notes.json for displayId + tag schema."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

VALID_TAGS = {"note", "work", "idea", "buy", "private"}
DISPLAY_RE = re.compile(r"^#\d{3}$")


def main() -> int:
    path = Path(__file__).resolve().parents[1] / "web" / "fixtures" / "notes.json"
    if not path.is_file():
        print(f"Missing {path}", file=sys.stderr)
        return 1
    data = json.loads(path.read_text(encoding="utf-8"))
    notes = data.get("notes")
    if not isinstance(notes, list) or not notes:
        print("notes[] empty or missing", file=sys.stderr)
        return 1
    errors = 0
    for i, n in enumerate(notes):
        nid = n.get("id")
        did = n.get("displayId")
        tag = n.get("tag")
        if not nid:
            print(f"[{i}] missing id")
            errors += 1
        if not isinstance(did, str) or not DISPLAY_RE.match(did):
            print(f"[{i}] bad displayId: {did!r}")
            errors += 1
        if tag not in VALID_TAGS:
            print(f"[{i}] bad tag: {tag!r}")
            errors += 1
        if "status" not in n:
            print(f"[{i}] missing status")
            errors += 1
    if errors:
        print(f"FAILED: {errors} issue(s)", file=sys.stderr)
        return 1
    print(f"OK: {len(notes)} notes ({path.name})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
