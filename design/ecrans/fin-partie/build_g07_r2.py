#!/usr/bin/env python3
"""G07-R2 PC preview: pixel-measured grid, deterministic 320x240 & NEAREST x4."""
import hashlib
import json
from pathlib import Path
import numpy as np
from PIL import Image, ImageChops, ImageDraw

D = Path(__file__).resolve().parent
SRC = D / "g07_r2_source_320x240.png"
NATIVE = D / "g07_r2_defaite_320x240.png"
ZOOM = D / "g07_r2_defaite_x4.png"
PROOF = D / "g07_r2_geometry_proof_x4.png"
MANIFEST = D / "g07_r2_provenance.json"

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def detect_grid(image):
    """Independently measure this raster's 1px cyan gridlines."""
    a = np.asarray(image.convert("RGB"))
    if a.shape != (240, 320, 3):
        raise AssertionError(f"Expected 320x240 RGB, got {a.shape}")
    cyan = np.all(a == [156, 214, 220], axis=2)
    x = [i for i in range(17,207) if int(cyan[55:233, i].sum()) >= 170]
    y = [i for i in range(48,234) if int(cyan[i,22:200].sum()) >= 170]
    wanted_x = list(range(21,202,18))
    wanted_y = list(range(53,234,18))
    if x != wanted_x or y != wanted_y:
        raise AssertionError(f"Grid nonconformant: x={x} y={y}, expected {wanted_x}/{wanted_y}")
    return x,y

with Image.open(SRC) as source:
    image = source.convert("RGB")
x,y = detect_grid(image)
shifted = Image.new("RGB",image.size)
shifted.paste(image.crop((0,0,319,240)),(1,0))
try:
    detect_grid(shifted)
except AssertionError:
    pass
else:
    raise AssertionError("Negative test failed: translated grid incorrectly accepted")

image.save(NATIVE,optimize=True)
zoom = image.resize((1280,960),Image.Resampling.NEAREST)
zoom.save(ZOOM,optimize=True)
with Image.open(NATIVE) as n, Image.open(ZOOM) as z:
    assert n.size == (320,240) and z.size == (1280,960)
    assert ImageChops.difference(n.resize(z.size,Image.Resampling.NEAREST),z).getbbox() is None

proof = zoom.copy()
draw = ImageDraw.Draw(proof)
for xx in x:
    draw.line((xx*4,53*4,xx*4,233*4),fill=(255,80,64),width=1)
for yy in y:
    draw.line((21*4,yy*4,201*4,yy*4),fill=(255,80,64),width=1)
draw.rectangle((84,212,804,932),outline=(255,240,40),width=2)
proof.save(PROOF,optimize=True)

manifest = {
  "issue":53,"status":"DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED",
  "state":"DEFAITE; ATTAQUE; flotte ennemie volontairement revelee (G08 B/C)",
  "source":{"file":SRC.name,"dimensions":[320,240],"kind":"user supplied flattened PNG; not layer editable",
            "provenance":"ChatGPT uploaded raster 2026-11-05","sha256":sha(SRC)},
  "exports":[
    {"file":NATIVE.name,"dimensions":[320,240],"method":"native copy; no resampling","sha256":sha(NATIVE)},
    {"file":ZOOM.name,"dimensions":[1280,960],"method":"x4 strict NEAREST","sha256":sha(ZOOM)},
    {"file":PROOF.name,"dimensions":[1280,960],"method":"diagnostic only","sha256":sha(PROOF)}
  ],
  "checks_pc":{"grid_geometry":"PASS; independent cyan pixel scan","x_lines":x,"y_lines":y,
               "origin":[21,53],"bottom_right":[201,233],"cells":[10,10],"step":[18,18],
               "negative_shift_x1":"PASS: translated grid rejected",
               "nearest_exact":"PASS","five_ships":"VISUAL_ONLY",
               "initial_defeat_secret_fleet":"NOT_DELIVERED",
               "victory_draw_and_announcement":"NOT_DELIVERED",
               "last_two_shots_simultaneity":"NOT_PROVEN",
               "editable_scene_layers":"NOT_DELIVERED",
               "tft_touch_flash_memory":"NOT_TESTED","on_device":"UNVERIFIED"}
}
MANIFEST.write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("PASS native 320x240, 10x10 grid (21,53)-(201,233), cells 18x18; zoom x4 NEAREST exact")
