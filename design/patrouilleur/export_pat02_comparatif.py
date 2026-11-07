#!/usr/bin/env python3
"""PAT-02, phase 1 — aperçus comparatifs PO T1A/T1B, sans sélection ni retouche."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import PIL
from PIL import Image, ImageChops, ImageStat

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "design/patrouilleur"
OUT = BASE / "revue_pat02"
MANIFEST = OUT / "preuves/export_manifest.json"
INTACT_NATIVE = BASE / "patrouilleur_intact_320x240.png"
INTACT_NATIVE_SHA = "84b38a704d9ded44f9cc880a1af6ebf092a87a2803c8c972e24f73df9cf808fb"
SOURCES = {
    "T1A": (
        ROOT / "draft/patrouilleur_T1A.jpeg",
        "1e25359998afdedbc93ebf6baebdbeae34b327f4ab75a2f4e9d55e4c7b8a9584",
    ),
    "T1B": (
        ROOT / "draft/patrouilleur_T1B.jpeg",
        "2bc761b1abcbced2fc760e8ef938571d58b3b4b14e390bad9f219846b32c8666",
    ),
}
ORIGINAL_SIZE = (1195, 896)
NATIVE_SIZE = (320, 240)
ZOOM_SIZE = (1280, 960)
ANCHOR = (164, 118)  # pivot visuel de PAT-01, PAS pivot runtime certifié.
SHIP_BBOX_INTACT_APPROX = (106, 92, 221, 143)
# La proue (à droite) reste théoriquement intacte : diagnostic visuel de décalage,
# pas une segmentation du navire ni une mesure officielle d'ancre.
FORE_ROI = (700, 410, 815, 465)
MAX_SHIFT = 8


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def identical(a: Image.Image, b: Image.Image) -> bool:
    return a.size == b.size and a.mode == b.mode and ImageChops.difference(a, b).getbbox() is None


def score_roi(reference: Image.Image, candidate: Image.Image, dx: int, dy: int) -> float:
    x0, y0, x1, y1 = FORE_ROI
    assert reference.size == candidate.size == ORIGINAL_SIZE
    a = reference.crop(FORE_ROI)
    b = candidate.crop((x0 + dx, y0 + dy, x1 + dx, y1 + dy))
    return sum(ImageStat.Stat(ImageChops.difference(a, b)).mean) / 3.0


def registration_check(reference: Image.Image, candidate: Image.Image) -> dict:
    unchanged_frame = score_roi(reference, candidate, 0, 0)
    # Ce test compare seulement une zone avant au rendu source ; les différences
    # JPEG / rendu artistique sont possibles et ne doivent pas être corrigées.
    best = (unchanged_frame, 0, 0)
    for dy in range(-MAX_SHIFT, MAX_SHIFT + 1):
        for dx in range(-MAX_SHIFT, MAX_SHIFT + 1):
            sc = score_roi(reference, candidate, dx, dy)
            if sc < best[0]:
                best = (sc, dx, dy)
    return {
        "roi_source_ltrb": list(FORE_ROI),
        "search_source_pixel_shift": MAX_SHIFT,
        "unshifted_mean_absolute_RGB_difference": round(unchanged_frame, 4),
        "best_diagnostic_offset_xy": [best[1], best[2]],
        "best_mean_absolute_RGB_difference": round(best[0], 4),
        "interpretation": "Diagnostic numerique local (proue) uniquement, pas une preuve de recalage global, aucune correction appliquee.",
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Verifier sans modifier les PNG ni le manifeste")
    args = parser.parse_args()

    assert sha256(INTACT_NATIVE) == INTACT_NATIVE_SHA, "Reference PAT-01 a ete modifiee"
    with Image.open(ROOT / "draft/patrouilleur_A.jpeg") as img:
        assert img.size == ORIGINAL_SIZE
        reference = img.convert("RGB")

    files = {}
    source_manifest = {}
    alignment = {}
    for key, (source_path, expected_sha) in SOURCES.items():
        actual_sha = sha256(source_path)
        assert actual_sha == expected_sha, f"Source PO changee {key} : {actual_sha}"
        with Image.open(source_path) as img:
            assert img.format == "JPEG" and img.size == ORIGINAL_SIZE
            original = img.convert("RGB")
        native = original.resize(NATIVE_SIZE, Image.Resampling.LANCZOS)
        zoom = native.resize(ZOOM_SIZE, Image.Resampling.NEAREST)
        native_path = OUT / f"patrouilleur_{key}_320x240.png"
        zoom_path = OUT / f"patrouilleur_{key}_x4.png"
        if not args.check:
            OUT.mkdir(parents=True, exist_ok=True)
            MANIFEST.parent.mkdir(parents=True, exist_ok=True)
            native.save(native_path, format="PNG", compress_level=9)
            zoom.save(zoom_path, format="PNG", compress_level=9)

        assert native_path.is_file() and zoom_path.is_file(), f"Manque export {key}"
        with Image.open(native_path) as im:
            assert im.format == "PNG" and im.mode == "RGB" and im.size == NATIVE_SIZE
            native_reloaded = im.copy()
        with Image.open(zoom_path) as im:
            assert im.format == "PNG" and im.mode == "RGB" and im.size == ZOOM_SIZE
            zoom_reloaded = im.copy()
        assert identical(native, native_reloaded), f"Export natif non fidele {key}"
        assert identical(zoom, zoom_reloaded), f"Export x4 non fidele {key}"
        assert identical(zoom_reloaded, native_reloaded.resize(ZOOM_SIZE, Image.Resampling.NEAREST)), f"Zoom non NEAREST {key}"

        source_manifest[key] = {
            "path": str(source_path.relative_to(ROOT)),
            "sha256": actual_sha,
            "dimensions": list(ORIGINAL_SIZE),
            "contains_painted_ocean": True,
            "transparent_sprite": False,
        }
        alignment[key] = registration_check(reference, original)
        for path, dimensions in ((native_path, NATIVE_SIZE), (zoom_path, ZOOM_SIZE)):
            files[path.name] = {
                "path": str(path.relative_to(ROOT)),
                "sha256": sha256(path),
                "bytes": path.stat().st_size,
                "dimensions": list(dimensions),
                "mode": "RGB",
            }

    manifest = {
        "issue": 80,
        "phase": "comparaison_avant_choix_PO",
        "state": "AUCUNE_VARIANTE_SELECTIONNEE",
        "status": "DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED",
        "sources": source_manifest,
        "reference_PAT01": {
            "native_path": str(INTACT_NATIVE.relative_to(ROOT)),
            "native_sha256": INTACT_NATIVE_SHA,
            "native_pivot": list(ANCHOR),
            "ship_bbox_native_visual_approx": list(SHIP_BBOX_INTACT_APPROX),
        },
        "export": {
            "tool": "Pillow",
            "version": PIL.__version__,
            "full_frame_no_crop_no_artistic_edit": True,
            "source_dimensions": list(ORIGINAL_SIZE),
            "native_dimensions": list(NATIVE_SIZE),
            "native_filter": "LANCZOS",
            "zoom_dimensions": list(ZOOM_SIZE),
            "zoom_filter": "NEAREST",
            "RGB_no_alpha": True,
            "sx": NATIVE_SIZE[0] / ORIGINAL_SIZE[0],
            "sy": NATIVE_SIZE[1] / ORIGINAL_SIZE[1],
        },
        "alignment_diagnostic": alignment,
        "files": files,
        "checks": {
            "canonical_PO_source_SHA256": "PASS",
            "PAT01_reference_unchanged": "PASS",
            "both_full_frame_dimensions": "PASS",
            "both_native_pixels_match_LANCZOS": "PASS",
            "both_zoom_pixels_exact_NEAREST_after_reopen": "PASS",
        },
    }
    payload = json.dumps(manifest, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.check:
        assert MANIFEST.read_text(encoding="utf-8") == payload, "Manifest n'est pas reproductible"
    else:
        MANIFEST.write_text(payload, encoding="utf-8")
    print("PASS PAT-02 PHASE 1: 2 originaux PO, 320x240 RGB plein cadre, 1280x960 NEAREST exact")
    print("PAS DE SELECTION PO — comparaison seulement.")
    for key in SOURCES:
        print("  ", key, alignment[key])
        for suffix in ("320x240", "x4"):
            name = f"patrouilleur_{key}_{suffix}.png"
            print("  ", name, files[name]["bytes"], "octets", files[name]["sha256"])


if __name__ == "__main__":
    main()
