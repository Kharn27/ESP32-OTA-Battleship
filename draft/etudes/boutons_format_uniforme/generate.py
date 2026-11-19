#!/usr/bin/env python3
"""BTN-FMT-01 #163 — cinq plaques 140x35, étude non promue dans assets/.

Sources exactes du dépôt, aucune écriture en dehors de --out (draft/etudes/...).
Remodelage des bandes de métal libres du master HD, aucune mise
à l'échelle anisotrope, aucun recadrage du texte, des icônes et des rivets.

Exemples :
  python3 draft/etudes/boutons_format_uniforme/generate.py --repo .
  python3 draft/etudes/boutons_format_uniforme/generate.py --repo . --check
Requiert Pillow 12.3.0 (sorties PNG byte-identiques sous la même version).
"""
from __future__ import annotations

import argparse
from collections import OrderedDict
import hashlib
import io
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont, __version__ as pillow_version

DEST = "draft/etudes/boutons_format_uniforme"
SRC = OrderedDict({
    "jouer_HD": ("assets/boutons/bouton_jouer_B_original_1107x252_RGBA.png",
                  "17be0804509cc038e8a684f59ccb7c21473b5289", (1107, 252)),
    "palmares_HD": ("assets/boutons/bouton_palmares_B_original_1163x250_RGBA.png",
                     "e36a6a4c6872549a03e755674045634e1df4cbe4", (1163, 250)),
    "jouer_actuel": ("assets/boutons/bouton_jouer_B_CYD_140x32_RGBA.png",
                     "7980e48307992a79bc9c4a9cdb6697a0f1c1ab9f", (140, 32)),
    "palmares_actuel": ("assets/boutons/bouton_palmares_B_CYD_140x30_RGBA.png",
                        "35d66002043eb04d3dc1f4d3465629c302d87c65", (140, 30)),
    "attaque_on_HD": ("assets/boutons/attaquer_on_original_2000x500_RGBA.png",
                      "1d5a28e344360c626de2b4a3820b0fdb0432c27b", (2000, 500)),
    "attaque_off_HD": ("assets/boutons/attaquer_off_original_2000x500_RGBA.png",
                       "a24dfa0bcb46cdf768ffc342bf2e49a4410aba3f", (2000, 500)),
    "attaque_on": ("assets/boutons/attaquer_on_140x35_RGBA.png",
                   "7ac6667492a96e4a2ad6d7445ab68a504576324b", (140, 35)),
    "attaque_off": ("assets/boutons/attaquer_off_140x35_RGBA.png",
                    "4a9adb3b3e1e1822cd0598a9c273093a66373983", (140, 35)),
    "abandon_PO": ("draft/Bouton_Abandon.png",
                   "3818cb092456e1ff222d8b2dacf9f9aa55266135", (2172, 724)),
})
CROP_ABANDON = (24, 95, 2148, 626)  # crop validé en QA de PR Draft #150
CANVAS = (140, 35)
NAVY=(13,29,41,255)
PALE=(226,229,220,255)


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def git_blob(data: bytes) -> str:
    return hashlib.sha1(b"blob "+str(len(data)).encode()+b"\0"+data).hexdigest()


def png(img: Image.Image) -> bytes:
    bio=io.BytesIO()
    img.save(bio,"PNG",optimize=True,compress_level=9)
    return bio.getvalue()


def font(px=10):
    for path in ("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
                 "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf"):
        if Path(path).is_file():
            return ImageFont.truetype(path,px)
    return ImageFont.load_default()


def nearest4(im: Image.Image):
    return im.resize((im.width*4,im.height*4),Image.Resampling.NEAREST)


def premul_resize(im: Image.Image, size):
    if im.width*size[1] != im.height*size[0]:
        raise ValueError(f"Refus de déformation : {im.size} vers {size}")
    return im.convert("RGBa").resize(size,Image.Resampling.LANCZOS).convert("RGBA")


