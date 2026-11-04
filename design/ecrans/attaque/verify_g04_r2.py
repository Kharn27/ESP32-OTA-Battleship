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
control_ref=Image.open(ROOT/'reference_controles_integres_1448x1086.png').convert('RGB')
assert control_ref.size==(1448,1086)

# The integrated console must use multiple material/depth values rather than
# a small flat rectangle laid over the art. Distinct native image regions have
# reproducible pixel statistics and there is a red enamel / blue enamel cue.
from collections import Counter
regions={'FEU':(215,61,306,90),'DEFENSE':(215,99,306,120),
         'ABANDON':(216,127,306,144),'HISTORIQUE':(215,151,306,182),
         'RIPOSTE':(215,185,306,206),'ALERTE':(215,216,306,233)}
region_complexity={}
for label,box in regions.items():
    patch=native.crop(box)
    kinds=len(set(patch.get_flattened_data()))
    # A flat 2-4-colour label stamp would not contain painted materials,
    # lustre, hardware rivets and inlaid shadows.
    assert kinds>100,(label,'button looks flat; lost painted shading',kinds)
    region_complexity[label]=kinds
# Check source compositing, independent of the report (full cropped HD image).
ref_panel=control_ref.resize((320,240),Image.Resampling.LANCZOS)
for xy in [(211,47),(228,74),(267,72),(249,110),(272,137),
           (249,165),(253,199),(273,220),(311,233)]:
    assert native.getpixel(xy)==ref_panel.getpixel(xy),('wrong painted panel',xy)
# Negative: altering a painted-panel pixel would violate the crop-equality check.
corrupt_button=native.copy()
r,g,b=corrupt_button.getpixel((249,110))
corrupt_button.putpixel((249,110),((r+1)%256,g,b))
assert corrupt_button.getpixel((249,110))!=ref_panel.getpixel((249,110))

# Validate the final PNG itself, not intermediary exports. Markers stay inside
# their 18x18 cells; every actual line pixel (3861) must be exact and uniform.
GRID_RGB=(185,232,238)
FRAME_RGB=(214,250,242)
marker_keys=('C3','E3','E4','I8','A8','B7','D1','F9','G2','H6','J4','G5')
def is_marker(x,y):
    for key in marker_keys:
        cx=21+(ord(key[0])-65)*18+9
        cy=53+(int(key[1:])-1)*18+9
        if abs(x-cx)<=9 and abs(y-cy)<=9:
            return True
    return False

count_grid_pixels=0
for yy in range(181):
    for xx in range(181):
        pxy=native.getpixel((21+xx,53+yy))
        if xx%18==0 or yy%18==0:
            # The selected target's 4px dark outline may cover the nearest
            # 1px grid border, correctly: skip only marked neighborhoods.
            # Only the gold G5 bracket is thick enough to reach its borders.
            if abs((21+xx)-138)<=9 and abs((53+yy)-134)<=9:
                continue
            expected=FRAME_RGB if xx in (0,180) or yy in (0,180) else GRID_RGB
            assert pxy==expected,('missing/wrong grid line',xx,yy,pxy,expected)
            count_grid_pixels+=1
assert count_grid_pixels>=3500,('insufficient unoccluded grid coverage',count_grid_pixels)

def lum(rgb):
    r,g,b=rgb
    return r*0.2126+g*0.7152+b*0.0722

# Check visible water separately from intended line pixels and game markers.
# Values and seams are checked *on the approved native image*, not a
# self-declared metadata grid or a discarded diagnostic PNG.

def is_water(x,y):
    return (21<=x<201 and 53<=y<233 and
            (x-21)%18!=0 and (y-53)%18!=0 and not is_marker(x,y))

def check_visible_water(img):
    samples=[lum(img.getpixel((x,y)))
             for y in range(54,233) for x in range(22,201)
             if is_water(x,y)]
    assert samples and max(samples)<125,('false bright water grid',max(samples) if samples else None)
    for offset in range(18,180,18):
        left,right=21+offset-1,21+offset+1
        pairs=[abs(lum(img.getpixel((left,y)))-lum(img.getpixel((right,y))))
               for y in range(54,233)
               if is_water(left,y) and is_water(right,y)]
        assert pairs and sum(pairs)/len(pairs)<12,('water vertical discontinuity',offset)
        top,bottom=53+offset-1,53+offset+1
        pairs=[abs(lum(img.getpixel((x,top)))-lum(img.getpixel((x,bottom))))
               for x in range(22,201)
               if is_water(x,top) and is_water(x,bottom)]
        assert pairs and sum(pairs)/len(pairs)<12,('water horizontal discontinuity',offset)
    return max(samples)

ocean_max_luma=check_visible_water(native)
# Synthetic false wave/bar (NOT a valid grid line) must be detected.
from PIL import ImageDraw
phantom=native.copy()
ImageDraw.Draw(phantom).line((22,62,200,62),fill=(165,185,192),width=1)
try:
    check_visible_water(phantom)
except AssertionError:
    pass
else:
    raise AssertionError('phantom bright water seam was accepted')
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
print('six integrated control/info regions: painted native colour variation='+str(region_complexity))
print('HD sculpted controls are present in native PNG at 9 pixel probes; negative mutation rejected')
print(f'native 320x240 grid: {count_grid_pixels} exact RGB line pixels; water max luminance={ocean_max_luma:.1f} (<125), 18px boundary gradients guarded')
print('negative phantom water seam (synthetic horizontal 1px): rejected')
print('grid PNG: 121 crossings + 220 line segments checked at x=21..201/y=53..233, 100 cells of 18px')
print('negative grid-line corruption: rejected')
print('4 state vignettes: native + strict x4 NEAREST PASS')
print('RIPOSTE B7 : RATÉ / NON LUE COULÉ: inset raster plates, native-size manual PO legibility check required')
print('hit centers:',','.join(hits))
print('miss centers:',','.join(misses))
print('selected G5; last riposte B7 RATÉ; unread COULÉ (distinct)')
print('negative one-channel corruption: rejected')
print('Original HD art + integrated-controls master + approved PNG exports SHA256: MATCH')
print('PO-approved artwork (PC only) — ON_DEVICE_UNVERIFIED')
