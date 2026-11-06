#!/usr/bin/env python3
"""I-009 — Convertit les deux images A/B du PO, sans création graphique."""
from pathlib import Path
from PIL import Image
import hashlib
import json

root = Path(__file__).resolve().parent
result = {}
for letter in ("A", "B"):
    src = root / f"ocean_{letter}_reference_1195x896.jpeg"
    original = Image.open(src).convert("RGB")
    assert original.size == (1195, 896)
    native = original.resize((320, 240), Image.Resampling.LANCZOS)
    x4 = native.resize((1280, 960), Image.Resampling.NEAREST)
    output = root / f"ocean_{letter}_320x240.png"
    zoom = root / f"ocean_{letter}_x4.png"
    native.save(output, optimize=True)
    x4.save(zoom, optimize=True)
    assert Image.open(output).size == (320, 240)
    assert Image.open(zoom).size == (1280, 960)
    for y in range(240):
        for x in range(320):
            pixel = native.getpixel((x, y))
            assert all(x4.getpixel((x*4+dx, y*4+dy)) == pixel for dy in range(4) for dx in range(4))
    result[letter] = {
        "native_sha256": hashlib.sha256(output.read_bytes()).hexdigest(),
        "x4_sha256": hashlib.sha256(zoom.read_bytes()).hexdigest(),
        "passed": True,
    }
    print(f"PASS: ocean_{letter} 320x240 + 1280x960 ×4 NEAREST")
(root / "export_manifest.json").write_text(json.dumps(result, indent=2)+"\n", encoding="utf-8")
