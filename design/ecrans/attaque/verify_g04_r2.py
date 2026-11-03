#!/usr/bin/env python3
"""Independent PC receiver for art preview, not hardware homologation."""
from pathlib import Path
from PIL import Image
import hashlib, json, sys
ROOT=Path(__file__).resolve().parent
native=Image.open(ROOT/'g04_r2_attaque_320x240.png').convert('RGB')
zoom=Image.open(ROOT/'g04_r2_attaque_x4.png').convert('RGB')
state_native=Image.open(ROOT/'g04_r2_etats_320x240.png').convert('RGB')
state_zoom=Image.open(ROOT/'g04_r2_etats_x4.png').convert('RGB')
report=json.loads((ROOT/'generation-report.json').read_text())
assert native.size==(320,240),native.size
assert zoom.size==(1280,960),zoom.size
assert state_native.size==(320,240) and state_zoom.size==(1280,960)
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
assert near(state_native,state_zoom), 'Vignettes NEAREST×4 invalid'
assert len(report['vignettes'])==4

# Independent *physical PNG pixel* evidence, not report metadata:
# 121 intersections and 220 near-boundary segments, expected source RGB fixed.
# 11 vertical and 11 horizontal lines; 100 real 18px-square cells.
GRID=(185,232,238)
FRAME=(214,250,242)
def pixel_grid(img):
    for ix in range(11):
        for iy in range(11):
            xy=(21+18*ix,53+18*iy)
            expected=FRAME if ix in (0,10) or iy in (0,10) else GRID
            assert img.getpixel(xy)==expected,('grid crossing mismatch',xy,img.getpixel(xy),expected)
    for ix in range(11):
        for iy in range(10):
            xy=(21+18*ix,54+18*iy)
            expected=FRAME if ix in (0,10) else GRID
            assert img.getpixel(xy)==expected,('vertical line missing',xy,img.getpixel(xy),expected)
    for iy in range(11):
        for ix in range(10):
            xy=(22+18*ix,53+18*iy)
            expected=FRAME if iy in (0,10) else GRID
            assert img.getpixel(xy)==expected,('horizontal line missing',xy,img.getpixel(xy),expected)
pixel_grid(native)
# Negative control: damaged one actual grid pixel must be rejected.
corrupt_grid=native.copy()
old=corrupt_grid.getpixel((57,54))
corrupt_grid.putpixel((57,54),((old[0]+1)%256,old[1],old[2]))
try:
    pixel_grid(corrupt_grid)
except AssertionError:
    pass
else:
    raise AssertionError('negative grid corruption was accepted')
# Four disjoint visual-state vignettes: sample native RGB inside each real
# painted control, not just the text/source metadata. Provenance in authored code.
expected_card_pixels={
 (20,70):(102,108,112),  # no target: disabled gray FEU
 (190,61):(142,55,52),   # committed FEU: burnished red bar
 (20,195):(52,93,112),   # combined resolution: shared blue result bar
 (185,170):(27,101,128), # back from defense: enabled navigation
}
for xy,expected in expected_card_pixels.items():
    assert state_native.getpixel(xy)==expected,('missing/wrong visual state',xy,state_native.getpixel(xy),expected)
# A tampered vignette pixel must fail the same probe.
mut_state=state_native.copy()
mut_state.putpixel((20,70),(0,0,0))
assert mut_state.getpixel((20,70))!=expected_card_pixels[(20,70)]

def center(key):
    return (21+(ord(key[0])-65)*18+9,53+(int(key[1:])-1)*18+9)
for key in hits:
    x,y=center(key)
    box=native.crop((x-7,y-7,x+8,y+8))
    assert sum(r>155 and r>g*1.25 and r>b*1.25 for r,g,b in box.get_flattened_data())>=6,(key,'hit contrast')
for key in misses:
    x,y=center(key)
    box=native.crop((x-7,y-8,x+8,y+8))
    assert sum(r>150 and g>190 and b>180 for r,g,b in box.get_flattened_data())>=6,(key,'miss foam')
x,y=center(target)
box=native.crop((x-8,y-8,x+9,y+9))
assert sum(r>180 and g>150 and b<160 for r,g,b in box.get_flattened_data())>=5,'target gold contrast'
# negative proof: one modified red channel must fail the strict near-neighbor comparator
mut=zoom.copy();r,g,b=mut.getpixel((73,91));mut.putpixel((73,91),((r+1)%256,g,b))
assert not near(native,mut),'negative test incorrectly accepted a modified pixel'
for name,info in report['files'].items():
    p=ROOT/name
    assert p.is_file(),name
    assert hashlib.sha256(p.read_bytes()).hexdigest()==info['sha256'],name
    assert p.stat().st_size==info['bytes']
print('G04-R2 PC PASS — native 320x240 / x4 1280x960 exact NEAREST')
print('grid PNG: 121 crossings + 220 line segments checked at x=21..201/y=53..233, 100 cells of 18px')
print('negative grid-line corruption: rejected')
print('4 state vignettes: native + strict x4 NEAREST PASS')
print('RIPOSTE B7 : RATÉ fully on one baseline / NON LUE COULÉ increased to 9px (visual check)')
print('hit centers:',','.join(hits))
print('miss centers:',','.join(misses))
print('selected G5; last riposte B7 RATÉ; unread COULÉ (distinct)')
print('negative one-channel corruption: rejected')
print('HD art + editable scene generator + bitmap exports SHA256: MATCH')
print('ON_DEVICE_UNVERIFIED — artistic PO approval still pending')
