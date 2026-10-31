#!/usr/bin/env python3
"""G02 #30: rasteriser les sources SVG versionnees sans interpolation.
Outil hote de preview uniquement. Ne produit aucun asset firmware.
Dependencies: pip install cairosvg pillow
"""
from pathlib import Path
from PIL import Image
import cairosvg
root = Path(__file__).resolve().parent
for name in ("g02_grille", "g02_etats", "g02_orientations"):
    src = root / (name + "_320x240.svg")
    target = root / (name + "_320x240.png")
    cairosvg.svg2png(url=str(src),write_to=str(target),output_width=320,output_height=240)
    with Image.open(target) as im:
        assert im.size == (320, 240), (name,im.size)
        im.convert("RGBA").resize((1280,960),Image.Resampling.NEAREST).save(root / (name + "_x4.png"),optimize=True)
        assert Image.open(root / (name+"_x4.png")).size == (1280,960)
    print(name, "PASS native=320x240, x4=1280x960 (nearest)")
cairosvg.svg2png(url=str(root / "g02_planche_flotte.svg"),write_to=str(root / "g02_planche_flotte.png"),output_width=960,output_height=560)
print("G02 DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED; validation PO requise.")
