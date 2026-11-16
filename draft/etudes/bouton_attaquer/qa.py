#!/usr/bin/env python3
"""BTN-ATK-01: checks PNG RGBA, Git blob identity and 320x240 / x4 PC previews.
Run in the complete Git checkout; requires Pillow. No file in draft/ PO originals is modified.
"""
from __future__ import annotations
import hashlib
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
PO = [
    ("Bouton_Attaquer_on.png", 1874024, "e7b663608fa56c452fb189fb3fd96ea03bd33aef"),
    ("Bouton_Attaquer_off.png", 1520779, "d425e792a713167f667f80de52ffe010c6281fba"),
]
SPRITES = {
    "attaquer_on_140x35_RGBA.png": ((140, 35), "c5cdd3fc788f0b2629cb3d2bac456bfd15e9fcccb8ff4a8979fb0286b82aee05"),
    "attaquer_off_140x35_RGBA.png": ((140, 35), "71b1a66c1380d4ab7e8035bc3cea6accf1ee6e77661c738ee4f25f0d20e18b94"),
    "attaquer_on_112x28_RGBA.png": ((112, 28), "68fcab7abe17906695f23ec07672098a357340918f5edd1533d0e9b8c6237c27"),
    "attaquer_off_112x28_RGBA.png": ((112, 28), "6b504b1673201a89af2f49e95a5874895c97825fef02813355aa1b6dd5e55faf"),
    "source_on_cutout_588x148.png": ((588, 148), "26e05999b4f1663f42713d31d76b214094b752ac5d17868e4f1d097bc38454d3"),
    "source_off_cutout_588x148.png": ((588, 148), "52dc7fb42aa207c6f7a1f0378fd7d113021d597a84c529d729d9818c862b6759"),
}
def sha256(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest()
def inspect():
    for name, expected_size, expected_blob in PO:
        p = ROOT / "draft" / name
        data = p.read_bytes()
        blob = hashlib.sha1(b"blob " + str(len(data)).encode() + b"\0" + data).hexdigest()
        assert len(data) == expected_size and blob == expected_blob, (name, len(data), blob)
        with Image.open(p) as im:
            rgba = im.convert("RGBA")
            alpha = rgba.getchannel("A")
            print("PO", name, "size", im.size, "mode", im.mode,
                  "sha256", sha256(data), "alpha_extrema", alpha.getextrema(),
                  "content_bbox", alpha.getbbox())
    for name, (dim, digest) in SPRITES.items():
        p = HERE / name
        data = p.read_bytes()
        assert sha256(data) == digest, (name, sha256(data))
        with Image.open(p) as im:
            assert im.format == "PNG" and im.mode == "RGBA" and im.size == dim, (name, im.mode, im.size)
            a = im.getchannel("A")
            assert a.getextrema() == (0, 255), (name, a.getextrema())
            assert all(im.getpixel(xy)[3] == 0 for xy in
                       [(0, 0), (im.width-1, 0), (0, im.height-1), (im.width-1, im.height-1)]), name
            print("CANDIDATE", name, im.size, "bytes", len(data), "RGBA", a.getextrema(), "sha256", digest)
def composite():
    cv = Image.new("RGBA", (320, 240), "#0c1b29")
    d = ImageDraw.Draw(cv)
    d.rectangle((0, 120, 319, 239), fill="#eee4d2")
    d.text((10, 9), "BTN-ATK-01 : DARK / 140px", fill="#ffe2aa")
    d.text((10, 129), "BTN-ATK-01 : LIGHT / 112px", fill="#3d2b23")
    for name, x, y in [
        ("attaquer_on_140x35_RGBA.png", 12, 45),
        ("attaquer_off_140x35_RGBA.png", 168, 45),
        ("attaquer_on_112x28_RGBA.png", 18, 170),
        ("attaquer_off_112x28_RGBA.png", 184, 170),
    ]:
        with Image.open(HERE/name) as im:
            cv.alpha_composite(im.convert("RGBA"), (x,y))
    d = ImageDraw.Draw(cv)
    for x, y, lab, color in [(13, 30, "ON", "#eeeeee"), (169,30,"OFF","#eeeeee"),
                            (19,153,"ON", "#35241c"), (185,153,"OFF","#35241c")]:
        d.text((x,y),lab,fill=color)
    n = HERE/"preuve_320x240.png"
    cv.convert("RGB").save(n, optimize=False)
    z = HERE/"preuve_x4_1280x960.png"
    cv.convert("RGB").resize((1280,960),Image.Resampling.NEAREST).save(z,optimize=False)
    for p in [n,z]:
        print("PREVIEW",p.name,Image.open(p).size,"sha256",sha256(p.read_bytes()))
if __name__ == "__main__":
    inspect()
    composite()
    print(json.dumps({"art_status":"CANDIDATE","device_status":"ON_DEVICE_UNVERIFIED","po_approval":None}))
