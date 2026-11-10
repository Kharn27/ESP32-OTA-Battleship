#!/usr/bin/env python3
"""Recompose user-supplied G07-R2 VICTOIRE into exact CYD geometry.

This is a DESIGN_PREVIEW, not a Flash-ready asset. Requires Pillow, NumPy,
OpenCV. Originating illustration is a user-supplied flattened PNG.

All display-space grid coordinates are deterministically constructed at
320x240; OpenCV only restores sea pixels under the original cyan line art.
"""
from __future__ import annotations

import hashlib
import io
import json
import zipfile
from pathlib import Path
from xml.etree import ElementTree as ET

import cv2
import numpy as np
from PIL import Image, ImageChops, ImageDraw

D = Path(__file__).resolve().parent
SOURCE = D / "g07_r2_victoire_source_hd.png"
OUT = D / "g07_r2_victoire_320x240.png"
X4 = D / "g07_r2_victoire_x4.png"
PROOF = D / "g07_r2_victoire_geometry_proof_320x240.png"
PROOF_X4 = D / "g07_r2_victoire_geometry_proof_x4.png"
ORA = D / "g07_r2_victoire_source.ora"
REPORT = D / "g07_r2_victoire_provenance.json"

SIZE = (320, 240)
ORIGIN = (21, 53)
CELL = 18
XS = [21 + 18 * k for k in range(11)]
YS = [53 + 18 * k for k in range(11)]
GRID_COLOR = (182, 227, 237)
# Measured from the 1448x1086 user-supplied input, not inferred from preview:
OLD_XS = [159, 234, 308, 384, 457, 531, 606, 679, 756, 828, 901]
OLD_YS = [228, 302, 373, 445, 515, 585, 658, 728, 797, 869, 942]


def transparent() -> Image.Image:
    return Image.new("RGBA", SIZE, (0, 0, 0, 0))


def paste(layer: Image.Image, art: Image.Image, x: int, y: int):
    layer.paste(art.convert("RGBA"), (x, y))


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def png_blob(img: Image.Image) -> bytes:
    b = io.BytesIO()
    img.save(b, format="PNG", optimize=True)
    return b.getvalue()


def create_ora(layers: list[tuple[str, Image.Image, bool]], merged: Image.Image):
    root = ET.Element("image", {"w": "320", "h": "240", "name": "G07-R2 VICTOIRE compact", "version": "0.0.3"})
    stack = ET.SubElement(root, "stack")
    # OpenRaster layers are serialized from top-most to bottom-most.
    with zipfile.ZipFile(ORA, "w") as z:
        z.writestr("mimetype", "image/openraster", compress_type=zipfile.ZIP_STORED)
        for i, (name, layer, visible) in enumerate(reversed(layers)):
            filename = f"data/layer_{i}.png"
            ET.SubElement(stack, "layer", {
                "name": name,
                "src": filename,
                "visibility": "visible" if visible else "hidden",
                "opacity": "1.0", "x": "0", "y": "0", "composite-op": "svg:src-over"})
            z.writestr(filename, png_blob(layer.convert("RGBA")))
        z.writestr("stack.xml", ET.tostring(root, encoding="utf-8", xml_declaration=True))
        z.writestr("mergedimage.png", png_blob(merged.convert("RGBA")))
        z.writestr("Thumbnails/thumbnail.png", png_blob(merged.resize((160, 120), Image.Resampling.NEAREST)))
    # Independent verification: merging visible layers from bottom to top
    # must reconstruct the composite exactly, including its alpha channel.
    actual = transparent()
    for _name, layer, visible in layers:
        if visible:
            actual = Image.alpha_composite(actual, layer)
    assert ImageChops.difference(actual.convert("RGB"), merged.convert("RGB")).getbbox() is None, "ORA render mismatch"