def rebuild_hd(img: Image.Image, top_range: tuple[int,int],
               bottom_range: tuple[int,int], add_top: int, add_bottom: int):
    """Reconstruct blank metal bands; leave all foreground/ornaments untouched.

    Original plate has naturally different proportions. Only TWO text-free,
    wheel-free, cup-free and rivet-free HD strips of plate metal are widened.
    Every other source pixel is copied *verbatim* (with integer vertical move).
    The 1 px transparent horizontal pad gives exact 4:1 HD source geometry:
    JOUER 1108x277 and PALMARES 1164x291. Final 140x35 is isotropic.
    No button-wide anisotropic resize, letter/icon stretch, or crop.
    """
    ta,tb=top_range;ba,bb=bottom_range
    if not (0<ta<tb<ba<bb<img.height):
        raise ValueError("Incorrect bounds for ornamental-free metal bands")
    if (img.width+1) != (img.height+add_top+add_bottom)*4:
        raise ValueError("HD master must be exactly 4:1")
    dest=Image.new("RGBA",(img.width+1,img.height+add_top+add_bottom))
    segments=((0,ta,0),(ta,tb,add_top),(tb,ba,0),
              (ba,bb,add_bottom),(bb,img.height,0))
    offset=0
    for y0,y1,extra in segments:
        slab=img.crop((0,y0,img.width,y1))
        if extra:
            # RGBa interpolation prevents transparent-edge color halos.
            # These are only the unornamented metal and bevel strips.
            slab=(slab.convert("RGBa")
                  .resize((img.width,y1-y0+extra),Image.Resampling.BICUBIC)
                  .convert("RGBA"))
        dest.paste(slab,(0,offset))
        offset += y1-y0+extra
    assert offset==dest.height
    return dest



def extend_caps(master: Image.Image, left1: int, left2: int,
                right1: int, right2: int, px: int = 24) -> Image.Image:
    """Reposition plate end-caps, widen metal gutters, never stretch icon/text/rivets.

    Source's transparent lateral frame allows exactly 24 HD pixels outward at
    each side. Keep center glyph region pixel-identical, translate intact left
    and right end-caps and interpolate only empty metal / engraved bevel.
    This corrects the *visible* horizontal footprint, not merely PNG canvas.
    """
    w, h = master.size
    if not (2*px < left1 < left2 < right1 < right2 < w-2*px):
        raise ValueError("Unsafe cap positions")
    out = Image.new("RGBA",(w,h))
    pieces = ((0,left1,-px,0),
              (left1,left2,left1-px,px),
              (left2,right1,left2,0),
              (right1,right2,right1,px),
              (right2,w,right2+px,0))
    for x0,x1,dest_x,add in pieces:
        part=master.crop((x0,0,x1,h))
        if add:
            part=(part.convert("RGBa")
                  .resize((x1-x0+add,h),Image.Resampling.BICUBIC)
                  .convert("RGBA"))
        out.paste(part,(dest_x,0))
    return out

def inspect(im:Image.Image):
    a=np.asarray(im.getchannel("A"),dtype=np.uint8)
    def bbox_above(v):
        y,x=np.where(a>v)
        return ([int(x.min()),int(y.min()),int(x.max()+1),int(y.max()+1)] if len(x) else None)
    return {"dimensions":list(im.size),"mode":im.mode,
            "alpha_zero":int(np.count_nonzero(a==0)),
            "alpha_partial":int(np.count_nonzero((a>0)&(a<255))),
            "alpha_opaque":int(np.count_nonzero(a==255)),
            "alpha_gt8_bbox":bbox_above(8),
            "alpha_gt64_bbox":bbox_above(64)}


