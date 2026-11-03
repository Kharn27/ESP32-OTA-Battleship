#!/usr/bin/env python3
"""Independent PC receiver for art preview, not hardware homologation."""
from pathlib import Path
from PIL import Image
import hashlib, json, sys
ROOT=Path(__file__).resolve().parent
native=Image.open(ROOT/'g04_r2_attaque_320x240.png').convert('RGB')
zoom=Image.open(ROOT/'g04_r2_attaque_x4.png').convert('RGB')
report=json.loads((ROOT/'generation-report.json').read_text())
assert native.size==(320,240),native.size
assert zoom.size==(1280,960),zoom.size
assert report['grid']==dict(x=21,y=53,cell=18,width=180,height=180)
assert report['axis']=='columns A-J; rows 1-10'
assert report['target']=='G5'
hits=report['hits'];misses=report['misses'];target=report['target']
assert len(hits)==4 and len(misses)==7 and len(set(hits+misses+[target]))==12
assert report['last_reply']=='B7 RATÉ' and report['unread_severity']=='COULÉ'

def near(src,large):
    # byte-exact nearest: 4x4 pixels for every source texel
    return src.resize((1280,960),Image.Resampling.NEAREST).tobytes()==large.tobytes()
assert near(native,zoom), 'NEAREST×4 invalid'

def center(key):
    return (21+(ord(key[0])-65)*18+9,53+(int(key[1:])-1)*18+9)
for key in hits:
    x,y=center(key)
    box=native.crop((x-7,y-7,x+8,y+8))
    assert sum(r>155 and r>g*1.25 and r>b*1.25 for r,g,b in box.getdata())>=6,(key,'hit contrast')
for key in misses:
    x,y=center(key)
    box=native.crop((x-7,y-8,x+8,y+8))
    assert sum(r>150 and g>190 and b>180 for r,g,b in box.getdata())>=6,(key,'miss foam')
x,y=center(target)
box=native.crop((x-8,y-8,x+9,y+9))
assert sum(r>180 and g>150 and b<160 for r,g,b in box.getdata())>=5,'target gold contrast'
# negative proof: one modified red channel must fail the strict near-neighbor comparator
mut=zoom.copy();r,g,b=mut.getpixel((73,91));mut.putpixel((73,91),((r+1)%256,g,b))
assert not near(native,mut),'negative test incorrectly accepted a modified pixel'
for name,info in report['files'].items():
    p=ROOT/name
    assert p.is_file(),name
    assert hashlib.sha256(p.read_bytes()).hexdigest()==info['sha256'],name
    assert p.stat().st_size==info['bytes']
print('G04-R2 PC PASS — native 320x240 / x4 1280x960 exact NEAREST')
print('grid x=21 y=53 size=180x180 cell=18 — 100 cells')
print('hit centers:',','.join(hits))
print('miss centers:',','.join(misses))
print('selected G5; last riposte B7 RATÉ; unread COULÉ (distinct)')
print('negative one-channel corruption: rejected')
print('HD art + editable scene generator + bitmap exports SHA256: MATCH')
print('ON_DEVICE_UNVERIFIED — artistic PO approval still pending')
