#!/usr/bin/env python3
"""G02: one canonical chain SVG -> 320x240 PNG -> NEAREST 1280x960 PNG."""
from pathlib import Path
from PIL import Image, ImageChops
import cairosvg

ROOT=Path(__file__).resolve().parent

def render(name):
    src=ROOT/(name+"_320x240.svg")
    png=ROOT/(name+"_320x240.png")
    zoom=ROOT/(name+"_x4.png")
    cairosvg.svg2png(url=str(src),write_to=str(png),
                     output_width=320,output_height=240)
    with Image.open(png) as original:
        assert original.size==(320,240),(name,original.size)
        up=original.convert("RGBA").resize((1280,960),Image.Resampling.NEAREST)
        up.save(zoom,optimize=True)
        with Image.open(zoom) as saved:
            assert saved.size==(1280,960)
            assert ImageChops.difference(saved.convert("RGBA"),up).getbbox() is None
            # Explicitly verify every 4x4 pixel block equals one native pixel.
            pix=original.convert("RGBA")
            assert all(saved.getpixel((x*4,y*4))==pix.getpixel((x,y))
                       for x in range(320) for y in range(240))
    print(f"PASS {name}: PNG 320x240, x4 1280x960, exact NEAREST")

for name in ("g02_grille","g02_etats","g02_orientations"):
    render(name)
cairosvg.svg2png(url=str(ROOT/"g02_planche_flotte.svg"),
                 write_to=str(ROOT/"g02_planche_flotte.png"),
                 output_width=960,output_height=560)
with Image.open(ROOT/"g02_planche_flotte.png") as im:
    assert im.size==(960,560)
print("PASS planche 960x560 — DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED")
