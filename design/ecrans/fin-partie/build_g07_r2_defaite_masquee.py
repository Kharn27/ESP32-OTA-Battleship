#!/usr/bin/env python3
"""G07-R2 #53: edited initial DEFAITE state, no enemy fleet disclosure.
Deterministic 320x240 technical-art build; preview only, not firmware.
Requires Pillow and numpy. Original user raster and previous G07 frame supplied.
"""
from pathlib import Path
from PIL import Image,ImageDraw,ImageChops
import hashlib,io,json,zipfile,numpy as np
from xml.etree.ElementTree import Element,SubElement,tostring
from g07_r2_defaite_history import load_and_validate,marks_layers,verify_history_pixels

D=Path(__file__).resolve().parent
src=D/"g07_r2_defaite_masquee_original_1448x1086.png"
base=D/"g07_r2_source_320x240.png"  # immutable original, not regenerated revealed export
out=D/"g07_r2_defaite_masquee_320x240.png"
zoom=D/"g07_r2_defaite_masquee_x4.png"
proof=D/"g07_r2_defaite_masquee_proof_320x240.png"
proof4=D/"g07_r2_defaite_masquee_proof_x4.png"
ora=D/"g07_r2_defaite_masquee_source.ora"
manifest=D/"g07_r2_defaite_masquee_provenance.json"
sx=[142,225,309,393,478,560,643,727,806,887,965]
sy=[234,304,372,440,506,574,642,711,780,848,916]
xs=list(range(21,202,18))
ys=list(range(53,234,18))
CYAN=(156,214,220)

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def png(im):
    b=io.BytesIO()
    im.save(b,format="PNG",optimize=True)
    return b.getvalue()
def check_grid(im):
    a=np.asarray(im.convert("RGB"))
    assert a.shape==(240,320,3)
    mask=np.all(a==np.asarray(CYAN),axis=2)
    X=[i for i in range(17,207) if int(mask[55:232,i].sum())>=172]
    Y=[i for i in range(48,234) if int(mask[i,22:200].sum())>=172]
    assert X==xs and Y==ys,(X,Y)

assert sha(src)=="bc8b51d0b9e579f305f4e8fabda834c3c4e19474685e2b907b7db98eedf317f0"
assert sha(base)=="81bd1bcfc6e04316a9b34c90a97727f3598ec7ebf1951a0444043e17f3a5da8d"
scenario=load_and_validate()
original=Image.open(src).convert("RGB")
background=Image.open(base).convert("RGBA")
assert original.size==(1448,1086) and background.size==(320,240)
# Previous revealed hulls are removed even from the editable base layer.
background.paste((0,0,0,0),(21,53,202,234))
sea=Image.new("RGBA",(320,240))
for row in range(10):
    for col in range(10):
        cell=original.crop((sx[col]+7,sy[row]+7,sx[col+1]-7,sy[row+1]-7))
        sea.paste(cell.resize((17,17),Image.Resampling.LANCZOS),(22+18*col,54+18*row))
grid=Image.new("RGBA",(320,240))
d=ImageDraw.Draw(grid)
for x in xs:d.line((x,53,x,233),fill=(*CYAN,255),width=1)
for y in ys:d.line((21,y,201,y),fill=(*CYAN,255),width=1)
miss_marks,hit_marks=marks_layers(scenario)
panel=Image.new("RGBA",(320,240))
panel.paste(original.resize((320,240),Image.Resampling.LANCZOS).crop((218,44,319,209)),(218,44))
merged=background
for layer in (sea,miss_marks,hit_marks,grid,panel):
    merged=Image.alpha_composite(merged,layer)
verify_history_pixels(merged,scenario)
assert merged.getextrema()[3]==(255,255)
native=merged.convert("RGB")
native.save(out,optimize=True)
z=native.resize((1280,960),Image.Resampling.NEAREST)
z.save(zoom,optimize=True)
check_grid(Image.open(out))
wrong=Image.new("RGB",(320,240))
wrong.paste(native.crop((0,0,319,240)),(1,0))
try:check_grid(wrong)
except AssertionError:pass
else:raise AssertionError("negative 1px shift passed")
assert ImageChops.difference(native.resize((1280,960),Image.Resampling.NEAREST),Image.open(zoom).convert("RGB")).getbbox() is None
pr=native.copy()
p=ImageDraw.Draw(pr)
for x in xs:p.line((x,53,x,233),fill=(255,59,147))
for y in ys:p.line((21,y,201,y),fill=(255,59,147))
p.rectangle((21,53,201,233),outline=(255,228,0))
pr.save(proof,optimize=True)
pr.resize((1280,960),Image.Resampling.NEAREST).save(proof4,optimize=True)
root=Element("image",{"version":"0.0.3","w":"320","h":"240","name":"G07-R2 Defaite initiale - flotte masquee"})
st=SubElement(root,"stack")
layers=[
    ("Panneau VOIR SA FLOTTE",panel,"data/layer3.png"),
    ("Lignes cyan grille 18x18",grid,"data/layer2.png"),
    ("Impacts TOUCHÉ — cendres discrètes",hit_marks,"data/layer5.png"),
    ("Impacts RATÉ — rides marines",miss_marks,"data/layer4.png"),
    ("Mer historique tirs sans navires",sea,"data/layer1.png"),
    ("Decor G07 reutilise sans coques",background,"data/layer0.png"),
]
for name,im,path in layers:
    SubElement(st,"layer",{"name":name,"src":path,"x":"0","y":"0","opacity":"1.0","visibility":"visible","composite-op":"svg:src-over"})
with zipfile.ZipFile(ora,"w") as zip:
    zip.writestr("mimetype","image/openraster",compress_type=zipfile.ZIP_STORED)
    zip.writestr("stack.xml",tostring(root,encoding="utf-8",xml_declaration=True))
    zip.writestr("mergedimage.png",png(merged))
    zip.writestr("Thumbnails/thumbnail.png",png(merged))
    for _,im,path in layers:zip.writestr(path,png(im))
doc={
    "issue":53,"pr":74,"state":"DEFAITE initiale / ATTAQUE / Capitaine MASQUE",
    "status":"DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED, awaiting PO visual acceptance",
    "sources":[{"file":f.name,"sha256":sha(f),"size_bytes":f.stat().st_size} for f in (src,base)],
    "deliverables":[{"file":f.name,"sha256":sha(f),"size_bytes":f.stat().st_size} for f in (out,zoom,proof,proof4,ora)],
    "editable":"ORA with 6 layers (2 editable impact overlays); ornaments and text within their layers remain rasterized",
    "scenario_file":"g07_r2_defaite_scenario.json",
    "scenario_sha256":sha(D/"g07_r2_defaite_scenario.json"),
    "history":{"turns":23,"attack_hit":5,"attack_miss":18,"defense_hit":17,"defense_miss":6,"last_simultaneous_pair":scenario["last_simultaneous_pair"]},
    "history_pixels":"PASS: 12 native ripple marks and 5 subtle hit cores measured",
    "grid_x":xs,"grid_y":ys,"origin":[21,53],"end":[201,233],"cells":[10,10],"cell_px":[18,18],
    "pixel_measured_grid":"PASS","negative_shift_1px":"PASS","nearest_4x_exact":"PASS",
    "remaining":"No firmware asset validation; no physical CYD/stylus test; last two simultaneous shots documented in a legal scenario, animation not proven by static raster"
}
manifest.write_text(json.dumps(doc,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("PASS grid / 320x240 / x4-nearest: "+sha(out))
