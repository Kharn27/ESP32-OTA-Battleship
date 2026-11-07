#!/usr/bin/env python3
"""PAT-03 phase 1 : T2A/T2B, exports fideles aux originaux PO (sans retouche)."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import PIL
from PIL import Image, ImageChops, ImageStat

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "design/patrouilleur"
TARGET = BASE / "revue_pat03"
MANIFEST = TARGET / "preuves/export_manifest.json"
SOURCE_SIZE = (1195, 896)
NATIVE_SIZE = (320, 240)
ZOOM_SIZE = (1280, 960)
SOURCES = {
    "T2A": {
        "path": ROOT / "draft/patrouilleur_T2A.jpeg",
        "sha256": "f8366482082febc917a812d94efd98a75d03c100a789d5db73ceacc379c41d4d",
    },
    "T2B": {
        "path": ROOT / "draft/patrouilleur_T2B.jpeg",
        "sha256": "bcb36de1ef21c7992af3196d460ce57f68d654d4826302be0fdfa20b5515a9c3",
    },
}
REFERENCES = {
    "PAT01_intact": {
        "path": BASE / "patrouilleur_intact_320x240.png",
        "sha256": "84b38a704d9ded44f9cc880a1af6ebf092a87a2803c8c972e24f73df9cf808fb",
    },
    "PAT02_arriere_T1A": {
        "path": BASE / "patrouilleur_arriere_endommage_320x240.png",
        "sha256": "a136b18ab4d32e522bb5fd314b1faa87dde275e752d2b4bd99a267b4fdf93588",
    },
}


def checksum(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def exact(a: Image.Image, b: Image.Image) -> bool:
    return (
        a.size == b.size
        and a.mode == b.mode
        and ImageChops.difference(a, b).getbbox() is None
    )


def load_reference(path: Path, expected_sha: str) -> Image.Image:
    assert checksum(path) == expected_sha, f"Reference alteree : {path}"
    with Image.open(path) as img:
        assert img.format == "PNG" and img.mode == "RGB" and img.size == NATIVE_SIZE
        return img.copy()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="Valide sans ecrire ni modifier aucun fichier")
    args = parser.parse_args()
    assert PIL.__version__ == "12.3.0", "Installer Pillow==12.3.0"
    references = {
        key: load_reference(value["path"], value["sha256"])
        for key, value in REFERENCES.items()
    }
    if not args.check:
        (TARGET / "preuves").mkdir(parents=True, exist_ok=True)

    outputs = {}
    sources = {}
    alignment = {}
    previews = {}

    for variant, spec in SOURCES.items():
        assert checksum(spec["path"]) == spec["sha256"], f"Source PO alteree : {variant}"
        with Image.open(spec["path"]) as img:
            assert img.format == "JPEG" and img.size == SOURCE_SIZE
            full = img.convert("RGB")

        # Cadre complet identique a PAT-01/02 : aucune translation ni decoupe.
        native = full.resize(NATIVE_SIZE, Image.Resampling.LANCZOS)
        zoom = native.resize(ZOOM_SIZE, Image.Resampling.NEAREST)
        paths = {
            "native": TARGET / f"patrouilleur_{variant}_320x240.png",
            "zoom_x4": TARGET / f"patrouilleur_{variant}_x4.png",
        }

        for label, path in paths.items():
            expected = native if label == "native" else zoom
            if not args.check:
                expected.save(path, format="PNG", compress_level=9)
            assert path.is_file(), f"Export absent : {path}"
            with Image.open(path) as img:
                assert img.format == "PNG" and img.mode == "RGB"
                assert img.size == expected.size
                from_disk = img.copy()
            assert exact(from_disk, expected), f"Pixels non conformes : {path}"
            assert from_disk.getbands() == ("R", "G", "B")
            outputs[f"{variant}_{label}"] = {
                "path": str(path.relative_to(ROOT)),
                "size": list(from_disk.size),
                "mode": from_disk.mode,
                "bytes": path.stat().st_size,
                "sha256": checksum(path),
            }

        with Image.open(paths["zoom_x4"]) as img:
            zoom_disk = img.copy()
        with Image.open(paths["native"]) as img:
            native_disk = img.copy()
        assert exact(
            zoom_disk, native_disk.resize(ZOOM_SIZE, Image.Resampling.NEAREST)
        ), f"Zoom non pixel-exact : {variant}"

        # Mesure diagnostique, pas recalage : ROI sur arriere non endommage.
        roi = (110, 109, 150, 139)
        intact = references["PAT01_intact"].crop(roi)
        sample = native.crop(roi)
        deviation = ImageStat.Stat(ImageChops.difference(intact, sample)).mean
        alignment[variant] = {
            "reference": "PAT01_intact",
            "roi_native_ltrb": list(roi),
            "mean_abs_RGB_difference_at_zero_offset": round(sum(deviation) / 3, 4),
            "shift_applied_xy": [0, 0],
            "interpretation": (
                "Diagnostic local de texture arriere seulement ; "
                "ne demontre ni l'absence ni la presence d'une translation globale."
            ),
        }
        sources[variant] = {
            "path": str(spec["path"].relative_to(ROOT)),
            "dimensions": list(SOURCE_SIZE),
            "format": "JPEG",
            "sha256": spec["sha256"],
            "painted_ocean": True,
            "alpha_sprite": False,
            "artistic_retouche": False,
        }
        previews[variant] = native

    # Ecart de l'etat arriere T1A volontairement descriptif (zone avant non homologue).
    manifest = {
        "issue": 81,
        "phase": "comparatif_T2A_T2B_pour_choix_PO",
        "chosen_variant": None,
        "sources": sources,
        "references": {
            k: {"path": str(v["path"].relative_to(ROOT)), "sha256": v["sha256"]}
            for k, v in REFERENCES.items()
        },
        "geometry": {
            "source_canvas": list(SOURCE_SIZE),
            "native_canvas": list(NATIVE_SIZE),
            "zoom_canvas": list(ZOOM_SIZE),
            "native_pivot_proposed": [164, 118],
            "PAT01_ship_bbox_native_visual_approx": [106, 92, 221, 143],
            "same_full_frame_cadence_as_PAT01_PAT02": True,
            "no_translation_or_crop": True,
            "sx": NATIVE_SIZE[0] / SOURCE_SIZE[0],
            "sy": NATIVE_SIZE[1] / SOURCE_SIZE[1],
        },
        "conversion": {
            "Pillow": PIL.__version__,
            "native_filter": "LANCZOS",
            "zoom_filter": "NEAREST",
            "output_format": "PNG",
            "output_mode": "RGB",
            "alpha": False,
            "full_scene_from_PO_JPEG_no_artistic_edit": True,
        },
        "reference_alignment_diagnostic": alignment,
        "outputs": outputs,
        "checks": {
            "source_sha256_T2A_T2B": "PASS",
            "PAT01_PAT02_reference_sha256": "PASS",
            "source_size_and_format": "PASS",
            "native_RGB_size_and_pixels_LANCZOS": "PASS",
            "zoom_RGB_1280x960_NEAREST_pixel_exact": "PASS",
            "output_SHA256_recorded_and_verified": "PASS",
            "manifest_reproducible": "PASS",
        },
        "status": "DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED",
        "po_choice_required": "PAT-03 = T2A ou PAT-03 = T2B",
        "final_runtime_asset": False,
    }
    payload = json.dumps(manifest, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.check:
        assert MANIFEST.is_file(), "Manifeste manquant"
        assert MANIFEST.read_text(encoding="utf-8") == payload, "Manifeste incoherent"
    else:
        MANIFEST.write_text(payload, encoding="utf-8")
    print("PASS PAT-03 phase 1 — deux sources PO canoniques intactes")
    print("PASS 2 apercus 320x240 LANCZOS et 2 zooms 1280x960 NEAREST")
    for k, v in sorted(outputs.items()):
        print(k, v["bytes"], "bytes", v["sha256"])
    for k, v in sorted(alignment.items()):
        print("Diagnostic", k, v["mean_abs_RGB_difference_at_zero_offset"])
    print("STOP : choix PO requis ; DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED")


if __name__ == "__main__":
    main()
