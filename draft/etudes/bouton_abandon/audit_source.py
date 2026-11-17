#!/usr/bin/env python3
"""Audit non destructif du PNG original PO ABANDON (Issue #149).

Usage : python3 draft/etudes/bouton_abandon/audit_source.py --check
Aucune image n'est réécrite. Pillow est nécessaire pour inspecter l'alpha.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

EXPECTED_BLOB_SHA1 = "3818cb092456e1ff222d8b2dacf9f9aa55266135"
EXPECTED_BYTES = 1814386
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def git_blob_sha1(content: bytes) -> str:
    header = b"blob " + str(len(content)).encode("ascii") + b"\0"
    return hashlib.sha1(header + content).hexdigest()


def audit(path: Path) -> dict:
    payload = path.read_bytes()
    if not payload.startswith(PNG_SIGNATURE):
        raise ValueError(f"{path}: signature PNG invalide")
    width, height, depth, color_type, compression, filtering, interlace = struct.unpack(
        ">IIBBBBB", payload[16:29]
    )
    result = {
        "path": path.as_posix(),
        "bytes": len(payload),
        "sha256": hashlib.sha256(payload).hexdigest(),
        "git_blob_sha1": git_blob_sha1(payload),
        "png_ihdr": {
            "width": width, "height": height, "bit_depth": depth,
            "color_type": color_type, "compression": compression,
            "filter": filtering, "interlace": interlace,
        },
    }
    from PIL import Image
    with Image.open(path) as src:
        src.load()
        result["mode"] = src.mode
        result["dimensions"] = list(src.size)
        rgba = src.convert("RGBA")
        alpha = rgba.getchannel("A")
        hist = alpha.histogram()
        result["alpha"] = {
            "fully_transparent_pixels": hist[0],
            "partially_transparent_pixels": sum(hist[1:255]),
            "opaque_pixels": hist[255],
            "content_bbox": list(alpha.getbbox()) if alpha.getbbox() else None,
        }
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="vérifier blob, taille, dimensions et alpha en lecture seule")
    args = parser.parse_args()
    original = Path(__file__).resolve().parents[2] / "Bouton_Abandon.png"
    report = audit(original)
    print(json.dumps(report, ensure_ascii=False, indent=2))
    errors = []
    if report["bytes"] != EXPECTED_BYTES:
        errors.append("taille originale différente du document d'inventaire")
    if report["git_blob_sha1"] != EXPECTED_BLOB_SHA1:
        errors.append("sha1 Git différent du blob PO sauvegardé")
    if not report["dimensions"] or report["dimensions"][0] < 1 or report["dimensions"][1] < 1:
        errors.append("dimensions invalides")
    for error in errors:
        print(f"ERROR: {error}")
    if errors:
        return 1
    print("SOURCE_PO_BLOB_OK; AUDIT_RGBA_DONE")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
