#!/usr/bin/env python3
"""PR #90 — compose le vrai accueil 320x240 à partir des quatre sources PO.

Exécutable dans GitHub Actions, sans retouche artistique des quatre originaux.
Les coordonnées sont une PROPOSITION soumise à la revue du PO.
"""
from __future__ import annotations

import argparse
import hashlib
import io
import json
from pathlib import Path
from zipfile import ZipFile, ZipInfo, ZIP_STORED, ZIP_DEFLATED
from xml.etree import ElementTree as ET

import PIL
from PIL import Image, ImageDraw

CANVAS = (320, 240)
LAYOUT = {
    "logo": (88, 8, 145, 66),
    "jouer": (14, 198, 140, 32),
    "palmares": (166, 199, 140, 30),
}
TOUCH = {
    "JOUER": (8, 190, 152, 46),
    "PALMARÈS": (160, 190, 152, 46),
}
SOURCES = {
    "fond": ("draft/Wallpaper_accueil.jpeg", (1195, 896), "e3c0603405fffbf361ca67b90f265d6fb7994a26"),
    "logo": ("design/ecrans/accueil/assets/Logo_titre_RGBA.png", (1075, 489), "010bb56a6c8056924d9b1af1ceeb99482e18fa3a"),
    "jouer": ("design/ecrans/accueil/assets/Bouton_jouer_RGBA.png", (1107, 252), "f81c510060af5aeee231bea50df420d04db7b320"),
    "palmares": ("design/ecrans/accueil/assets/Bouton_palmares_RGBA.png", (1163, 250), "06a53829d508ae7366338b14a187a655edd6505c"),
}
SPRITES = {
    "logo": "logo_titre_145x66.png",
    "jouer": "bouton_jouer_140x32.png",
    "palmares": "bouton_palmares_140x30.png",
}


def sha256(raw: bytes) -> str:
    return hashlib.sha256(raw).hexdigest()


def git_blob_sha(raw: bytes) -> str:
    prefix = ("blob " + str(len(raw)) + "\0").encode("ascii")
    return hashlib.sha1(prefix + raw).hexdigest()


def as_png(image: Image.Image) -> bytes:
    out = io.BytesIO()
    image.save(out, format="PNG", optimize=True)
    return out.getvalue()


def save_png(image: Image.Image, path: Path):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(as_png(image))
    with Image.open(path) as check:
        assert check.format == "PNG" and check.size == image.size and check.mode == image.mode
        assert check.tobytes() == image.tobytes(), "Écart pixels après réouverture : " + str(path)


