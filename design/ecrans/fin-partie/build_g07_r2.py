#!/usr/bin/env python3
"""G07-R2 PC design preview from a flattened PNG, NOT firmware assets."""
import hashlib, json
from pathlib import Path
from PIL import Image, ImageChops
D=Path(__file__).resolve().parent
src=D/"g07_r2_source_1448x1086.png"
native=D/"g07_r2_defaite_320x240.png"
zoom=D/"g07_r2_defaite_x4.png"
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
with Image.open(src) as image:
    assert image.size==(1448,1086), f"Unexpected source: {image.size}"
    mode=image.mode
    thumb=image.convert("RGB").resize((320,240),Image.Resampling.LANCZOS)
thumb.save(native,optimize=True)
thumb.resize((1280,960),Image.Resampling.NEAREST).save(zoom,optimize=True)
with Image.open(native) as n, Image.open(zoom) as z:
    assert n.size==(320,240) and z.size==(1280,960)
    assert ImageChops.difference(n.resize(z.size,Image.Resampling.NEAREST),z).getbbox() is None
manifest={
 "issue":53,"status":"DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED",
 "source":{"file":src.name,"dimensions":[1448,1086],"mode":mode,"sha256":sha(src),
           "kind":"flattened user-supplied PNG, not editable scene layers",
           "provenance":"Graphiste illustration forwarded in chat, 2026-11-04"},
 "exports":[
   {"file":native.name,"dimensions":[320,240],"resampling":"LANCZOS","sha256":sha(native)},
   {"file":zoom.name,"dimensions":[1280,960],"resampling":"NEAREST","sha256":sha(zoom)}],
 "checks_pc":{"native_size":"PASS","x4_size":"PASS","strict_nearest_zoom":"PASS",
              "grid_10x10":"VISUAL_ONLY",
              "grid_geometry_21_53_18px":"NON_CONFORMANT (image only)",
              "simultaneous_last_shots":"NOT_PROVEN",
              "victoire_nul_initial_compact":"NOT_DELIVERED",
              "on_device":"UNVERIFIED"}}
(D/"g07_r2_provenance.json").write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("PASS: 320x240 PNG + 1280x960 strict NEAREST; design still incomplete")
