#!/usr/bin/env python3
"""PAT-03 phase 2 — finaliser le patrouilleur avant endommage T2A du PO."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import PIL
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[3]
BASE = ROOT / "draft/etudes/patrouilleur"
SOURCE = ROOT / "draft/patrouilleur_T2A.jpeg"
OTHER_SOURCE = ROOT / "draft/patrouilleur_T2B.jpeg"
INTACT = BASE / "patrouilleur_intact_320x240.png"
REAR = BASE / "patrouilleur_arriere_endommage_320x240.png"
NATIVE = BASE / "patrouilleur_avant_endommage_320x240.png"
ZOOM = BASE / "patrouilleur_avant_endommage_x4.png"
MANIFEST = BASE / "preuves/pat03_export_manifest.json"

SOURCE_SHA = "f8366482082febc917a812d94efd98a75d03c100a789d5db73ceacc379c41d4d"
OTHER_SOURCE_SHA = "bcb36de1ef21c7992af3196d460ce57f68d654d4826302be0fdfa20b5515a9c3"
INTACT_SHA = "84b38a704d9ded44f9cc880a1af6ebf092a87a2803c8c972e24f73df9cf808fb"
REAR_SHA = "a136b18ab4d32e522bb5fd314b1faa87dde275e752d2b4bd99a267b4fdf93588"
NATIVE_SHA = "c079ba0e2e79ebbbcbb31e4a046d7c337dd1e1164aff1a678955d54633ceb36c"
ZOOM_SHA = "e9dbcb956a35d4bef6f655592ef560d14f62d7fd93c3ef52b207c09a20e8a943"
SOURCE_SIZE = (1195, 896)
NATIVE_SIZE = (320, 240)
ZOOM_SIZE = (1280, 960)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def identical(a: Image.Image, b: Image.Image) -> bool:
    return (
        a.mode == b.mode and a.size == b.size
        and ImageChops.difference(a, b).getbbox() is None
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="Verifier les PNG et le manifeste sans aucune ecriture")
    args = parser.parse_args()

    assert PIL.__version__ == "12.3.0", "Installer Pillow==12.3.0"
    for path, expected_sha in (
        (SOURCE, SOURCE_SHA),
        (OTHER_SOURCE, OTHER_SOURCE_SHA),
        (INTACT, INTACT_SHA),
        (REAR, REAR_SHA),
    ):
        assert sha256(path) == expected_sha, f"Original / reference modifie : {path}"

    with Image.open(SOURCE) as image:
        assert image.format == "JPEG" and image.size == SOURCE_SIZE
        original = image.convert("RGB")
    with Image.open(OTHER_SOURCE) as image:
        assert image.format == "JPEG" and image.size == SOURCE_SIZE
    for path in (INTACT, REAR):
        with Image.open(path) as image:
            assert image.format == "PNG" and image.mode == "RGB"
            assert image.size == NATIVE_SIZE

    # Cadrage integral et repere PAT-01 ; aucun crop, translation ou retouche.
    native = original.resize(NATIVE_SIZE, Image.Resampling.LANCZOS)
    zoom = native.resize(ZOOM_SIZE, Image.Resampling.NEAREST)
    if not args.check:
        MANIFEST.parent.mkdir(parents=True, exist_ok=True)
        native.save(NATIVE, format="PNG", compress_level=9)
        zoom.save(ZOOM, format="PNG", compress_level=9)

    files = {}
    for path, expected_image, dimensions, expected_sha in (
        (NATIVE, native, NATIVE_SIZE, NATIVE_SHA),
        (ZOOM, zoom, ZOOM_SIZE, ZOOM_SHA),
    ):
        assert path.is_file(), f"Manque fichier {path}"
        with Image.open(path) as image:
            assert image.format == "PNG" and image.mode == "RGB"
            assert image.size == dimensions
            disk = image.copy()
        assert identical(disk, expected_image), f"Erreur pixels {path.name}"
        actual_sha = sha256(path)
        assert actual_sha == expected_sha, f"SHA-256 non conforme {path.name}"
        files[path.name] = {
            "path": str(path.relative_to(ROOT)),
            "bytes": path.stat().st_size,
            "dimensions": list(dimensions),
            "mode": "RGB",
            "sha256": actual_sha,
        }

    with Image.open(NATIVE) as image:
        reloaded_native = image.copy()
    with Image.open(ZOOM) as image:
        reloaded_zoom = image.copy()
    assert identical(
        reloaded_zoom, reloaded_native.resize(ZOOM_SIZE, Image.Resampling.NEAREST)
    ), "Zoom final x4 non NEAREST pixel exact"

    manifest = {
        "issue": 81,
        "phase": "etat_avant_final_apres_choix_PO",
        "selection_PO": {
            "variant": "T2A",
            "cadrage": "plein cadre original, sans translation",
            "reference": "Issue #81 commentaire Pilotage #6708055689 (07/11/2026)",
        },
        "source": {
            "path": str(SOURCE.relative_to(ROOT)),
            "sha256": SOURCE_SHA,
            "dimensions": list(SOURCE_SIZE),
            "format": "JPEG",
            "ocean_peint": True,
            "alpha": False,
        },
        "source_alternative_conservee_seulement_dans_draft": {
            "path": str(OTHER_SOURCE.relative_to(ROOT)),
            "sha256": OTHER_SOURCE_SHA,
            "exported": False,
        },
        "references": {
            "PAT01": {
                "path": str(INTACT.relative_to(ROOT)), "sha256": INTACT_SHA,
            },
            "PAT02_T1A": {
                "path": str(REAR.relative_to(ROOT)), "sha256": REAR_SHA,
            },
        },
        "export": {
            "tool": "Pillow",
            "version": PIL.__version__,
            "scene_originale_complete_sans_retouche_ni_translation": True,
            "native_dimensions": list(NATIVE_SIZE),
            "native_filter": "LANCZOS",
            "zoom_dimensions": list(ZOOM_SIZE),
            "zoom_filter": "NEAREST",
            "mode": "RGB",
            "alpha": False,
            "sx": NATIVE_SIZE[0] / SOURCE_SIZE[0],
            "sy": NATIVE_SIZE[1] / SOURCE_SIZE[1],
        },
        "geometry_reference": {
            "pivot_natif_indicatif": [164, 118],
            "bbox_coque_intacte_natif_approx": [106, 92, 221, 143],
            "translation_appliquee": [0, 0],
            "preuve_alignement_pixel_global": False,
        },
        "files": files,
        "checks": {
            "canonical_PO_sources_SHA256": "PASS",
            "PAT01_PAT02_references_SHA256": "PASS",
            "image_source_size_and_format": "PASS",
            "native_LANCZOS_RGB_320x240_exact": "PASS",
            "zoom_NEAREST_RGB_1280x960_pixel_exact_apres_relecture": "PASS",
            "output_SHA256_fixed_expected": "PASS",
            "manifest_reproducible": "PASS",
        },
        "status": "DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED",
        "runtime_asset": False,
    }
    payload = json.dumps(manifest, indent=2, ensure_ascii=False, sort_keys=True) + "\n"
    if args.check:
        assert MANIFEST.is_file()
        assert MANIFEST.read_text(encoding="utf-8") == payload, "Manifeste incoherent"
    else:
        MANIFEST.write_text(payload, encoding="utf-8")

    print("PASS PAT-03 FINAL T2A — sources et references SHA256")
    print("PASS 320x240 RGB LANCZOS ; zoom 1280x960 NEAREST strict")
    for path, info in files.items():
        print(path, info["bytes"], "octets", info["sha256"])
    print("DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED")


if __name__ == "__main__":
    main()
