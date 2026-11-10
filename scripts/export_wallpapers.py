#!/usr/bin/env python3
"""Reproduce historical (PR #8) and current (PR #90) accueil artwork."""
from __future__ import annotations

import argparse
import base64
import hashlib
import io
import json
import re
import subprocess
from pathlib import Path
from xml.etree import ElementTree as ET

import PIL
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / "assets/wallpaper"
HISTORICAL_DEST = ROOT / "maquettes/accueil-historique/legacy"
HISTORICAL_SCREEN = HISTORICAL_DEST.parent / "screen.json"


def output_path(name: str) -> Path:
    return (HISTORICAL_DEST if name.startswith("accueil_historique_") else DEST) / name
MANIFEST = ROOT / "assets/manifest.json"
OLD_COMMIT = "50d9e1dabd71725adba50279929694123fde0fda"
OLD_DIR = "design/graph-acc-001/"
PO_MASTER = ROOT / "draft/Wallpaper_accueil.jpeg"
PO_MASTER_SHA = "3f686ba83e3f91e6349ebec89069d74a9595ec5d7a4c177518c0bb0f74712642"
APPROVAL = "https://github.com/Kharn27/ESP32-OTA-Battleship/issues/105"


def sha256(buf: bytes) -> str:
    return hashlib.sha256(buf).hexdigest()


