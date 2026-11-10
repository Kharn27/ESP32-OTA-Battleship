#!/usr/bin/env python3
"""D14 TEST-ONLY: reproduce EXACT C-generated RGB565BE midtones on PC.
No alternative RGB source, no color correction, no gamma or saturation.
"""
import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image

WIDTH = 320
HEIGHT = 240
BYTE_COUNT = WIDTH * HEIGHT * 2


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--raw", type=Path, required=True)
    p.add_argument("--png", type=Path, required=True)
    p.add_argument("--report", type=Path, required=True)
    args = p.parse_args()
    src = args.raw.read_bytes()
    if len(src) != BYTE_COUNT:
        raise ValueError(f"bad RGB565BE length: {len(src)} != {BYTE_COUNT}")
    out = bytearray(WIDTH * HEIGHT * 3)
    for i in range(WIDTH * HEIGHT):
        word = (src[2 * i] << 8) | src[2 * i + 1]
        out[3 * i] = ((word >> 11) & 31) << 3
        out[3 * i + 1] = ((word >> 5) & 63) << 2
        out[3 * i + 2] = (word & 31) << 3
    args.png.parent.mkdir(parents=True, exist_ok=True)
    Image.frombytes("RGB", (WIDTH, HEIGHT), bytes(out)).save(args.png)
    samples = {}
    for name, x, y in (
        ("sky", 0, 120), ("sea", 80, 120),
        ("bronze", 160, 120), ("logo", 240, 120),
        ("orange_mid", 40, 60), ("red_mid", 120, 60),
        ("blue_mid", 200, 60), ("gray_mid", 280, 60),
    ):
        pos = 2 * (y * WIDTH + x)
        samples[name] = {
            "xy": [x, y],
            "rgb565be_hex": src[pos:pos + 2].hex(),
            "rgb888_from_565": list(out[3 * (y * WIDTH + x):3 * (y * WIDTH + x + 1)]),
        }
    report = {
        "type": "D14_DOOM_REFERENCE_PC_ORACLE_TEST_ONLY",
        "image_origin": "C99_D14_DOOM_MIDTONE_BANDS_RAW565BE",
        "size": [WIDTH, HEIGHT],
        "bytes": BYTE_COUNT,
        "sha256_rgb565be": hashlib.sha256(src).hexdigest(),
        "pixel_transform": "RGB565BE bit extraction, left shifts 3/2/3 ONLY",
        "artistic_transform": "NONE",
        "samples": samples,
    }
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"MIDTONES_PC_REF PASS bytes={BYTE_COUNT} sha256={report['sha256_rgb565be']}")


if __name__ == "__main__":
    main()
