#!/usr/bin/env python3
"""Independent PC oracle for generated C renderer #128; no source changes.
Compare C RGB565BE output against the frozen graphic PR #127 PC RGBA board.
Quantization in 5/6/5 causes bounded deviations; do not assert RGB888 exact.
"""
import argparse
import hashlib
import json
from pathlib import Path
from PIL import Image

EXPECTED = "10b30aff6e089c1365dc0a6c0271ab8598b4fa8abb5ec5be085da7867998fc4e"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dump", type=Path, required=True)
    ap.add_argument("--board", type=Path, required=True)
    ap.add_argument("--report", type=Path)
    args = ap.parse_args()
    data = args.dump.read_bytes()
    assert len(data) == 320 * 240 * 2, "non-native or incomplete band output"
    assert hashlib.sha256(args.board.read_bytes()).hexdigest() == EXPECTED, "PC board drift"
    with Image.open(args.board) as img:
        assert img.size == (320, 240) and img.mode == "RGBA"
        reference = img.convert("RGB").tobytes()
    delta_max, total, wrong = 0, 0, 0
    examples = []
    for i in range(320 * 240):
        u = (data[2 * i] << 8) | data[2 * i + 1]
        rgb = (((u >> 11) & 31) * 255 // 31,
               ((u >> 5) & 63) * 255 // 63, (u & 31) * 255 // 31)
        orig = reference[3*i:3*i+3]
        d = [abs(a-b) for a,b in zip(rgb,orig)]
        current = max(d)
        if current:
            wrong += 1
            if current >= 12 and len(examples) < 12:
                examples.append({"xy": [i % 320, i // 320], "rgb": rgb,
                                 "reference": list(orig), "max_delta": current})
        delta_max = max(delta_max,current)
        total += sum(d)
    report = {
        "render": "actual C99 RGB565BE output with real candidate pixels",
        "reference": "frozen PR #127 320x240 RGBA PC board",
        "source_sha256": EXPECTED,
        "different_pixels_rgb888": wrong,
        "max_abs_channel": delta_max,
        "mean_abs_rgb_channel": round(total/(320*240*3),6),
        "outliers": examples,
    }
    if args.report:
        args.report.parent.mkdir(parents=True,exist_ok=True)
        args.report.write_text(json.dumps(report,sort_keys=True,indent=2)+"\n")
    print("PORTRAITS01_PC_COMPARISON " + json.dumps(report,sort_keys=True))
    # Source RGB is quantized twice (source and background in 565), and
    # alpha is interpolated in 5/6/5 precision. 11 is conservative.
    assert delta_max <= 11, "PC comparison divergence: check alpha/layout/original PNG"


if __name__ == "__main__":
    main()
