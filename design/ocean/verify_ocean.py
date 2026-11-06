#!/usr/bin/env python3
from PIL import Image, ImageDraw
from pathlib import Path
import hashlib, json
ROOT=Path(__file__).resolve().parent
full=Image.open(ROOT/'ocean_bg_full_320x240.png').convert('RGB')
zoom=Image.open(ROOT/'ocean_bg_full_x4.png').convert('RGB')
battle=Image.open(ROOT/'ocean_bg_battle_180x180.png').convert('RGB')
assert full.size==(320,240) and zoom.size==(1280,960) and battle.size==(180,180)
assert full.resize((1280,960),Image.Resampling.NEAREST).tobytes()==zoom.tobytes(), 'x4 interpolated or altered'
assert full.crop((21,53,201,233)).tobytes()==battle.tobytes(), 'battle crop mismatch'
proof=full.convert('RGBA'); overlay=Image.new('RGBA',(320,240));d=ImageDraw.Draw(overlay)
for i in range(11):
    x=21+18*i;y=53+18*i
    d.line((x,53,x,233),fill=(220,242,230,170),width=1)
    d.line((21,y,201,y),fill=(220,242,230,170),width=1)
d.rectangle((21,53,200,232),outline=(232,245,237,220))
d.ellipse((99,165,105,171),outline=(239,185,103,240),width=2)
d.ellipse((153,93,159,99),outline=(236,245,242,240),width=2)
for j in range(10):d.rectangle((208,61+j*15,313,70+j*15),fill=(10,32,43,180))
proof=Image.alpha_composite(proof,overlay)
pr=ROOT/'preuves';pr.mkdir(exist_ok=True)
proof.save(pr/'ocean_overlay_legibility_320x240.png')
proof.resize((1280,960),Image.Resampling.NEAREST).save(pr/'ocean_overlay_legibility_x4.png')
print('PASS dimensions, NEAREST, exact crop; overlay test images generated separately')
