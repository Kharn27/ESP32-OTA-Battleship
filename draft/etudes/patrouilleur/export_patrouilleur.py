#!/usr/bin/env python3
"""PAT-01 — export reproductible de la source PO, sans retouche artistique."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import PIL
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[3]
SOURCE = ROOT / "draft/patrouilleur_A.jpeg"
TARGET = ROOT / "design/patrouilleur"
NATIVE = TARGET / "patrouilleur_intact_320x240.png"
ZOOM = TARGET / "patrouilleur_intact_x4.png"
MANIFEST = TARGET / "preuves/export_manifest.json"
SOURCE_SHA256 = "631ff1b8f4245599137b6d0d3bd6919b9db3e30844d478434ee864ff885f5df5"
SOURCE_SIZE = (1195, 896)
NATIVE_SIZE = (320, 240)
ZOOM_SIZE = (1280, 960)

# Repères COMPOSITIONS (lecture visuelle approximative de la source), pas masque alpha.
# Bbox [gauche, haut, droite, bas] à coordonnées limites, hors sillage/écume.
SHIP_BBOX_SOURCE_APPROX = (397, 343, 827, 535)
SHIP_PIVOT_SOURCE = (612, 439)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def to_native(point: tuple[int, int]) -> list[int]:
    return [round(point[0] * NATIVE_SIZE[0] / SOURCE_SIZE[0]),
            round(point[1] * NATIVE_SIZE[1] / SOURCE_SIZE[1])]


def image_equal(a: Image.Image, b: Image.Image) -> bool:
    return a.mode == b.mode and a.size == b.size and ImageChops.difference(a, b).getbbox() is None


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Verifier les exports sans les modifier")
    args = parser.parse_args()

    actual_source_sha = sha256(SOURCE)
    assert actual_source_sha == SOURCE_SHA256, (
        f"Source modifiee : {actual_source_sha}, attendu {SOURCE_SHA256}")
    with Image.open(SOURCE) as source:
        assert source.format == "JPEG" and source.size == SOURCE_SIZE
        original = source.convert("RGB")

    # AUCUN recadrage : image PO complete. Traitement strictement dimensionnel.
    native = original.resize(NATIVE_SIZE, Image.Resampling.LANCZOS)
    zoom = native.resize(ZOOM_SIZE, Image.Resampling.NEAREST)

    if not args.check:
        TARGET.mkdir(parents=True, exist_ok=True)
        MANIFEST.parent.mkdir(parents=True, exist_ok=True)
        native.save(NATIVE, format="PNG", compress_level=9)
        zoom.save(ZOOM, format="PNG", compress_level=9)

    assert NATIVE.is_file() and ZOOM.is_file(), "Les deux PNG doivent exister"
    with Image.open(NATIVE) as im:
        disk_native = im.convert("RGB")
        assert im.format == "PNG" and im.mode == "RGB"
    with Image.open(ZOOM) as im:
        disk_zoom = im.convert("RGB")
        assert im.format == "PNG" and im.mode == "RGB"

    assert image_equal(disk_native, native), "Echec du controle source -> 320x240"
    assert image_equal(disk_zoom, zoom), "Echec du controle 320x240 -> x4"
    # Epreuve pixel par pixel sur le fichier relu : aucune interpolation en x4.
    assert image_equal(disk_zoom, disk_native.resize(ZOOM_SIZE, Image.Resampling.NEAREST))
    assert disk_native.size == NATIVE_SIZE and disk_zoom.size == ZOOM_SIZE

    x0, y0, x1, y1 = SHIP_BBOX_SOURCE_APPROX
    nx0, ny0 = to_native((x0, y0))
    nx1, ny1 = to_native((x1, y1))
    manifest = {
        "mission": "PAT-01 / Issue #79",
        "provenance": {
            "path": "draft/patrouilleur_A.jpeg",
            "source_dimensions": list(SOURCE_SIZE),
            "source_sha256": actual_source_sha,
            "source_is_flat_scene_with_painted_ocean": True,
        },
        "export": {
            "library": "Pillow",
            "library_version": PIL.__version__,
            "full_frame_no_crop": True,
            "native_filter": "LANCZOS",
            "zoom_filter": "NEAREST",
            "alpha": False,
            "native_size": list(NATIVE_SIZE),
            "zoom_size": list(ZOOM_SIZE),
            "scale_xy": [NATIVE_SIZE[0] / SOURCE_SIZE[0],
                         NATIVE_SIZE[1] / SOURCE_SIZE[1]],
        },
        "geometry_reference": {
            "type": "visually_estimated_composition_anchor_not_alpha_mask",
            "ship_bbox_source_approx_ltrb": list(SHIP_BBOX_SOURCE_APPROX),
            "ship_bbox_native_approx_ltrb": [nx0, ny0, nx1, ny1],
            "pivot_source": list(SHIP_PIVOT_SOURCE),
            "pivot_native": to_native(SHIP_PIVOT_SOURCE),
            "frame_origin_source": [0, 0],
            "frame_origin_native": [0, 0],
            "note": "Bbox du navire avec drapeau, hors ecume; estimation manuelle a valider sur tous les etats.",
        },
        "files": {
            "patrouilleur_intact_320x240.png": {
                "bytes": NATIVE.stat().st_size, "sha256": sha256(NATIVE)},
            "patrouilleur_intact_x4.png": {
                "bytes": ZOOM.stat().st_size, "sha256": sha256(ZOOM)},
        },
        "checks": {
            "source_sha256": "PASS",
            "dimensions": "PASS",
            "native_matches_full_source_LANCZOS": "PASS",
            "zoom_pixel_exact_NEAREST": "PASS",
        },
        "status": "DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED",
    }

    payload = json.dumps(manifest, indent=2, ensure_ascii=False, sort_keys=True) + "\n"
    if args.check:
        assert MANIFEST.read_text(encoding="utf-8") == payload, "Manifeste incoherent"
    else:
        MANIFEST.write_text(payload, encoding="utf-8")
    print("PASS PAT-01: source PO exacte, native 320x240, x4 1280x960 NEAREST pixel-exact")
    print("Source:", actual_source_sha)
    for name, info in manifest["files"].items():
        print(name, info["bytes"], "octets", info["sha256"])
    print("Ancrage natif:", manifest["geometry_reference"]["pivot_native"],
          "; bbox approx:", manifest["geometry_reference"]["ship_bbox_native_approx_ltrb"])


if __name__ == "__main__":
    main()
