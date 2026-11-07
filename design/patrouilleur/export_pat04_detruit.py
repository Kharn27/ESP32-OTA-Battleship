#!/usr/bin/env python3
"""PAT-04 #82 — fusion du PO, remplacement de B, exports reproductibles."""
from __future__ import annotations

import argparse
import hashlib
import io
import json
from pathlib import Path

import PIL
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "design/patrouilleur"
ORIGINAL = ROOT / "draft/patrouilleur_B_PO_fusion.png"
JPEG = ROOT / "draft/patrouilleur_B.jpeg"
NATIVE = BASE / "patrouilleur_detruit_320x240.png"
ZOOM = BASE / "patrouilleur_detruit_x4.png"
MANIFEST = BASE / "preuves/pat04_export_manifest.json"

SHA_PNG = "e004d3b1274133f118ab407e7854d2050121654cca0881b513c7acbc3bd1a808"
SHA_JPEG = "0f6eb6a9dd4df3974a1cee5423d304930d4b1643c3612847e7599128fd374830"
SHA_NATIVE = "8681312240ed81e4e2951b54fb4f8066dc6cbdbcc85aaa256bb848ffb6f3870f"
SHA_ZOOM = "c2bfa2de4b37f8e0db255202b961efe0f9e2493a182760bb52bc168b57e7328f"
REFS = {
    "PAT-01": ("patrouilleur_intact_320x240.png", "84b38a704d9ded44f9cc880a1af6ebf092a87a2803c8c972e24f73df9cf808fb"),
    "PAT-02_T1A": ("patrouilleur_arriere_endommage_320x240.png", "a136b18ab4d32e522bb5fd314b1faa87dde275e752d2b4bd99a267b4fdf93588"),
    "PAT-03_T2A": ("patrouilleur_avant_endommage_320x240.png", "c079ba0e2e79ebbbcbb31e4a046d7c337dd1e1164aff1a678955d54633ceb36c"),
}

def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()

def record(path: Path, size: tuple[int, int]) -> dict:
    return {"path": str(path.relative_to(ROOT)), "dimensions": list(size),
            "bytes": path.stat().st_size, "sha256": sha(path)}

def same(a: Image.Image, b: Image.Image) -> bool:
    return (a.mode == b.mode and a.size == b.size
            and ImageChops.difference(a, b).getbbox() is None)

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Verifier sans ecriture")
    opt = parser.parse_args()
    assert PIL.__version__ == "12.3.0", "Pillow==12.3.0 requis"
    assert sha(ORIGINAL) == SHA_PNG, "PNG transmis par PO non conforme"
    with Image.open(ORIGINAL) as file:
        assert file.format == "PNG" and file.mode == "RGB" and file.size == (1024, 768)
        source = file.copy()

    # Remplacement demandé explicitement par le PO. Le master PNG reste intact.
    memory = io.BytesIO()
    source.save(memory, format="JPEG", quality=98, subsampling=0, optimize=True)
    expected = memory.getvalue()
    assert hashlib.sha256(expected).hexdigest() == SHA_JPEG, "JPEG non reproductible"
    if not opt.check:
        JPEG.write_bytes(expected)
    assert JPEG.read_bytes() == expected, "B.jpeg ne correspond pas au PO"

    with Image.open(JPEG) as file:
        assert file.format == "JPEG" and file.size == (1024, 768)
        rgb = file.convert("RGB")
    native = rgb.resize((320, 240), Image.Resampling.LANCZOS)
    zoom = native.resize((1280, 960), Image.Resampling.NEAREST)
    if not opt.check:
        MANIFEST.parent.mkdir(parents=True, exist_ok=True)
        native.save(NATIVE, format="PNG", compress_level=9)
        zoom.save(ZOOM, format="PNG", compress_level=9)

    for path, expected_image, dims, digest in (
        (NATIVE, native, (320, 240), SHA_NATIVE),
        (ZOOM, zoom, (1280, 960), SHA_ZOOM),
    ):
        with Image.open(path) as f:
            assert f.format == "PNG" and f.mode == "RGB" and f.size == dims
            assert same(f.copy(), expected_image), f"Pixels incorrects: {path}"
        assert sha(path) == digest, f"Empreinte non conforme: {path}"

    with Image.open(NATIVE) as n, Image.open(ZOOM) as z:
        assert same(z.copy(), n.convert("RGB").resize((1280, 960), Image.Resampling.NEAREST))

    refs = {}
    for name, (filename, digest) in REFS.items():
        path = BASE / filename
        assert sha(path) == digest, f"Autre etat accepte modifie : {name}"
        with Image.open(path) as f:
            assert f.format == "PNG" and f.mode == "RGB" and f.size == (320, 240)
        refs[name] = {"path": str(path.relative_to(ROOT)), "sha256": digest}

    manifest = {
        "issue": 82,
        "selection_PO": "nouvelle image fusion avant/arriere jointe 07/11/2026",
        "source_originale_PNG_preservee": record(ORIGINAL, (1024, 768)),
        "source_JPEG_remplacee": {
            **record(JPEG, (1024, 768)),
            "encodage": "Pillow 12.3.0 JPEG quality=98 subsampling=0 optimize=True",
            "ancien_git_blob": "06c76f842a9d7dbbffb36044d55bdb27c61e8a69",
        },
        "references_precedentes_inchangees": refs,
        "export_RGB_320x240": {**record(NATIVE, (320, 240)), "resampling": "LANCZOS", "alpha": False},
        "zoom_RGB_1280x960": {**record(ZOOM, (1280, 960)), "resampling": "NEAREST", "alpha": False},
        "geometrie": {
            "source_plein_cadre": [0, 0, 1024, 768],
            "export_plein_cadre": [0, 0, 320, 240],
            "recadrage": False, "translation": [0, 0],
            "mer_peinte": True, "alpha": False,
            "pivot_PAT01_indicatif": [164, 118], "alignement_pixel_non_demontre": True,
        },
        "checks": {
            "master_PO_SHA256": "PASS", "remplacement_JPEG": "PASS",
            "PAT01_PAT02_PAT03_SHA256": "PASS", "natif_LANCZOS": "PASS",
            "zoom_NEAREST_pixel_par_pixel": "PASS", "exports_SHA256": "PASS",
            "manifest_reproductible": "PASS",
        },
        "version_Pillow": PIL.__version__,
        "status": "DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED",
        "asset_firmware_certifie": False,
    }
    data = json.dumps(manifest, indent=2, ensure_ascii=False, sort_keys=True) + "\n"
    if opt.check:
        assert MANIFEST.read_text(encoding="utf-8") == data, "Manifeste divergent"
    else:
        MANIFEST.write_text(data, encoding="utf-8")
    print("PASS PAT-04 #82 — PNG PO conserve ; patrouilleur_B.jpeg remplace")
    print("PASS 320x240 RGB LANCZOS ; zoom 1280x960 NEAREST strict")
    print("PASS references PAT-01/02/03 SHA-256, manifest et --check")
    print("DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED")

if __name__ == "__main__":
    main()
