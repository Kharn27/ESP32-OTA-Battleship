#!/usr/bin/env python3
"""G07-R2 MATCH NUL — pixel-preserving sources, exports and PC controls."""
from pathlib import Path
from hashlib import sha256
import json, io, zipfile
import numpy as np
from PIL import Image, ImageChops
from xml.etree import ElementTree as ET

D=Path(__file__).resolve().parent
SOURCE=D/"image(20261105-160127).png"
PROOF_SOURCE=D/"image(20261105-160150).png"
N=D/"g07_r2_match_nul_320x240.png"
Z=D/"g07_r2_match_nul_x4.png"
P=D/"g07_r2_match_nul_geometry_proof_320x240.png"
PX4=D/"g07_r2_match_nul_geometry_proof_x4.png"
ORA=D/"g07_r2_match_nul_source.ora"
REPORT=D/"g07_r2_match_nul_provenance.json"
if not SOURCE.exists(): SOURCE=Z
if not PROOF_SOURCE.exists(): PROOF_SOURCE=P
def sha(p): return sha256(p.read_bytes()).hexdigest()
def detect(image):
    a=np.array(image.convert("RGB"))
    assert a.shape==(240,320,3), a.shape
    cyan=np.all(a==[182,227,237],axis=2)
    xs=[x for x in range(18,205) if cyan[54:233,x].sum()>=178]
    ys=[y for y in range(50,236) if cyan[y,22:201].sum()>=179]
    assert xs==list(range(21,202,18)),xs
    assert ys==list(range(53,234,18)),ys
    assert all(cyan[53:234,x].all() for x in xs)
    assert all(cyan[y,21:202].all() for y in ys)
    return xs,ys

with Image.open(SOURCE) as f: zoom=f.convert("RGB")
with Image.open(PROOF_SOURCE) as f: annotated=f.convert("RGB")
assert zoom.size==(1280,960) and annotated.size==(320,240)
native=zoom.resize((320,240),Image.Resampling.NEAREST)
assert ImageChops.difference(native.resize((1280,960),Image.Resampling.NEAREST),zoom).getbbox() is None
xs,ys=detect(native)
mask=np.any(np.array(native)!=np.array(annotated),axis=2)
colors=np.unique(np.array(annotated)[mask],axis=0).tolist()
assert colors==[[227,96,120],[255,255,20]],colors
assert mask.sum()==3925,int(mask.sum())
shift=Image.new("RGB",(320,240))
shift.paste(native.crop((0,0,319,240)),(1,0))
try: detect(shift)
except AssertionError: pass
else: raise AssertionError("Negative x+1 test was not rejected")

native.save(N,optimize=True)
zoom.save(Z,optimize=True)
annotated.save(P,optimize=True)
annotated.resize((1280,960),Image.Resampling.NEAREST).save(PX4,optimize=True)

# OpenRaster: faithful flattened original artwork, plus a hideable verification layer.
a=np.array(annotated.convert("RGBA")).copy()
base=np.array(native.convert("RGBA"))
a[np.all(a[:,:,:3]==base[:,:,:3],axis=2)]=[0,0,0,0]
over=Image.fromarray(a)
def png_bytes(img):
    b=io.BytesIO(); img.save(b,format="PNG"); return b.getvalue()
root=ET.Element("image",{"w":"320","h":"240","name":"G07-R2 MATCH NUL","version":"0.0.3"})
st=ET.SubElement(root,"stack")
ET.SubElement(st,"layer",{"name":"Repères géométriques (masqués)","src":"data/layer1.png","visibility":"hidden","opacity":"1.0","x":"0","y":"0","composite-op":"svg:src-over"})
ET.SubElement(st,"layer",{"name":"Illustration native (raster aplati)","src":"data/layer0.png","visibility":"visible","opacity":"1.0","x":"0","y":"0","composite-op":"svg:src-over"})
with zipfile.ZipFile(ORA,"w") as z:
    z.writestr("mimetype","image/openraster",compress_type=zipfile.ZIP_STORED)
    z.writestr("stack.xml",ET.tostring(root,encoding="utf-8",xml_declaration=True))
    z.writestr("data/layer0.png",png_bytes(native.convert("RGBA")))
    z.writestr("data/layer1.png",png_bytes(over))
    z.writestr("mergedimage.png",png_bytes(native.convert("RGBA")))
    z.writestr("Thumbnails/thumbnail.png",png_bytes(native.resize((160,120),Image.Resampling.NEAREST)))
assert ImageChops.difference(native.resize((1280,960),Image.Resampling.NEAREST),Image.open(Z).convert("RGB")).getbbox() is None
report={
 "issue":53,"pr":74,"screen":"G07-R2 MATCH NUL, verdict compact VS Capitaine",
 "status":"DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED; PO review pending",
 "geometry":{"origin":[21,53],"end":[201,233],"cells":[10,10],"cell_size":[18,18],
  "x_lines":xs,"y_lines":ys,"grid_line_rgb":[182,227,237],"all_line_pixels":"PASS"},
 "checks_pc":{"x4_nearest_pixel_exact":"PASS","one_pixel_shift_rejected":"PASS",
  "control_overlay_pixels":int(mask.sum()),"control_overlay_only_red_and_yellow":"PASS",
  "on_device":"NOT_TESTED","editable_separate_ui_elements":"NOT_DELIVERED"},
 "source":{"input_x4_sha256":"aaf5a765b3da31ec3727d7e9d207feb8a4cb7496c4054ef22c2b56609eea3976",
  "input_proof_sha256":"b8034a610ef31db6c349fccc55e8e292605f19c80785a819f399e8c9441928e7",
  "provenance":"User supplied PNGs on 2026-11-05; no artistic retouching",
  "rebuild_inputs":"Canonical x4 export and annotated native proof retained in repository"},
 "exports":[{"file":p.name,"sha256":sha(p),"size_bytes":p.stat().st_size}
             for p in [N,Z,P,PX4,ORA]]
}
REPORT.write_text(json.dumps(report,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("PASS G07-R2 MATCH NUL: 320x240; (21,53)→(201,233); 18x18; x4 exact; negative check PASS")