def preview(old:list[Image.Image], new:list[Image.Image], background:str):
    light=(background=="clair")
    base=PALE if light else NAVY
    ink=(38,52,57,255) if light else (240,227,195,255)
    accent=(120,101,65,255) if light else (126,161,172,255)
    im=Image.new("RGBA",(320,240),base)
    d=ImageDraw.Draw(im)
    d.text((10,6),"EXISTANT",font=font(10),fill=ink)
    d.text((170,6),"FAMILLE F",font=font(10),fill=ink)
    d.line((159,21,159,227),fill=accent,width=1)
    for i,(a,b) in enumerate(zip(old,new)):
        y=23+i*40
        im.alpha_composite(a,(10,y+(35-a.height)//2))
        im.alpha_composite(b,(170,y))
        if i<4:d.line((10,y+38,310,y+38),fill=accent,width=1)
    d.text((10,226),"AVANT",font=font(8),fill=ink)
    d.text((170,226),"140x35 / CANDIDAT",font=font(8),fill=ink)
    return im


def new_two_backgrounds(new:list[Image.Image]):
    im=Image.new("RGBA",(320,240),NAVY)
    d=ImageDraw.Draw(im)
    d.rectangle((160,0,319,239),fill=PALE)
    d.line((159,0,159,239),fill=(128,146,144,255))
    d.text((10,6),"FOND SOMBRE",font=font(10),fill=(240,227,195,255))
    d.text((170,6),"FOND CLAIR",font=font(10),fill=(38,52,57,255))
    for i,im1 in enumerate(new):
        y=23+i*40
        im.alpha_composite(im1,(10,y));im.alpha_composite(im1,(170,y))
    d.text((10,226),"5 VISUELS / 4:1",font=font(8),fill=(240,227,195,255))
    d.text((170,226),"PO A VALIDER",font=font(8),fill=(38,52,57,255))
    return im


def fit_context(new:list[Image.Image]):
    """320x240 real-size touch-layout proof. NOT an actual game screen."""
    im=Image.new("RGBA",(320,240),NAVY)
    d=ImageDraw.Draw(im)
    for y in range(26,225):
        d.line((0,y,319,y),fill=(14+(y//19)%3,37+(y//20)%7,53+(y//19)%8,255))
    d.rectangle((0,0,319,26),fill=(8,20,31,255))
    d.line((0,26,319,26),fill=(161,128,71,255))
    d.text((10,8),"CYD 320x240 / ENCOMBREMENT",font=font(10),fill=(237,212,164))
    # 10px lateral margins, 20px horizontal gap, 21px vertical gap;
    # only a gabarit: not a layout for the actual accueil/combat.
    spots=[(10,48),(170,48),(10,111),(170,111),(90,174)]
    for piece,xy in zip(new,spots):
        im.alpha_composite(piece,xy)
    d.text((10,224),"GABARIT, PAS UN ECRAN DE JEU",font=font(8),fill=(237,212,164))
    return im


def build(repo:Path):
    raw={};original={};sources={}
    for k,(path,expected,dim) in SRC.items():
        data=(repo/path).read_bytes()
        actual=git_blob(data)
        if actual != expected:raise RuntimeError(f"SOURCE_MODIFIEE {path} expected={expected} got={actual}")
        im=Image.open(io.BytesIO(data));im.load()
        if im.mode!="RGBA" or im.size!=dim:
            raise RuntimeError(f"FORMAT_SOURCE_INATTENDU {path}: {im.mode} {im.size}")
        raw[k]=data;original[k]=im
        sources[path]={"git_blob_sha1":actual,"sha256":sha(data),"bytes":len(data),"dimensions":list(dim)}

    j_hd=extend_caps(rebuild_hd(original["jouer_HD"],top_range=(12,32),
                    bottom_range=(216,239),add_top=13,add_bottom=12),
                    299,385,884,970)
    p_hd=extend_caps(rebuild_hd(original["palmares_HD"],top_range=(12,32),
                    bottom_range=(213,238),add_top=21,add_bottom=20),
                    290,342,1018,1062)
    j=premul_resize(j_hd,CANVAS)
    p=premul_resize(p_hd,CANVAS)
    for name,sprite in (("jouer",j),("palmares",p)):
        box=inspect(sprite)["alpha_gt64_bbox"]
        if box != [1,1,139,34]:
            raise RuntimeError(f"NON_UNIFORM_VISIBLE_PLATE {name} : {box}")
    # Exact PO source; same safe 4:1 composition proven in #149.
    abd_src=original["abandon_PO"]
    abdbox=abd_src.getchannel("A").point(lambda a:255 if a>1 else 0).getbbox()
    if abdbox!=(31,98,2141,623):
        raise RuntimeError(f"ABANDON alpha bbox changed: {abdbox}")
    abd_hd=abd_src.crop(CROP_ABANDON)
    abd=premul_resize(abd_hd,CANVAS)
    a_on=original["attaque_on"].copy()
    a_off=original["attaque_off"].copy()
    old=[original["jouer_actuel"],original["palmares_actuel"],a_on,a_off,abd]
    new=[j,p,a_on,a_off,abd]
    paths=OrderedDict({
        "maitres/jouer_f_recompose_HD_1108x277_RGBA.png":j_hd,
        "maitres/palmares_f_recompose_HD_1164x291_RGBA.png":p_hd,
        "exports/jouer_f_140x35_RGBA.png":j,
        "exports/palmares_f_140x35_RGBA.png":p,
        "exports/attaquer_on_f_140x35_RGBA.png":a_on,
        "exports/attaquer_off_f_140x35_RGBA.png":a_off,
        "exports/abandon_f_140x35_RGBA.png":abd,
    })
    for name,comp in [("comparatif_sombre",preview(old,new,"sombre")),
                      ("comparatif_clair",preview(old,new,"clair")),
                      ("famille_f_sombre_clair",new_two_backgrounds(new)),
                      ("gabarit_contexte",fit_context(new))]:
        paths[f"preuves/{name}_320x240.png"]=comp
        paths[f"preuves/{name}_1280x960_NEAREST.png"]=nearest4(comp)
    # Per-candidate zoom to verify text/icon pixels with no smoothing.
    for n,img in zip(("jouer","palmares","attaquer_on","attaquer_off","abandon"),new):
        paths[f"preuves/{n}_f_560x140_NEAREST.png"]=nearest4(img)
    outbytes={k:png(v) for k,v in paths.items()}
    # Approved ATTAQUER exports copied *octet for octet*, not just pixel-identical.
    # This preserves the already-approved file hash and original PNG packing.
    outbytes["exports/attaquer_on_f_140x35_RGBA.png"]=raw["attaque_on"]
    outbytes["exports/attaquer_off_f_140x35_RGBA.png"]=raw["attaque_off"]
    report={
        "mission":"BTN-FMT-01 #163",
        "status":"CANDIDATE / ART_NOT_REVIEWED / ON_DEVICE_UNVERIFIED",
        "standard":"140x35 RGBA transparent, ratio 4:1, five visual states",
        "git_original_abandon_sha256":"1b7c026a5ebcf7283de9ce93bf740fab350aa4aaf22da80641c34fe421c8719f",
        "filiation":{
            "jouer":"JOUER B approved HD source immutable; stretch 2 bare-metal horizontal strips +13/+12 rows; move caps 24 HD pixels outward and stretch 2 empty vertical metal gutters; icon, text and rivets moved intact with no letter stretching; iso 140x35",
            "palmares":"PALMARES B approved HD immutable; stretch bare-metal horizontal strips +21/+20 rows; move side caps outward by 24 HD pixels and stretch only blank metal gutters; preserve trophy, text including accent and rivets without scaling; iso 140x35",
            "attaquer_on_off":"Exact byte copies of approved 140x35 sprites, including PNG packing; 2000x500 approved HD masters checked as provenance",
            "abandon":"Exact Git PO original 2172x724; safe crop [24,95,2148,626] -> 2124x531 (4:1), premult LANCZOS 140x35; PR #150 is a reference only",
        },
        "sources":sources,
        "artistic_status":{"jouer_palmares_B":"VALIDATED_PO_PC only for historical 140x32 / 140x30 versions; format-F redraw requires new GO",
                           "attaquer":"VALIDATED_PO_PC source / reused unchanged; family F still subject to PO review",
                           "abandon":"ART_NOT_REVIEWED"},
        "font":"DejaVuSans-Bold or LiberationSans-Bold for proof captions only, never inside sprites",
        "pillow_version":pillow_version,
        "numpy_version":np.__version__,
        "files":{k:{"bytes":len(outbytes[k]),"sha256":sha(outbytes[k]),**inspect(paths[k])} for k in paths},
    }
    repbytes=(json.dumps(report,ensure_ascii=False,indent=2,sort_keys=True)+"\n").encode("utf-8")
    return outbytes, repbytes, new, old


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--repo",type=Path,default=Path("."))
    p.add_argument("--out",type=Path,default=Path(DEST))
    p.add_argument("--check",action="store_true",help="Never write: verify PNG and SHA-256 report bytes")
    a=p.parse_args()
    repo=a.repo.resolve()
    dst=a.out if a.out.is_absolute() else repo/a.out
    outputs,report,new,old=build(repo)
    if a.check:
        for rel,data in outputs.items():
            path=dst/rel
            if not path.is_file() or path.read_bytes()!=data:
                raise SystemExit(f"CHECK FAIL: PNG {path}")
        f=dst/"rapport.json"
        if not f.is_file() or f.read_bytes()!=report:
            raise SystemExit("CHECK FAIL: rapport.json")
        print(f"PASS BTN-FMT-01: {len(outputs)} PNG and rapport.json byte-for-byte; read only")
    else:
        for rel,data in outputs.items():
            f=dst/rel;f.parent.mkdir(parents=True,exist_ok=True);f.write_bytes(data)
        (dst/"rapport.json").write_bytes(report)
        print(f"BUILT BTN-FMT-01: {len(outputs)} PNG and rapport.json in {dst}")

if __name__=="__main__":main()
