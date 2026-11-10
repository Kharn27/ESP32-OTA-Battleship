#!/usr/bin/env python3
"""Catalog and verify FLOT-01 INTACT H/V with non-runtime ×4 NEAREST previews.

Artistic GO: PR #97 comment #6766809070 (R2 only).
The 54×18 and 18×54 canonical PNGs remain byte-for-byte unchanged.
Requires Pillow==12.3.0. No TFT/radio/runtime validation implied.
"""
from __future__ import annotations

import argparse
import hashlib
import io
import json
from pathlib import Path

import PIL
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ASSET_DIR = ROOT / "assets/flotte/sous-marin"
MANIFEST = ROOT / "assets/manifest.json"
PO_REF = "https://github.com/Kharn27/ESP32-OTA-Battleship/pull/97#issuecomment-6766809070"
ORIGINALS = {
    "h": ("sous_marin_intact_h.png", (54, 18),
          "e6c6639e37edbc040fba4a3f04a70f588fab8d4cba66c290c1f2beaf362f1425"),
    "v": ("sous_marin_intact_v.png", (18, 54),
          "0a4203cbfb9ea152fe9610732ca2fdd71e0d734d2aab2272fc4ffed3941629cb"),
}


def sha256(blob: bytes) -> str:
    return hashlib.sha256(blob).hexdigest()


def image(blob: bytes, dimensions: tuple[int, int]) -> Image.Image:
    with Image.open(io.BytesIO(blob)) as src:
        src.load()
        if src.format != "PNG" or src.mode != "RGBA" or src.size != dimensions:
            raise RuntimeError(f"Expected native PNG RGBA {dimensions}, got {src.format}/{src.mode}/{src.size}")
        im = src.copy()
    values = im.getchannel("A").getdata()
    alpha = list(values)
    if alpha.count(0) != 486 or alpha.count(255) != 486 or len(alpha) != 972:
        raise RuntimeError(f"Unexpected binary alpha mask: {dimensions}")
    return im


def make_preview(im: Image.Image) -> bytes:
    x4 = im.resize((im.width * 4, im.height * 4), Image.Resampling.NEAREST)
    # Explicitly prove identical RGBA quadruple pixels, never anti-aliased.
    for py in range(im.height):
        for px in range(im.width):
            pixel = im.getpixel((px, py))
            for dy in range(4):
                for dx in range(4):
                    if x4.getpixel((px * 4 + dx, py * 4 + dy)) != pixel:
                        raise RuntimeError("Pixel mismatch in nearest-neighbor 4×")
    buf = io.BytesIO()
    x4.save(buf, format="PNG", optimize=True)
    return buf.getvalue()


def catalog_record(identifier: str, path: str, blob: bytes,
                   source: str, dimensions: list[int], usage: str) -> dict:
    return {
        "id": identifier,
        "path": "assets/flotte/sous-marin/" + path,
        "sha256": sha256(blob),
        "source_ref": source,
        "po_validation_ref": PO_REF,
        "dimensions": dimensions,
        "usage": usage,
        "device_status": "ON_DEVICE_UNVERIFIED",
    }


def build() -> tuple[dict[str, bytes], list[dict]]:
    if PIL.__version__ != "12.3.0":
        raise RuntimeError("Pillow==12.3.0 required")
    native: dict[str, Image.Image] = {}
    files: dict[str, bytes] = {}
    records: list[dict] = []
    for orientation, (name, dimensions, expected) in ORIGINALS.items():
        data = (ASSET_DIR / name).read_bytes()
        if sha256(data) != expected:
            raise RuntimeError(f"Native approved image changed: {name}")
        native[orientation] = image(data, dimensions)
        records.append(catalog_record(
            f"flotte.sous-marin.intact.{orientation}", name, data,
            f"assets/flotte/sous-marin/sources/sous_marin_intact_{orientation}.svg",
            list(dimensions), "RUNTIME_SPRITE_NOT_INTEGRATED"))
        preview = f"sous_marin_intact_{orientation}_x4.png"
        files[preview] = make_preview(native[orientation])
        records.append(catalog_record(
            f"flotte.sous-marin.intact.{orientation}.x4", preview, files[preview],
            "assets/flotte/sous-marin/" + name,
            [dimensions[0] * 4, dimensions[1] * 4], "INSPECTION_ONLY_NOT_RUNTIME"))
        src = f"sources/sous_marin_intact_{orientation}.svg"
        records.append(catalog_record(
            f"flotte.sous-marin.source.{orientation}", src,
            (ASSET_DIR / src).read_bytes(),
            "draft/sous_marin_PO_A.jpeg", list(dimensions),
            "EDITABLE_SOURCE_NOT_RUNTIME"))
    if native["h"].transpose(Image.Transpose.ROTATE_90).tobytes() != native["v"].tobytes():
        raise RuntimeError("Vertical sprite differs from canonical raster rotation of H")
    return files, records


def check(files: dict[str, bytes], records: list[dict]) -> None:
    for path, data in files.items():
        file = ASSET_DIR / path
        if not file.is_file() or file.read_bytes() != data:
            raise RuntimeError(f"Invalid, missing, or non-reproducible zoom {path}")
    doc = json.loads(MANIFEST.read_text(encoding="utf-8"))
    registered = {entry["id"]: entry for entry in doc["assets"]}
    for entry in records:
        if registered.get(entry["id"]) != entry:
            raise RuntimeError("Catalog metadata differs: " + entry["id"])
    print("FLOT_01_ASSETS_OK: native RGBA alpha/rotation unchanged; zooms ×4 NEAREST exact; 6 catalog entries")
    for entry in records:
        print(entry["path"], entry["dimensions"], entry["sha256"], entry["usage"])


def write(files: dict[str, bytes], records: list[dict]) -> None:
    for name, data in files.items():
        path = ASSET_DIR / name
        if path.exists() and path.read_bytes() != data:
            raise RuntimeError("Refusing to change existing PNG pixels: " + name)
        path.write_bytes(data)
    doc = json.loads(MANIFEST.read_text(encoding="utf-8"))
    known = {entry["id"]: entry for entry in doc["assets"]}
    for entry in records:
        if entry["id"] in known and known[entry["id"]] != entry:
            raise RuntimeError("Existing approved asset metadata differs: " + entry["id"])
        if entry["id"] not in known:
            doc["assets"].append(entry)
    MANIFEST.write_text(json.dumps(doc, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    check(files, records)


def main() -> None:
    parser = argparse.ArgumentParser()
    actions = parser.add_mutually_exclusive_group(required=True)
    actions.add_argument("--write", action="store_true")
    actions.add_argument("--check", action="store_true")
    args = parser.parse_args()
    generated, records = build()
    if args.write:
        write(generated, records)
    else:
        check(generated, records)


if __name__ == "__main__":
    main()