def historical(path: str) -> bytes:
    return subprocess.run(
        ["git", "show", OLD_COMMIT + ":" + OLD_DIR + path],
        cwd=ROOT, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout


def dimensions(buf: bytes) -> tuple[int, int]:
    with Image.open(io.BytesIO(buf)) as im:
        im.load()
        return im.size


def original_from_svg() -> bytes:
    document = ET.fromstring(historical("accueil_cyd_warship.svg"))
    images = [e for e in document.iter() if e.tag.rsplit("}", 1)[-1] == "image"]
    if len(images) != 1:
        raise RuntimeError("Historical SVG must contain exactly one embedded raster")
    href = images[0].get("{http://www.w3.org/1999/xlink}href") or images[0].get("href")
    if not href or not href.startswith("data:image/") or ";base64," not in href:
        raise RuntimeError("No embedded image data URI in historical SVG")
    buf = base64.b64decode(re.sub(r"\s+", "", href.split(";base64,", 1)[1]), validate=True)
    with Image.open(io.BytesIO(buf)) as im:
        im.load()
        if im.size != (1448, 1086):
            raise RuntimeError("Unexpected historical raster dimensions: " + str(im.size))
        if im.format == "PNG":
            return buf
        output = io.BytesIO()
        im.convert("RGB").save(output, format="PNG", optimize=True)
        return output.getvalue()


def construct() -> tuple[dict[str, bytes], list[dict]]:
    if PIL.__version__ != "12.3.0":
        raise RuntimeError("Require Pillow==12.3.0")
    master = PO_MASTER.read_bytes()
    if sha256(master) != PO_MASTER_SHA:
        raise RuntimeError("Original PO wallpaper has changed")
    if dimensions(master) != (1195, 896):
        raise RuntimeError("Unexpected PO wallpaper dimensions")
    with Image.open(io.BytesIO(master)) as im:
        resized = im.convert("RGB").resize((320, 240), Image.Resampling.LANCZOS)
    output = io.BytesIO()
    resized.save(output, format="PNG", optimize=True)
    historical_hi = original_from_svg()
    historical_cyd = historical("accueil_cyd_warship_320x240.png")
    files = {
        "accueil_historique_original_1448x1086.png": historical_hi,
        "accueil_historique_CYD_320x240.png": historical_cyd,
        "accueil_retenu_original_1195x896.jpeg": master,
        "accueil_retenu_CYD_320x240.png": output.getvalue(),
    }
    meta = [
        ("wallpaper.accueil.historique.original", "accueil_historique_original_1448x1086.png",
         [1448, 1086], f"https://github.com/Kharn27/ESP32-OTA-Battleship/blob/{OLD_COMMIT}/{OLD_DIR}accueil_cyd_warship.svg",
         "HISTORICAL_FLATTENED_ACCUEIL", "https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8"),
        ("wallpaper.accueil.historique.cyd", "accueil_historique_CYD_320x240.png",
         [320, 240], f"https://github.com/Kharn27/ESP32-OTA-Battleship/blob/{OLD_COMMIT}/{OLD_DIR}accueil_cyd_warship_320x240.png",
         "HISTORICAL_FLATTENED_ACCUEIL", "https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8"),
        ("wallpaper.accueil.retenu.original", "accueil_retenu_original_1195x896.jpeg",
         [1195, 896], "draft/Wallpaper_accueil.jpeg",
         "CURRENT_WALLPAPER_BACKGROUND", "https://github.com/Kharn27/ESP32-OTA-Battleship/pull/90#issuecomment-6740684237"),
        ("wallpaper.accueil.retenu.cyd", "accueil_retenu_CYD_320x240.png",
         [320, 240], "assets/wallpaper/accueil_retenu_original_1195x896.jpeg",
         "CURRENT_WALLPAPER_BACKGROUND", "https://github.com/Kharn27/ESP32-OTA-Battleship/pull/90#issuecomment-6740684237"),
    ]
    catalog = []
    for identifier, name, dims, source, role, art in meta:
        if dimensions(files[name]) != tuple(dims):
            raise RuntimeError("Wrong output dimensions: " + name)
        if role == "HISTORICAL_FLATTENED_ACCUEIL":
            # The old complete UI is an archived screen, NEVER a firmware asset.
            continue
        catalog.append({
            "id": identifier, "path": "assets/wallpaper/" + name,
            "sha256": sha256(files[name]), "source_ref": source,
            "po_validation_ref": APPROVAL, "artistic_source_ref": art,
            "catalog_role": role, "dimensions": dims,
            "device_status": "ON_DEVICE_UNVERIFIED",
        })
    return files, catalog


def verify(files: dict[str, bytes], entries: list[dict]) -> None:
    for name, data in files.items():
        path = output_path(name)
        if not path.is_file() or path.read_bytes() != data:
            raise RuntimeError("Export missing or changed: " + str(path))
    m = json.loads(MANIFEST.read_text(encoding="utf-8"))
    by_id = {e["id"]: e for e in m["assets"]}
    for entry in entries:
        if by_id.get(entry["id"]) != entry:
            raise RuntimeError("Manifest mismatch: " + entry["id"])
    screen = json.loads(HISTORICAL_SCREEN.read_text(encoding="utf-8"))
    for name, data in files.items():
        if name.startswith("accueil_historique_"):
            canonical_blob = hashlib.sha1(f"blob {len(data)}\0".encode("ascii") + data).hexdigest()
            if screen.get("legacy_files", {}).get(name) != canonical_blob:
                raise RuntimeError("Historical screen inventory mismatch: " + name)
    print("ACCUEIL_EXPORT_OK: two runtime wallpaper images and two archived screen images byte-reproducible")
    for e in entries:
        print(e["path"], e["dimensions"], e["sha256"])


def write(files: dict[str, bytes], entries: list[dict]) -> None:
    DEST.mkdir(parents=True, exist_ok=True)
    HISTORICAL_DEST.mkdir(parents=True, exist_ok=True)
    for name, data in files.items():
        path = output_path(name)
        if path.exists() and path.read_bytes() != data:
            raise RuntimeError("Refusing to overwrite approved pixels: " + name)
        path.write_bytes(data)
    m = json.loads(MANIFEST.read_text(encoding="utf-8"))
    old = {e["id"]: e for e in m["assets"]}
    for entry in entries:
        if entry["id"] in old and old[entry["id"]] != entry:
            raise RuntimeError("Refusing to alter approved manifest: " + entry["id"])
        if entry["id"] not in old:
            m["assets"].append(entry)
    MANIFEST.write_text(json.dumps(m, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    verify(files, entries)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument("--write", action="store_true")
    action.add_argument("--check", action="store_true")
    args = parser.parse_args()
    files, entries = construct()
    (write if args.write else verify)(files, entries)
