#!/usr/bin/env python3
"""BTN-ABD-01 #149 : audit Git original, filiation pixel et comparaison artistique.

Le contrôle ne modifie aucun fichier. Les SHA-1 des sprites approuvés sont les
blobs exacts de main. Les PNG de preuve sont comparés en pixels RGBA (et non
selon leur compression). Les états restent CANDIDATE / ON_DEVICE_UNVERIFIED.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path
from PIL import Image, ImageDraw

ORIGINAL = "3818cb092456e1ff222d8b2dacf9f9aa55266135"
UPLOAD_PIXEL_SHA256 = "da7ca8f59aac3c27e02d9fb0b5966ee6ea048389221834610c1d11845ec818a5"
UPLOAD_SIZE = (2048, 682)
APPROUVES = (
    ("assets/boutons/bouton_jouer_CYD_140x32_RGBA.png", "ddd7f95eee37ba3759a108b92f536c515d8d78e8", (140, 32)),
    ("assets/boutons/bouton_palmares_CYD_140x30_RGBA.png", "0c3ac936ebc0c9cbe20d3e9e0c5fe29be289985d", (140, 30)),
    ("assets/boutons/attaquer_on_140x35_RGBA.png", "7ac6667492a96e4a2ad6d7445ab68a504576324b", (140, 35)),
)
CANDIDAT = "2a2f274fc640266bc4cd05ba9b28e39eb836561e"
PIXEL_FONT = {
    "A": ("01110", "10001", "10001", "11111", "10001", "10001", "10001"),
    "B": ("11110", "10001", "10001", "11110", "10001", "10001", "11110"),
    "C": ("01111", "10000", "10000", "10000", "10000", "10000", "01111"),
    "D": ("11110", "10001", "10001", "10001", "10001", "10001", "11110"),
    "E": ("11111", "10000", "10000", "11110", "10000", "10000", "11111"),
    "F": ("11111", "10000", "10000", "11110", "10000", "10000", "10000"),
    "I": ("11111", "00100", "00100", "00100", "00100", "00100", "11111"),
    "L": ("10000", "10000", "10000", "10000", "10000", "10000", "11111"),
    "M": ("10001", "11011", "10101", "10101", "10001", "10001", "10001"),
    "N": ("10001", "11001", "10101", "10011", "10001", "10001", "10001"),
    "O": ("01110", "10001", "10001", "10001", "10001", "10001", "01110"),
    "P": ("11110", "10001", "10001", "11110", "10000", "10000", "10000"),
    "R": ("11110", "10001", "10001", "11110", "10100", "10010", "10001"),
    "S": ("01111", "10000", "10000", "01110", "00001", "00001", "11110"),
    "T": ("11111", "00100", "00100", "00100", "00100", "00100", "00100"),
    "U": ("10001", "10001", "10001", "10001", "10001", "10001", "01110"),
    "V": ("10001", "10001", "10001", "10001", "10001", "01010", "00100"),
    " ": ("00000",) * 7,
}


def h(data):
    return hashlib.sha256(data).hexdigest()


def blob(data):
    return hashlib.sha1(b"blob " + str(len(data)).encode() + b"\0" + data).hexdigest()


def rgba(path):
    with Image.open(path) as x:
        return x.convert("RGBA")


def label(im, x, y, text, color):
    for c in text:
        for j, row in enumerate(PIXEL_FONT.get(c, PIXEL_FONT[" "])):
            for i, b in enumerate(row):
                if b == "1":
                    im.putpixel((x + i, y + j), color)
        x += 6


def planche(images):
    bg = Image.new("RGBA", (320, 240), (16, 31, 44, 255))
    dr = ImageDraw.Draw(bg)
    dr.rectangle((160, 0, 319, 239), fill=(214, 224, 220, 255))
    dr.line((159, 0, 159, 239), fill=(138, 159, 159, 255))
    label(bg, 10, 10, "FOND SOMBRE", (239, 234, 207, 255))
    label(bg, 171, 10, "FOND CLAIR", (27, 40, 49, 255))
    for x0, x1 in ((8, 149), (169, 310)):
        for y in (164, 211):
            dr.line((x0, y, x1, y), fill=(110, 131, 145, 255))
    label(bg, 10, 220, "PO A VALIDER", (239, 234, 207, 255))
    label(bg, 170, 220, "ETUDE", (27, 40, 49, 255))
    for im, y in zip(images, (34, 78, 121, 169)):
        bg.alpha_composite(im, (10, y))
        bg.alpha_composite(im, (170, y))
    return bg


def must_match(path, expected):
    assert path.is_file(), f"CHECK FAIL: fichier absent : {path}"
    im = rgba(path)
    if im.size != expected.size or im.tobytes() != expected.tobytes():
        raise AssertionError(f"CHECK FAIL: pixels RGBA differents : {path}")


def verify_source(path):
    data = path.read_bytes()
    assert blob(data) == ORIGINAL, f"CHECK FAIL: original Git non conforme : {blob(data)}"
    im = rgba(path)
    pixels = h(im.tobytes())
    print(f"Original Git: {len(data)} octets; SHA-256: {h(data)}; image={im.size}; RGBA SHA256={pixels}")
    if im.size != UPLOAD_SIZE or pixels != UPLOAD_PIXEL_SHA256:
        raise AssertionError(
            "CHECK FAIL: pixels de l'original Git differents de la reference upload. "
            "Pas de filiation PO certifiee. STOP avant regeneration."
        )
    print("SOURCE PO PIXELS PASS: pixels RGBA identiques a la piece jointe 2048x682")
    return im


def verify_comparatif(repo, out):
    src = []
    for rel, ident, size in APPROUVES:
        p = repo / rel
        raw = p.read_bytes()
        assert blob(raw) == ident, f"CHECK FAIL: reference approuvee modifiee : {rel}"
        im = rgba(p)
        assert im.size == size, f"CHECK FAIL: taille reference incorrecte {rel}"
        src.append(im)
    cand = out / "exports/abandon_140x35_RGBA.png"
    assert blob(cand.read_bytes()) == CANDIDAT, "CHECK FAIL: candidat 140x35 different de la version revue"
    src.append(rgba(cand))
    expected = planche(src)
    must_match(out / "preuves/comparatif_approuves_320x240.png", expected)
    must_match(
        out / "preuves/comparatif_approuves_1280x960_NEAREST.png",
        expected.resize((1280, 960), Image.Resampling.NEAREST),
    )
    print("COMPARATIF PASS: 4 vrais blobs, fonds clair/sombre, 320x240 + zoom x4 NEAREST")


def verify_full(repo, out, original):
    po = verify_source(original)
    import generate
    master, _, _ = generate.crop_to_master(po)
    large = generate.scale_premultiplied(master, 140, 35)
    small = generate.scale_premultiplied(master, 112, 28)
    expected = {
        "exports/abandon_master_2000x500_RGBA.png": master,
        "exports/abandon_140x35_RGBA.png": large,
        "exports/abandon_112x28_RGBA.png": small,
        "preuves/abandon_140x35_x4_NEAREST.png": generate.nearest4(large),
        "preuves/abandon_112x28_x4_NEAREST.png": generate.nearest4(small),
        "preuves/etude_abandon_320x240.png": generate.preview_single(large, small),
        "preuves/etude_abandon_1280x960_NEAREST.png": generate.nearest4(generate.preview_single(large, small)),
    }
    declaration = json.loads((out / "rapport_reference_upload.json").read_text(encoding="utf-8"))
    assert declaration.get("origin_type") == "user_upload_visual_reference_NOT_exact_git_blob"
    for rel, im in expected.items():
        p = out / rel
        must_match(p, im)
        assert h(p.read_bytes()) == declaration["files"][rel]["sha256"], f"CHECK FAIL: SHA-256 faux {rel}"
    verify_comparatif(repo, out)
    print("CHECK PASS: generate.py --check, 9 PNG en pixels RGBA, filiation PO, rapport_reference_upload.json coherent. NO FILES WRITTEN")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--repo", type=Path, default=Path("."))
    p.add_argument("--original", type=Path, default=Path("draft/Bouton_Abandon.png"))
    p.add_argument("--check-source", action="store_true")
    p.add_argument("--check-comparatif", action="store_true")
    a = p.parse_args()
    repo = a.repo.resolve()
    out = repo / "draft/etudes/bouton_abandon"
    original = a.original if a.original.is_absolute() else repo / a.original
    if a.check_comparatif:
        verify_comparatif(repo, out)
    elif a.check_source:
        verify_source(original)
    else:
        verify_full(repo, out, original)


if __name__ == "__main__":
    main()
