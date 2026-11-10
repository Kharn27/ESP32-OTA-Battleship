#!/usr/bin/env python3
"""D14 COLOR TEST ONLY: PO-approved G01 PR90 PNG -> exact Flash RGB565BE.

No SVG/Pillow/re-rendering/gamma/palette/contrast adjustments. Source PNG
is the EXACT main PR90 Git blob. RGB565 raw is smaller AND faster than RLE
for this detailed image (measured 153600 vs 172909 bytes). No framebuffer.
"""
import argparse
import hashlib
import json
from pathlib import Path

from t03_export import (W, H, c_array, check_roundtrip, encode_rle,
                        read_png, rgb565_be)

SOURCE_SHA256 = "ca73f029e80c95c09b3c7defe1437bba3156280b2f2169465ee3f30014343873"
SOURCE_BYTES = 144890
SOURCE_BLOB_SHA = "7bd321c6ccbfc230802e20010bfa06e71a9099a9"
SOURCE_MAIN_HEAD = "8fc0a2b4947d9f74023a4d1dc44bd54d680a9847"


def render_c(raw):
    # Generated C exists ONLY under the diagnostic build directory.
    arr = c_array("g01_pr90_raw565", raw)
    arr = arr.replace("const uint8_t ", "static const uint8_t ", 1)
    return ("/* D14 G01 PO-approved PR90 PNG. Byte-preserving RGB565BE. */\n"
            "/* sha256 source: " + SOURCE_SHA256 + " */\n"
            '#include "t03_decode.h"\n'
            + arr +
            "const T03Image d14_color_g01_image = {\n"
            "  g01_pr90_raw565, sizeof(g01_pr90_raw565),\n"
            "  NULL, 0u, 320u, 240u, T03_RAW565BE\n"
            "};\n").encode("ascii")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--input", required=True)
    p.add_argument("--output", required=True)
    p.add_argument("--raw-reference", help="Host-only RGB565BE golden pixels")
    args = p.parse_args()
    rgb, png = read_png(args.input)  # exact 320x240 RGB8, CRC and filters
    actual = hashlib.sha256(png).hexdigest()
    if len(png) != SOURCE_BYTES or actual != SOURCE_SHA256:
        raise ValueError(
            f"G01 MUST be PR90 approved PNG, got {len(png)} bytes sha256={actual}")
    raw = rgb565_be(rgb)
    if len(raw) != W*H*2:
        raise ValueError("G01 RGB565 dimension mismatch")

    # Reuse T03 encoding only as an OFF-DEVICE size decision/proof.
    # Do NOT place larger RLE+row indices in the diagnostic firmware.
    rle, offsets = encode_rle(raw)
    check_roundtrip(raw, rle, offsets)
    rle_total = len(rle) + 4*len(offsets)
    if rle_total <= len(raw):
        raise ValueError("Expected PR90 raw RGB565BE to beat RLE; re-evaluate format")
    generated = render_c(raw)
    path = Path(args.output)
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_bytes() != generated:
        path.write_bytes(generated)
    if args.raw_reference:
        ref = Path(args.raw_reference)
        ref.parent.mkdir(parents=True, exist_ok=True)
        if not ref.exists() or ref.read_bytes() != raw:
            ref.write_bytes(raw)
    print(json.dumps({
        "source": "G01_PR90_PNG",
        "source_main_sha": SOURCE_MAIN_HEAD,
        "source_git_blob_sha": SOURCE_BLOB_SHA,
        "source_png_bytes": len(png),
        "source_png_sha256": actual,
        "selected_format": "RAW565BE-v1",
        "raw_rgb565_bytes": len(raw),
        "raw_rgb565_sha256": hashlib.sha256(raw).hexdigest(),
        "rle_alternative_total_bytes": rle_total,
        "raw_saved_vs_rle": rle_total-len(raw),
        "generated_c_sha256": hashlib.sha256(generated).hexdigest(),
        "pixel_equal_after_quantization": True
    }, sort_keys=True))


if __name__ == "__main__":
    main()
