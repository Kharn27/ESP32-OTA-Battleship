#!/usr/bin/env python3
"""D14 COLOR TEST ONLY: approved G01 PR#90 PNG -> exact Flash ROW-RLE565BE.

The ONLY input is the byte-for-byte PNG approved in main via PR#90.
No SVG, Pillow, resizing, saturation, gamma, palette, or alpha blending.
This generator does not modify design/ and uses Python stdlib only.
"""
import argparse
import hashlib
import json
from pathlib import Path

from t03_export import W, H, check_roundtrip, encode_rle, read_png, rgb565_be

SOURCE_SHA256 = "ca73f029e80c95c09b3c7defe1437bba3156280b2f2169465ee3f30014343873"
SOURCE_BYTES = 144890
SOURCE_BLOB_SHA = "7bd321c6ccbfc230802e20010bfa06e71a9099a9"
SOURCE_MAIN_HEAD = "8fc0a2b4947d9f74023a4d1dc44bd54d680a9847"


def render_c(rle, offsets):
    lines = [
        "/* AUTO GENERATED D14 TEST ONLY from G01 PO-approved PR90 PNG. */",
        "/* Source sha256: " + SOURCE_SHA256 + " */",
        '#include "t03_decode.h"',
        "#include <stdint.h>",
        "static const uint8_t g01_rle[" + str(len(rle)) + "] = {",
    ]
    for at in range(0, len(rle), 16):
        lines.append("    " + ", ".join("0x%02xu" % b for b in rle[at:at+16]) + ",")
    lines.extend(["};", "static const uint32_t g01_rows[241] = {"])
    for at in range(0, len(offsets), 12):
        lines.append("    " + ", ".join(str(n) + "u" for n in offsets[at:at+12]) + ",")
    lines.extend([
        "};",
        "const T03Image d14_color_g01_image = {",
        "    g01_rle, sizeof(g01_rle), g01_rows, 241u,",
        "    320u, 240u, T03_ROW_RLE565BE",
        "};",
        "",
    ])
    return "\n".join(lines).encode("ascii")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--input", required=True)
    p.add_argument("--output", required=True)
    p.add_argument("--raw-reference", help="Host-only RGB565BE golden pixels")
    args = p.parse_args()
    rgb, png = read_png(args.input)
    actual = hashlib.sha256(png).hexdigest()
    if len(png) != SOURCE_BYTES or actual != SOURCE_SHA256:
        raise ValueError(
            f"G01 MUST be PR90 approved PNG, got {len(png)} bytes sha256={actual}")
    raw = rgb565_be(rgb)
    if len(raw) != W*H*2:
        raise ValueError("G01 RGB565 dimension mismatch")
    rle, offsets = encode_rle(raw)
    check_roundtrip(raw, rle, offsets)
    generated = render_c(rle, offsets)
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
        "format": "ROW-RLE565BE-v1",
        "raw_rgb565_bytes": len(raw),
        "raw_rgb565_sha256": hashlib.sha256(raw).hexdigest(),
        "rle_bytes": len(rle),
        "offset_index_bytes": len(offsets)*4,
        "rle_total_bytes": len(rle)+len(offsets)*4,
        "generated_c_sha256": hashlib.sha256(generated).hexdigest(),
        "pixel_equal_after_quantization": True
    }, sort_keys=True))


if __name__ == "__main__":
    main()
