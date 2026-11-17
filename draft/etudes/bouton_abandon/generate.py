#!/usr/bin/env python3
"""BTN-ABD-01 #149. Non-destructive reproducible graphic study.

Source in repo: draft/Bouton_Abandon.png, immutable Git blob 3818cb...; 2172x724
The old upload differs from the Git source; Git source takes precedence.
Requires Pillow. --check compares exact bytes, never writes.
"""
from __future__ import annotations
import argparse
import hashlib
import io
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont, __version__ as pillow_version

EXPECTED_GIT_SHA1 = "3818cb092456e1ff222d8b2dacf9f9aa55266135"
CROP_GIT = (24, 95, 2148, 626)
FORMATS = ((140, 35), (112, 28))
APPROVED = {
    "jouer": "assets/boutons/bouton_jouer_CYD_140x32_RGBA.png",
    "palmares": "assets/boutons/bouton_palmares_CYD_140x30_RGBA.png",
    "attaquer": "assets/boutons/attaquer_on_140x35_RGBA.png",
}
NAVY = (16, 31, 44, 255)
PALE = (214, 224, 220, 255)
INK = (239, 234, 207, 255)
DARK_INK = (27, 40, 49, 255)

def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def git_blob_sha1(data: bytes) -> str:
    return hashlib.sha1(b"blob " + str(len(data)).encode("ascii") + b"\0" + data).hexdigest()

def save_bytes(im: Image.Image) -> bytes:
    buf = io.BytesIO()
    im.save(buf, format="PNG", optimize=True, compress_level=9)
    return buf.getvalue()

def nearest4(im: Image.Image) -> Image.Image:
    return im.resize((im.width * 4, im.height * 4), Image.Resampling.NEAREST)

def crop_to_master(im: Image.Image) -> tuple[Image.Image, tuple[int,int,int,int], tuple[int,int,int,int]]:
    a = im.getchannel("A")
    mask = a.point(lambda x: 255 if x > 1 else 0)
    bbox = mask.getbbox()
    if bbox is None:
        raise ValueError("Empty image alpha >1")
    # Exact geometrical framing measured on the original immutable Git PNG:
    # removes empty transparent surrounds but does not alter the design.
    # Refuse unknown shapes rather than force a 4:1 stretch/crop.
    if im.size != (2172, 724):
        raise ValueError(f"Unknown source dimensions {im.size}; manual framing review required")
    if not (bbox[0] >= CROP_GIT[0] and bbox[1] >= CROP_GIT[1]
            and bbox[2] <= CROP_GIT[2] and bbox[3] <= CROP_GIT[3]):
        raise ValueError(f"Graphic protrudes beyond safe crop; alpha bbox: {bbox}")
    return im.crop(CROP_GIT), CROP_GIT, bbox

def scale_premultiplied(im: Image.Image, w: int, h: int) -> Image.Image:
    if im.width * h != im.height * w:
        raise ValueError("Refusing aspect-ratio distortion")
    # Premultiplied alpha avoids black halos on the transparent bevel edge.
    return im.convert("RGBa").resize((w,h), Image.Resampling.LANCZOS).convert("RGBA")

def get_font(size=10):
    for path in ("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
                 "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf"):
        if Path(path).is_file():
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()

def draw_label(dr, xy, s, color, size=10):
    dr.text(xy, s, fill=color, font=get_font(size))

def preview_single(large: Image.Image, small: Image.Image) -> Image.Image:
    im = Image.new("RGBA", (320,240), NAVY)
    dr = ImageDraw.Draw(im)
    dr.rectangle((0,0,319,118),fill=PALE)
    dr.rectangle((0,119,319,239),fill=NAVY)
    dr.line((0,119,319,119),fill=(130,146,144,255),width=1)
    draw_label(dr,(12,10),"ABANDON - ETUDE CYD",DARK_INK,11)
    draw_label(dr,(12,132),"FOND SOMBRE",INK,10)
    im.alpha_composite(large, (90,37))
    im.alpha_composite(small, (104,167))
    draw_label(dr,(12,84),"140 x 35 px",DARK_INK,10)
    draw_label(dr,(12,207),"112 x 28 px",INK,10)
    draw_label(dr,(12,225),"CANDIDAT - NON VALIDE PO",INK,8)
    return im

def preview_approved(large: Image.Image, repo: Path) -> Image.Image | None:
    # Render 4 authentic sprites using the same deterministic 5x7 bitmap
    # lettering and RGBA composition as the published GitHub comparison.
    if not all((repo / p).is_file() for p in APPROVED.values()):
        return None
    from check_preuves import APPROUVES, blob, planche
    imgs = []
    for path, expected_sha1, size in APPROUVES:
        raw = (repo / path).read_bytes()
        if blob(raw) != expected_sha1:
            raise ValueError(f"Approved sprite changed: {path}")
        with Image.open(io.BytesIO(raw)) as img:
            sprite = img.convert("RGBA")
        if sprite.size != size:
            raise ValueError(f"Invalid approved sprite size: {path}")
        imgs.append(sprite)
    imgs.append(large)
    return planche(imgs)

