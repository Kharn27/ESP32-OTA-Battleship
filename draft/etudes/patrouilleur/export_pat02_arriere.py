#!/usr/bin/env python3
"""PAT-02 — export final T1A depuis l'original PO. Aucune retouche artistique."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import PIL
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[3]
BASE = ROOT / "draft/etudes/patrouilleur"
SOURCE = ROOT / "draft/patrouilleur_T1A.jpeg"
INTACT = BASE / "patrouilleur_intact_320x240.png"
NATIVE = BASE / "patrouilleur_arriere_endommage_320x240.png"
ZOOM = BASE / "patrouilleur_arriere_endommage_x4.png"
MANIFEST = BASE / "preuves/pat02_export_manifest.json"

SOURCE_SHA = "1e25359998afdedbc93ebf6baebdbeae34b327f4ab75a2f4e9d55e4c7b8a9584"
INTACT_SHA = "84b38a704d9ded44f9cc880a1af6ebf092a87a2803c8c972e24f73df9cf808fb"
NATIVE_SHA = "a136b18ab4d32e522bb5fd314b1faa87dde275e752d2b4bd99a267b4fdf93588"
ZOOM_SHA = "88f9abdc1539e9bbdd1bcd1092bf77a07c272c2aa525e699327123fffa7457ca"
SOURCE_SIZE = (1195, 896)
NATIVE_SIZE = (320, 240)
ZOOM_SIZE = (1280, 960)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def identical(a: Image.Image, b: Image.Image) -> bool:
    return (
        a.size == b.size
        and a.mode == b.mode
        and ImageChops.difference(a, b).getbbox() is None
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="Verifie sans modifier aucun livrable")
    args = parser.parse_args()
    assert PIL.__version__ == "12.3.0", "Pillow 12.3.0 requis pour l'export reproductible"
    assert sha256(SOURCE) == SOURCE_SHA, "La source originale T1A a change"
    assert sha256(INTACT) == INTACT_SHA, "Le PNG intact PAT-01 a change"

    with Image.open(SOURCE) as image:
        assert image.format == "JPEG" and image.size == SOURCE_SIZE
        original = image.convert("RGB")

    # Même cadrage plein cadre que PAT-01, AUCUNE correction du dessin PO.
    native = original.resize(NATIVE_SIZE, Image.Resampling.LANCZOS)
    zoom = native.resize(ZOOM_SIZE, Image.Resampling.NEAREST)

    if not args.check:
        BASE.mkdir(parents=True, exist_ok=True)
        MANIFEST.parent.mkdir(parents=True, exist_ok=True)
        native.save(NATIVE, format="PNG", compress_level=9)
        zoom.save(ZOOM, format="PNG", compress_level=9)

    files = {}
    for path, calculated, size, expected_sha in (
        (NATIVE, native, NATIVE_SIZE, NATIVE_SHA),
        (ZOOM, zoom, ZOOM_SIZE, ZOOM_SHA),
    ):
        assert path.is_file(), f"Export absent : {path}"
        with Image.open(path) as image:
            assert image.format == "PNG" and image.mode == "RGB"
            assert image.size == size
            reloaded = image.copy()
        assert identical(calculated, reloaded), f"Image non conforme : {path.name}"
        assert sha256(path) == expected_sha, f"SHA-256 inattendu : {path.name}"
        files[path.name] = {
            "bytes": path.stat().st_size,
            "dimensions": list(size),
            "mode": "RGB",
            "path": str(path.relative_to(ROOT)),
            "sha256": expected_sha,
        }

    with Image.open(NATIVE) as image:
        native_disk = image.copy()
    with Image.open(ZOOM) as image:
        zoom_disk = image.copy()
    assert identical(
        zoom_disk, native_disk.resize(ZOOM_SIZE, Image.Resampling.NEAREST)
    ), "Echec pixel par pixel : zoom non NEAREST"

    manifest = {
        "issue": 80,
        "phase": "etat_arriere_final_apres_selection_PO",
        "selection_PO": {
            "variant": "T1A",
            "rationale": "Meilleure lisibilite pressentie a petite echelle, y compris deux cases de grille ; non testee sur CYD.",
            "artistic_change": False,
            "reference": "decision explicite du PO dans la conversation PAT-02 du 07/11/2026",
        },
        "source": {
            "path": str(SOURCE.relative_to(ROOT)),
            "sha256": SOURCE_SHA,
            "dimensions": list(SOURCE_SIZE),
            "contains_painted_ocean": True,
            "transparent_sprite": False,
        },
        "reference_PAT01": {
            "native_path": str(INTACT.relative_to(ROOT)),
            "native_sha256": INTACT_SHA,
            "native_pivot": [164, 118],
            "ship_bbox_native_visual_approx": [106, 92, 221, 143],
        },
        "export": {
            "tool": "Pillow",
            "version": PIL.__version__,
            "full_frame_no_crop_no_artistic_edit": True,
            "source_dimensions": list(SOURCE_SIZE),
            "native_dimensions": list(NATIVE_SIZE),
            "native_filter": "LANCZOS",
            "zoom_dimensions": list(ZOOM_SIZE),
            "zoom_filter": "NEAREST",
            "RGB_no_alpha": True,
            "sx": NATIVE_SIZE[0] / SOURCE_SIZE[0],
            "sy": NATIVE_SIZE[1] / SOURCE_SIZE[1],
        },
        "alignment_diagnostic": {
            "best_diagnostic_offset_xy": [3, 0],
            "roi_source_ltrb": [700, 410, 815, 465],
            "unshifted_mean_absolute_RGB_difference": 24.2708,
            "best_mean_absolute_RGB_difference": 14.4265,
            "interpretation": "Mesure locale comparative de la phase 1 uniquement. Aucun recalage applique. Pas preuve de translation globale.",
        },
        "files": files,
        "checks": {
            "canonical_PO_source_SHA256": "PASS",
            "PAT01_reference_unchanged": "PASS",
            "full_frame_dimensions": "PASS",
            "native_pixels_match_LANCZOS": "PASS",
            "zoom_pixels_exact_NEAREST_after_reopen": "PASS",
            "final_export_SHA256": "PASS",
        },
        "status": "DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED",
    }

    payload = json.dumps(manifest, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.check:
        assert MANIFEST.read_text(encoding="utf-8") == payload, "Manifest final incoherent"
    else:
        MANIFEST.write_text(payload, encoding="utf-8")
    print("PASS PAT-02 FINAL T1A — PO selectionne, source originale intacte")
    print("PASS natif 320x240 RGB, zoom x4 1280x960 NEAREST pixel-exact")
    for name, info in sorted(files.items()):
        print(name, info["bytes"], "octets", info["sha256"])
    print("DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED")


if __name__ == "__main__":
    main()