def check(image: Image.Image):
    a = np.asarray(image.convert("RGB"))
    assert a.shape == (240, 320, 3), a.shape
    mask = np.all(a == np.asarray(GRID_COLOR), axis=2)
    # Detect rather than merely assume the exact lines.
    got_xs = [x for x in range(18, 205) if int(mask[53:234, x].sum()) == 181]
    got_ys = [y for y in range(50, 236) if int(mask[y, 21:202].sum()) == 181]
    assert got_xs == XS, (got_xs, XS)
    assert got_ys == YS, (got_ys, YS)
    assert all(bool(mask[53:234, x].all()) for x in XS)
    assert all(bool(mask[y, 21:202].all()) for y in YS)
    assert len(XS) == len(YS) == 11
    assert (XS[-1] - XS[0], YS[-1] - YS[0]) == (180, 180)
    return got_xs, got_ys


def main():
    with Image.open(SOURCE) as f:
        src = f.convert("RGB")
    assert src.size == (1448, 1086), f"unexpected input size {src.size}"
    native_source = src.resize(SIZE, Image.Resampling.LANCZOS)
    original = np.asarray(src).copy()
    mask = np.zeros((1086, 1448), dtype=np.uint8)
    for x in OLD_XS:
        mask[223:948, x-7:x+8] = 255
    for y in OLD_YS:
        mask[y-7:y+8, 155:906] = 255
    # Restore continuous maritime sea under the already-rasterized grid.
    x0, y0, x1, y1 = (151, 218, 910, 950)
    cleaned = cv2.inpaint(original[y0:y1, x0:x1], mask[y0:y1, x0:x1], 4, cv2.INPAINT_TELEA)
    water = Image.fromarray(cleaned).crop((15, 16, 744, 718)).resize((181, 181), Image.Resampling.LANCZOS)
    head = src.crop((159, 158, 903, 228)).resize((181, 17), Image.Resampling.LANCZOS)
    labels = src.crop((89, 228, 159, 942)).resize((21, 181), Image.Resampling.LANCZOS)
    bottom = src.crop((159, 941, 902, 975)).resize((181, 6), Image.Resampling.LANCZOS)

    # Keep the whole approved right-hand panel pixel-for-pixel unmodified.
    # Blank the old left playfield in the ORA background so the rebuilt parts
    # can be toggled as independent editable raster layers.
    backdrop = native_source.convert("RGBA")
    db = ImageDraw.Draw(backdrop)
    db.rectangle((21, 36, 201, 239), fill=(44, 19, 10, 255))
    db.rectangle((0, 53, 20, 233), fill=(44, 19, 10, 255))
    header = transparent(); paste(header, head, 21, 36)
    side = transparent(); paste(side, labels, 0, 53)
    sea = transparent(); paste(sea, water, 21, 53)
    grid = transparent(); g = ImageDraw.Draw(grid)
    for x in XS: g.line((x, 53, x, 233), fill=(*GRID_COLOR, 255), width=1)
    for y in YS: g.line((21, y, 201, y), fill=(*GRID_COLOR, 255), width=1)
    rail = transparent(); paste(rail, bottom, 21, 234)
    ImageDraw.Draw(rail).line((21,234,201,234),fill=(244,144,23,255),width=1)

    visible = [
       ("Fond original — ciel, phare, commandes à droite (raster aplati)", backdrop, True),
       ("Bandeau A–J — boiseries du visuel source", header, True),
       ("Réglette 1–10 — boiseries du visuel source", side, True),
       ("Mer et impacts — source recomposee et raccordee (raster)", sea, True),
       ("Quadrillage deterministe 18px — RVB 182,227,237", grid, True),
       ("Traverse basse — laiton et bois", rail, True),
    ]
    result = transparent()
    for _, layer, show in visible:
        if show:
            result = Image.alpha_composite(result, layer)
    result = result.convert("RGB")
    measured_xs, measured_ys = check(result)
    # Every pixel on the right remains exactly the input downsample (RGB).
    right_unchanged = int(np.count_nonzero(np.any(np.asarray(result)[:, 202:] != np.asarray(native_source)[:, 202:], axis=2)))
    assert right_unchanged == 0, right_unchanged
    shift = Image.new("RGB", SIZE)
    shift.paste(result.crop((0, 0, 319, 240)), (1, 0))
    try:
        check(shift)
    except AssertionError:
        negative_rejected = True
    else:
        negative_rejected = False
    assert negative_rejected, "negative +1 px translation must fail"

    result.save(OUT, format="PNG", optimize=True)
    zoom = result.resize((1280, 960), Image.Resampling.NEAREST)
    zoom.save(X4, format="PNG", optimize=True)
    assert ImageChops.difference(zoom, Image.open(X4).convert("RGB")).getbbox() is None
    assert ImageChops.difference(zoom.resize(SIZE, Image.Resampling.NEAREST), result).getbbox() is None
    assert all(np.all(np.asarray(zoom)[y*4:(y+1)*4, x*4:(x+1)*4] == np.asarray(result)[y,x])
               for x,y in [(21,53),(20,52),(39,71),(123,144),(201,233),(299,168)])

    proof = result.copy()
    pd = ImageDraw.Draw(proof)
    # Explicit diagnostics: corners and outer limits, not for the UI.
    for x,y in [(21,53),(201,53),(21,233),(201,233)]:
        pd.rectangle((x-3,y-3,x+3,y+3),outline=(255,255,20),width=1)
        pd.rectangle((x-1,y-1,x+1,y+1),fill=(227,96,120))
    pd.line((20, 53, 20, 233), fill=(255,255,20), width=1)
    pd.line((21, 234, 201, 234), fill=(255,255,20), width=1)
    proof.save(PROOF,format="PNG",optimize=True)
    proof.resize((1280, 960), Image.Resampling.NEAREST).save(PROOF_X4,format="PNG",optimize=True)
    visible.append(("DIAGNOSTIC — repères uniquement (MASQUÉ)", Image.new("RGBA", SIZE), False))
    create_ora(visible, result)

    report = {
       "issue": 53, "pr": 74, "screen": "G07-R2 — IMAGE 1/7, VICTOIRE compact VS Capitaine",
       "status": "DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED ; PO/Pilotage pending ; PR Draft",
       "origin": {"type": "PNG fourni par le PO le 2026-11-06", "file": SOURCE.name,
                  "size": list(src.size), "sha256": sha256(SOURCE),
                  "rights": "Fichier fourni dans la conversation, droits de re-utilisation de source non expertises"},
       "geometry": {"origin": list(ORIGIN),"end":[201,233],"size":[180,180],"cells":[10,10],
                    "cell_size":[18,18],"line_rgb":list(GRID_COLOR),"x_lines":measured_xs,"y_lines":measured_ys},
       "assertions_PC": {"native_320x240":"PASS","all_22_line_segments_exact_rgb":"PASS",
                         "one_pixel_right_shift_negative":"REJECTED_AS_EXPECTED",
                         "nearest_x4_no_interpolation":"PASS","unchanged_panel_x_ge_202":"PASS",
                         "pixel_changes_in_right_panel":right_unchanged,
                         "screen_legibility":"VISUAL_REVIEW_PC_ONLY",
                         "on_device":"NOT_TESTED","firmware_sprites":"NOT_PREPARED"},
       "layers": [name for name, _, _ in visible],
       "limitations": ["Maritime scene comes from one flattened user PNG", "Fire/splashes/sea remain a single raster layer",
                       "Button/icons/text on right panel remain flattened", "No proof of two-last-shot synchronized reveal", "No physical touch/render checks"],
       "exports": [{"file":p.name,"bytes":p.stat().st_size,"sha256":sha256(p)}
                   for p in [OUT,X4,PROOF,PROOF_X4,ORA]],
    }
    REPORT.write_text(json.dumps(report,ensure_ascii=False,indent=2)+"\n", encoding="utf-8")
    print("PASS VICTOIRE 320x240 grid 21,53 to 201,233; X/Y 11 each; exact x4; panel unchanged; negative shift rejected")


if __name__ == "__main__":
    main()
