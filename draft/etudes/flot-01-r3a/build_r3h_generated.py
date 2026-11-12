#!/usr/bin/env python3
"""R3-H generated art: isolated 54x18 hull + independent 66x22 foam.

CANDIDATE / DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED.
Requires Pillow 12.3.0. Does not overwrite the earlier R3-A candidates.
"""
from __future__ import annotations
import argparse
import hashlib
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "r3h_source_generated_2172x724_RGBA.png"
BOX = (320, 45, 2020, 665)
ORIGIN = (10, 0)

def separate(source: Image.Image):
    """Hand-tuned, repeatable segmentation of metal and foam in flattened art."""
    source = source.convert("RGBA")
    if source.size != (2172, 724):
        raise ValueError("Wrong source dimensions")
    original = source.load()
    hull = Image.new("RGBA", source.size)
    wake = Image.new("RGBA", source.size)
    hp, wp = hull.load(), wake.load()
    for y in range(35, 688):
        for x in range(110, 2120):
            r,g,b,a = original[x,y]
            if a < 42:
                continue
            metallic = False
            if y < 580:
                metallic = True
            elif 405 <= x < 550 and y <= 656:
                metallic = True
            elif 530 <= x <= 1920 and y < 633:
                metallic = y <= 605 or (b-r < 20 and max(r,g,b) < 110)
            elif 1920 < x < 2010 and y < 602:
                metallic = True
            if metallic:
                hp[x,y] = (r,g,b,a)
            elif y >= 570:
                wp[x,y] = (r,g,b,a)
    return hull,wake

def hard_alpha(image):
    rgb = image.convert("RGB")
    a = image.getchannel("A").point(lambda n: 255 if n >= 122 else 0)
    rgb.putalpha(a)
    return rgb

def export_layers():
    source = Image.open(SOURCE).convert("RGBA")
    hull,wake = separate(source)
    ship = hard_alpha(hull.crop(BOX).resize((54,18),Image.Resampling.LANCZOS))
    scalex = (BOX[2]-BOX[0])/54
    scaley = (BOX[3]-BOX[1])/18
    sx = BOX[0]-ORIGIN[0]*scalex
    sy = BOX[1]
    b = (round(sx),round(sy),round(sx+66*scalex),round(sy+22*scaley))
    foam = hard_alpha(wake.crop(b).resize((66,22),Image.Resampling.LANCZOS))
    for image,size in ((ship,(54,18)),(foam,(66,22))):
        if image.size != size or image.mode != "RGBA":
            raise ValueError("Wrong layer shape")
        if set(image.getchannel("A").get_flattened_data()) - {0,255}:
            raise ValueError("Nonbinary alpha")
    return ship,foam

def sea(light):
    image = Image.new("RGBA",(320,240))
    px = image.load()
    for y in range(240):
        for x in range(320):
            waves = ((x*23+y*17+(x*y)//9)%37-19)
            if light:
                px[x,y] = (min(255,43+waves//3),min(255,137+waves),
                           min(255,156+waves),255)
            else:
                px[x,y] = (3+max(0,waves//8),38+waves//4,58+waves//3,255)
    d=ImageDraw.Draw(image,"RGBA")
    for i in range(18):
        y=20+i*13
        d.line((0,y,320,y),fill=(110,179,176,35 if light else 16),width=1)
    return image

def frame(ship,wake,light):
    image=sea(light)
    d=ImageDraw.Draw(image,"RGBA")
    gx,gy,pitch=21,53,18
    d.rectangle((gx,gy,gx+180,gy+180),fill=(0,25,34,18))
    for i in range(11):
        x=gx+pitch*i
        y=gy+pitch*i
        d.line((x,gy,x,gy+180),fill=(203,234,218,65),width=1)
        d.line((gx,y,gx+180,y),fill=(203,234,218,65),width=1)
    pos=(57,143)
    image.alpha_composite(wake,(pos[0]-ORIGIN[0],pos[1]))
    image.alpha_composite(ship,pos)
    d=ImageDraw.Draw(image)
    d.text((8,10),"FLOT-01 R3-H  |  CANDIDAT",fill="#eacda2")
    d.text((8,24),"REVUE PO / PILOTAGE",fill="#afcdca")
    d.text((210,222),"PC NON CYD",fill="#eacda2")
    return image.convert("RGBA")

def detail(ship,wake,light):
    image=sea(light).crop((47,143,47+66,143+22))
    image.alpha_composite(wake)
    image.alpha_composite(ship,ORIGIN)
    return image.convert("RGBA").resize((264,88),Image.Resampling.NEAREST)

def generate():
    ship,wake=export_layers()
    dark=frame(ship,wake,False)
    light=frame(ship,wake,True)
    outputs={
       "r3h_generated_intact_h_54x18.png": ship,
       "r3h_generated_intact_h_x4.png": ship.resize((216,72),Image.Resampling.NEAREST),
       "r3h_generated_sillage_h_66x22.png": wake,
       "r3h_generated_sillage_h_x4.png": wake.resize((264,88),Image.Resampling.NEAREST),
       "r3h_generated_mer_sombre_320x240.png": dark,
       "r3h_generated_mer_claire_320x240.png": light,
       "r3h_generated_mer_sombre_x4_1280x960.png": dark.resize((1280,960),Image.Resampling.NEAREST),
       "r3h_generated_mer_claire_x4_1280x960.png": light.resize((1280,960),Image.Resampling.NEAREST),
       "r3h_generated_detail_sombre_x4.png": detail(ship,wake,False),
       "r3h_generated_detail_claire_x4.png": detail(ship,wake,True),
    }
    return outputs

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--check",action="store_true")
    args=parser.parse_args()
    for name,image in generate().items():
        target=ROOT/name
        if args.check:
            if not target.is_file():
                raise SystemExit("MISSING: "+name)
            found=Image.open(target)
            if found.size!=image.size or found.convert("RGBA").tobytes()!=image.convert("RGBA").tobytes():
                raise SystemExit("PIXEL_MISMATCH: "+name)
        else:
            image.save(target,"PNG",optimize=True)
        print(name,image.size,image.mode,target.stat().st_size,
              hashlib.sha256(target.read_bytes()).hexdigest())
    print("R3H_GENERATED_OK: pixel checks, RGBA alpha, 1x and nearest 4x; ON_DEVICE_UNVERIFIED")

if __name__=="__main__":
    main()
