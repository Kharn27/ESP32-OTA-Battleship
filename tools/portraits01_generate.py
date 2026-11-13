#!/usr/bin/env python3
"""Issue #128 TEST-ONLY: freeze candidate PNGs from PR #127 and generate Flash C.
Unapproved draft/ portraits stay unchanged; no firmware PNG decoder.
"""
import argparse
import hashlib
import json
from pathlib import Path
from PIL import Image

DIR = "draft/portraits/pilote_A_L1_C1"
HEAD = "dab0a35a1a891ab4d215a55cd16f2fe095a558df"
SOURCES = (
    (32, "2d17cc05d8bfb64a0c01b5f4ad125979dc7c5de64d304ec52fafa992217f86e7"),
    (48, "5abbcacd3981e6b426c93542dcb6289712afc4ca49b7f38dc5c35c9345360dee"),
    (64, "2241bf1306772bbca3164356529a4fb8290f9ef33ae89aaa9b29d083572967da"),
)
PC_SHA = "10b30aff6e089c1365dc0a6c0271ab8598b4fa8abb5ec5be085da7867998fc4e"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def freeze(root):
    candidates = []
    for n, expected in SOURCES:
        source = root / DIR / ("pilote_%d.png" % n)
        raw = source.read_bytes()
        sha = digest(raw)
        if sha != expected:
            raise ValueError("PORTRAITS01_SOURCE_DRIFT %s sha256=%s expected=%s" %
                             (source, sha, expected))
        with Image.open(source) as im:
            if im.format != "PNG" or im.mode != "RGBA" or im.size != (n, n):
                raise ValueError("Incorrect PNG format/mode/dimensions: %s" % source)
            rgba = im.tobytes()
        packed = bytearray()
        alpha = [0, 0, 0]
        for i in range(0, len(rgba), 4):
            r, g, b, a = rgba[i:i + 4]
            rgb = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            packed.extend((rgb >> 8, rgb & 255, a))
            alpha[0 if a == 0 else 2 if a == 255 else 1] += 1
        if alpha[0] == 0 or alpha[1] == 0:
            raise ValueError("Missing alpha class: %s alpha=%s" % (source, alpha))
        candidates.append((n, sha, bytes(packed), alpha))
    board = root / DIR / "comparatif_320x240.png"
    if digest(board.read_bytes()) != PC_SHA:
        raise ValueError("PORTRAITS01_PC_BOARD_DRIFT; review graphic HEAD")
    with Image.open(board) as im:
        if im.format != "PNG" or im.mode != "RGBA" or im.size != (320, 240):
            raise ValueError("Invalid native PC comparison board")
    return candidates


def generate(candidates):
    lines = ["/* GENERATED #128 TEST ONLY from 3 CANDIDATE PNGs; not approved assets. */",
             '#include "portraits01.h"']
    for n, _, data, _ in candidates:
        lines.append("static const uint8_t p%d[%d] = {" % (n, len(data)))
        for i in range(0, len(data), 32):
            lines.append("    " + ",".join(str(b) for b in data[i:i + 32]) + ",")
        lines.append("};")
    lines.append("const Portrait01Sprite portrait01_sprites[P01_COUNT] = {")
    for n, sha, _, _ in candidates:
        lines.append('    {p%d, sizeof p%d, %du, %du, "%s"},' %
                     (n, n, n, n, sha))
    lines.extend(["};", ""])
    return "\n".join(lines).encode("ascii")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    candidates = freeze(args.repo.resolve())
    output = generate(candidates)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_bytes() != output:
        args.output.write_bytes(output)
    metrics = {
        "source_pr_head": HEAD, "status": "CANDIDATE_ON_DEVICE_UNVERIFIED",
        "pixel_format": "RGB565BE+straight_alpha8",
        "flash_bytes": sum(len(item[2]) for item in candidates),
        "board_png_sha256": PC_SHA, "generated_c_sha256": digest(output),
        "sprites": [
            {"dimension": n, "png_sha256": sha, "flash_bytes": len(data),
             "flash_sha256": digest(data), "alpha_0_mid_255": alphas}
            for n, sha, data, alphas in candidates],
    }
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(metrics, sort_keys=True, indent=2) + "\n")
    print("PORTRAITS01_EXPORT " + json.dumps(metrics, sort_keys=True))


if __name__ == "__main__":
    main()