def build(repo: Path, out: Path):
    assert PIL.__version__ == "12.3.0", "Pillow doit être figé à 12.3.0"
    originals = {}
    images = {}
    for key, (rel, dimensions, sha_expected) in SOURCES.items():
        path = repo / rel
        source_bytes = path.read_bytes()
        sha = git_blob_sha(source_bytes)
        assert sha == sha_expected, f"Source originale changée : {rel}, {sha}"
        with Image.open(io.BytesIO(source_bytes)) as src:
            src.load()
            assert src.size == dimensions, f"Dimensions source inattendues : {rel}"
            if key == "fond":
                assert src.format == "JPEG"
                images[key] = src.convert("RGB").resize(CANVAS, Image.Resampling.LANCZOS)
            else:
                assert src.format == "PNG" and src.mode == "RGBA"
                w, h = LAYOUT[key][2:]
                ratio_delta = abs((w / h) / (src.width / src.height) - 1)
                assert ratio_delta < 0.005, f"Ratio altéré : {key}"
                images[key] = src.copy().resize((w, h), Image.Resampling.LANCZOS)
                assert images[key].getchannel("A").getextrema()[0] == 0
        originals[key] = {
            "path": rel,
            "dimensions": list(dimensions),
            "bytes": len(source_bytes),
            "git_blob_sha1": sha,
            "sha256": sha256(source_bytes),
        }

    layers = {"fond": images["fond"].convert("RGBA")}
    for key in ("logo", "jouer", "palmares"):
        layer = Image.new("RGBA", CANVAS, (0, 0, 0, 0))
        layer.alpha_composite(images[key], LAYOUT[key][:2])
        layers[key] = layer
        save_png(images[key], out / "sprites" / SPRITES[key])

    merged = layers["fond"].copy()
    for key in ("logo", "jouer", "palmares"):
        merged = Image.alpha_composite(merged, layers[key])
    assert merged.getchannel("A").getextrema() == (255, 255)

    native = merged.convert("RGB")
    zoom = native.resize((1280, 960), Image.Resampling.NEAREST)
    save_png(native, out / "accueil_cyd_warship_320x240.png")
    save_png(zoom, out / "accueil_cyd_warship_x4.png")
    assert zoom.tobytes() == native.resize((1280, 960), Image.Resampling.NEAREST).tobytes()

    # OpenRaster éditable : quatre calques placés sur la toile entière.
    stack = ET.Element("image", {
        "w": "320", "h": "240", "name": "CYD WARSHIP — accueil PR90", "version": "0.0.3"
    })
    root_stack = ET.SubElement(stack, "stack")
    for key, title in (
        ("palmares", "03 - PALMARES"),
        ("jouer", "02 - JOUER"),
        ("logo", "01 - CYD WARSHIP"),
        ("fond", "00 - Wallpaper"),
    ):
        ET.SubElement(root_stack, "layer", {
            "name": title, "src": f"data/{key}.png", "x": "0", "y": "0",
            "opacity": "1.0", "visibility": "visible", "composite-op": "svg:src-over"
        })
    ora = out / "accueil_cyd_warship_4_calques.ora"
    ora.parent.mkdir(parents=True, exist_ok=True)
    # OpenRaster est un ZIP : des dates variables rendraient l'archive non reproductible.
    # Chaque entree utilise des metadonnees stables, independantes de l'heure CI.
    def add_stable(z, name: str, data: bytes, compression: int):
        zi = ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
        zi.compress_type = compression
        zi.create_system = 3
        zi.external_attr = 0o644 << 16
        z.writestr(zi, data)

    with ZipFile(ora, "w") as z:
        add_stable(z, "mimetype", b"image/openraster", ZIP_STORED)
        add_stable(z, "stack.xml", ET.tostring(stack, encoding="utf-8", xml_declaration=True), ZIP_DEFLATED)
        for key, layer in layers.items():
            add_stable(z, f"data/{key}.png", as_png(layer), ZIP_DEFLATED)
        add_stable(z, "mergedimage.png", as_png(merged), ZIP_DEFLATED)
        add_stable(z, "Thumbnails/thumbnail.png", as_png(native.resize((256, 192), Image.Resampling.NEAREST)), ZIP_DEFLATED)
    with ZipFile(ora) as z:
        assert z.namelist()[0] == "mimetype"
        assert z.read("mimetype") == b"image/openraster"
        assert len(ET.fromstring(z.read("stack.xml")).find("stack").findall("layer")) == 4

    # Preuves alpha : vraie transparence des trois éléments sur fond clair/sombre.
    alpha_sheet = Image.new("RGB", (640, 720))
    for row, key in enumerate(("logo", "jouer", "palmares")):
        for col, background in enumerate(((240, 244, 244, 255), (14, 23, 37, 255))):
            preview = Image.new("RGBA", CANVAS, background)
            preview.alpha_composite(layers[key])
            alpha_sheet.paste(preview.convert("RGB"), (col * 320, row * 240))
    save_png(alpha_sheet, out / "preuves" / "alpha_clair_sombre_640x720.png")

    annotated = native.copy()
    draw = ImageDraw.Draw(annotated)
    for name, (x, y, w, h) in TOUCH.items():
        draw.rectangle((x, y, x+w-1, y+h-1), outline=(255, 235, 70), width=1)
        draw.text((x+3, y-12), name, fill="white", stroke_width=1, stroke_fill="black")
    save_png(annotated, out / "preuves" / "zones_tactiles_320x240.png")
    assert TOUCH["JOUER"][0] + TOUCH["JOUER"][2] == TOUCH["PALMARÈS"][0]
    assert all(0 <= y and y+h <= 240 for (x,y,w,h) in TOUCH.values())

    # Ne mesurer que les livrables générés, pas les sources ou les README du dossier.
    names = [
        "accueil_cyd_warship_320x240.png",
        "accueil_cyd_warship_x4.png",
        "accueil_cyd_warship_4_calques.ora",
        "sprites/logo_titre_145x66.png",
        "sprites/bouton_jouer_140x32.png",
        "sprites/bouton_palmares_140x30.png",
        "preuves/alpha_clair_sombre_640x720.png",
        "preuves/zones_tactiles_320x240.png",
    ]
    exported = {}
    for name in sorted(names):
        raw = (out / name).read_bytes()
        exported[name] = {"bytes": len(raw), "sha256": sha256(raw)}
    manifest = {
        "title": "CYD WARSHIP — PR90 accueil quatre calques",
        "detourage_source_commit": "5fd1ce6efd7a0c0b6f014a3b3da9fb323774570d",
        "source_status": "JPEG PO originaux inchanges; RGBA detailles dans le meme dossier que l ecran canonique",
        "status": "DESIGN_PREVIEW — PO_REVIEW_PENDING — ON_DEVICE_UNVERIFIED",
        "tool": {"name":"Pillow", "version": PIL.__version__, "native_filter":"LANCZOS", "zoom_filter":"NEAREST"},
        "canvas": [320, 240],
        "layout_proposed": {key: dict(zip(("x", "y", "width", "height"), box)) for key, box in LAYOUT.items()},
        "touch_proposed": {key: dict(zip(("x", "y", "width", "height"), box)) for key, box in TOUCH.items()},
        "sources": originals, "exports": exported,
        "limitations": [
            "Composition proposée et non validée par le PO",
            "Liserés alpha : inspection artistique humaine encore nécessaire",
            "Aucun essai sur ESP32 CYD, aucun contrat mémoire/Flash implicite",
            "Les sources JPEG et PNG originaux sont restés inchangés"
        ],
    }
    out.joinpath("manifest.json").write_text(json.dumps(manifest, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print("PASS PR90 — vrais PNG 320x240, zoom x4 NEAREST, sprites RGBA, OpenRaster 4 couches")
    for name, info in exported.items():
        print(f"{name}: {info['bytes']} octets — sha256 {info['sha256']}")
    print("DESIGN_PREVIEW / PO_REVIEW_PENDING / ON_DEVICE_UNVERIFIED")


def verify_existing(repo: Path, out: Path):
    """Regénère hors du dépôt, compare les octets et n'altère aucun fichier."""
    from tempfile import TemporaryDirectory

    with TemporaryDirectory(prefix="accueil-cyd-check-") as tmp:
        generated = Path(tmp)
        build(repo, generated)
        for fresh in sorted(generated.rglob("*")):
            if fresh.is_file():
                rel = fresh.relative_to(generated)
                existing = out / rel
                assert existing.is_file(), f"Export absent : {existing}"
                assert existing.read_bytes() == fresh.read_bytes(), f"Export different : {rel}"
        print("PASS accueil canonique --check : ORA, PNG, manifest et sprites reproductibles")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, required=True)
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--check", action="store_true", help="Regenerer dans un temporaire et comparer sans modifier")
    args = ap.parse_args()
    repo, out = args.repo.resolve(), args.out.resolve()
    if args.check:
        verify_existing(repo, out)
    else:
        build(repo, out)