def alpha_report(im):
    a = im.getchannel("A")
    h = a.histogram()
    return {"size":list(im.size),"mode":im.mode,"alpha_zero":h[0],
            "alpha_partial":sum(h[1:255]),"alpha_opaque":h[255],
            "alpha_nonzero_bbox":list(a.getbbox()) if a.getbbox() else None,
            "alpha_gt1_bbox":list(a.point(lambda x:255 if x>1 else 0).getbbox() or ())}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--input",default="draft/Bouton_Abandon.png",type=Path)
    p.add_argument("--repo",default=".",type=Path)
    p.add_argument("--out",default="draft/etudes/bouton_abandon",type=Path)
    p.add_argument("--allow-reference",action="store_true",help="off-repo upload: no claim of exact Git blob lineage")
    p.add_argument("--check",action="store_true",help="read-only: verify all expected generated PNG and report bytes")
    p.add_argument("--check-source", action="store_true", help="audit source Git et comparaison de pixels RGBA avec l'upload PO")
    p.add_argument("--check-comparatif", action="store_true", help="validation des vrais sprites approuves et du comparatif natif + zoom")
    a=p.parse_args()
    if (a.check or a.check_source or a.check_comparatif) and not a.allow_reference:
        from check_preuves import verify_source, verify_comparatif, verify_full
        repo = a.repo.resolve()
        outdir = a.out if a.out.is_absolute() else repo / a.out
        source = a.input if a.input.is_absolute() else repo / a.input
        if a.check_comparatif:
            verify_comparatif(repo, outdir)
        elif a.check_source:
            verify_source(source)
        else:
            verify_full(repo, outdir, source)
        return
    a.input = a.input if a.input.is_absolute() else a.repo / a.input
    a.out = a.out if a.out.is_absolute() else a.repo / a.out
    d=a.input.read_bytes(); g=git_blob_sha1(d)
    if not a.allow_reference and g!=EXPECTED_GIT_SHA1:
        p.error(f"Source not the exact original Git blob: {g}")
    if a.allow_reference and g==EXPECTED_GIT_SHA1:
        print("Reference is also exact Git source")
    src=Image.open(io.BytesIO(d)).convert("RGBA")
    master,crop,bbox=crop_to_master(src)
    large=scale_premultiplied(master,140,35)
    small=scale_premultiplied(master,112,28)
    files={
        "exports/abandon_master_2124x531_RGBA.png":master,
        "exports/abandon_140x35_RGBA.png":large,
        "exports/abandon_112x28_RGBA.png":small,
        "preuves/abandon_140x35_x4_NEAREST.png":nearest4(large),
        "preuves/abandon_112x28_x4_NEAREST.png":nearest4(small),
        "preuves/etude_abandon_320x240.png":preview_single(large,small),
        "preuves/etude_abandon_1280x960_NEAREST.png":nearest4(preview_single(large,small)),
    }
    approved=preview_approved(large,a.repo)
    if approved:
        files["preuves/comparatif_approuves_320x240.png"]=approved
        files["preuves/comparatif_approuves_1280x960_NEAREST.png"]=nearest4(approved)
    out={"mission":"BTN-ABD-01 #149","status":"CANDIDATE","on_device":"ON_DEVICE_UNVERIFIED",
         "origin_type":"exact_original_git_blob" if g==EXPECTED_GIT_SHA1 else "user_upload_visual_reference_NOT_exact_git_blob",
         "source":{"path":"draft/Bouton_Abandon.png" if g==EXPECTED_GIT_SHA1 else str(a.input),"bytes":len(d),"git_blob_sha1":g,"sha256":sha256(d),
                   "metadata":alpha_report(src)},
         "master":{"crop_xyxy":list(crop),"source_bbox_alpha_gt1":list(bbox),"metadata":alpha_report(master)},
         "technique":"RGBA, no imposed runtime format; premultiplied LANCZOS downsizing; NEAREST x4 proof",
         "pillow_version":pillow_version,
         "approved_comparison_generated":bool(approved),"files":{}}
    for rel,im in files.items():
        b=save_bytes(im)
        out["files"][rel]={"bytes":len(b),"sha256":sha256(b),"metadata":alpha_report(im)}
        path=a.out/rel
        if a.check:
            if not path.is_file() or path.read_bytes()!=b:
                raise SystemExit(f"CHECK FAIL: {path}")
        else:
            path.parent.mkdir(parents=True,exist_ok=True)
            path.write_bytes(b)
    report=json.dumps(out,indent=2,ensure_ascii=False,sort_keys=True)+"\n"
    rp=a.out/"rapport_git_original.json"
    if a.check:
        if not rp.is_file() or rp.read_text(encoding="utf-8")!=report:
            raise SystemExit(f"CHECK FAIL: {rp}")
        print("CHECK PASS: "+str(len(files))+" PNG + rapport_git_original.json; NO FILES WRITTEN")
    else:
        rp.write_text(report,encoding="utf-8")
        print("BUILD: "+str(len(files))+" PNG + rapport_git_original.json")
    print("source git blob",g,"pixel source",src.size,"alpha>1 bbox",bbox,
          "approved comps",bool(approved))
if __name__=="__main__":main()