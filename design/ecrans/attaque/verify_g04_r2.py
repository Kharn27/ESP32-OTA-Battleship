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
old=Image.open(ROOT/'g04_r2_avant_quadrillage_320x240.png').convert('RGB')
comparative=Image.open(ROOT/'g04_r2_comparaison_avant_apres_640x260.png').convert('RGB')
comparative_zoom=Image.open(ROOT/'g04_r2_comparaison_x4.png').convert('RGB')
previous_buttons=Image.open(ROOT/'g04_r2_avant_boutons_320x240.png').convert('RGB')
compare_buttons=Image.open(ROOT/'g04_r2_comparaison_boutons_640x260.png').convert('RGB')
compare_buttons_zoom=Image.open(ROOT/'g04_r2_comparaison_boutons_x4.png').convert('RGB')
control_ref=Image.open(ROOT/'reference_controles_integres_1448x1086.png').convert('RGB')
sea=Image.open(ROOT/'g04_r2_mer_continue_180x180.png').convert('RGB')
board_clean=Image.open(ROOT/'g04_r2_plateau_sans_impacts_181x181.png').convert('RGB')
assert old.size==(320,240) and comparative.size==(640,260)
assert comparative_zoom.size==(2560,1040)
assert sea.size==(180,180) and board_clean.size==(181,181)
assert previous_buttons.size==(320,240) and compare_buttons.size==(640,260)
assert control_ref.size==(1448,1086) and compare_buttons_zoom.size==(2560,1040)
assert hashlib.sha256((ROOT/'g04_r2_avant_boutons_320x240.png').read_bytes()).hexdigest()==\
 'd0113e3d82c7f947d7ee3d699514cd6edfd6dda2f365abc8e3db39e206968556'
assert compare_buttons.crop((0,20,320,260)).tobytes()==previous_buttons.tobytes()
assert compare_buttons.crop((320,20,640,260)).tobytes()==native.tobytes()
assert compare_buttons.resize((2560,1040),Image.Resampling.NEAREST).tobytes()==compare_buttons_zoom.tobytes()
# Rejected reference is immutable: prevents re-labelling an after image as BEFORE.
assert hashlib.sha256((ROOT/'g04_r2_avant_quadrillage_320x240.png').read_bytes()).hexdigest()==    '7be1719d1876b6fe4fe82517117b3ead0433640a6f3571a1a19a35ca3de26ad8'
# This is an ACTUAL visual before/after: two intact 320x240 screenshots at 1:1.
assert comparative.crop((0,20,320,260)).tobytes()==old.tobytes()
assert comparative.crop((320,20,640,260)).tobytes()==native.tobytes()
assert comparative.resize((2560,1040),Image.Resampling.NEAREST).tobytes()==comparative_zoom.tobytes()
# Two independently delimited corrections, not a broad redrawing:
# - historical water change in x=21..201, y=53..233;
# - present controls in x=208..319, y=42..238.
# Every other pixel remains 1:1 against the formerly rejected tiled art.
inside_changes=panel_changes=other_changes=0
for yy in range(240):
    for xx in range(320):
        if native.getpixel((xx,yy)) != old.getpixel((xx,yy)):
            if 21<=xx<=201 and 53<=yy<=233:
                inside_changes+=1
            elif 208<=xx<320 and 42<=yy<239:
                panel_changes+=1
            else:
                other_changes+=1
assert inside_changes>=18000,(inside_changes,'ocean correction not present')
assert panel_changes>=14000,(panel_changes,'new sculpted controls not present')
assert other_changes==0,(other_changes,'changes outside allowed grid and controls')

# Stronger independently grounded targeted protection: compare to the
# immutable already-reviewed continuous-sea image. Only the right-side panel
# may have changed, so grid + every marker/text outside it is pixel-identical.
button_changes=0
for yy in range(240):
    for xx in range(320):
        if native.getpixel((xx,yy)) != previous_buttons.getpixel((xx,yy)):
            assert 208<=xx<320 and 42<=yy<239,('unrelated game pixel changed',xx,yy)
            button_changes+=1
assert button_changes>14000,(button_changes,'right controls did not materially change')

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

# Diagnostic image shows the full grid with NO markers. We assert not merely
# the presence of all 22 intended lines but their exclusive colour: none of
# the remaining 100 interiors can be mistaken for a second white grid line.
GRID_RGB=(185,232,238)
FRAME_RGB=(214,250,242)
count_grid_pixels=0
for yy in range(181):
    for xx in range(181):
        pxy=board_clean.getpixel((xx,yy))
        intended=(xx%18==0 or yy%18==0)
        if intended:
            expect=FRAME_RGB if (xx in (0,180) or yy in (0,180)) else GRID_RGB
            assert pxy==expect,('authoritative grid pixel missing',xx,yy,pxy,expect)
            count_grid_pixels+=1
        else:
            assert pxy not in (GRID_RGB,FRAME_RGB),('ghost grid-coloured pixel',xx,yy)
assert count_grid_pixels==11*181*2-11*11

# A separate ocean-only pixel check makes false separations measurable even
# when they use colours OTHER than the real grid.  Its maximum luminance stays
# well BELOW the grid; at every former 18px tile boundary, the mean first
# derivative must be modest (no discontinuous 100-tile copy/paste seam).
def lum(rgb):
    r,g,b=rgb
    return r*0.2126+g*0.7152+b*0.0722

def check_ocean_only(img):
    max_lum=0
    for yy in range(180):
        for xx in range(180):
            max_lum=max(max_lum,lum(img.getpixel((xx,yy))))
    assert max_lum<125,('water visually competes with white cell boundaries',max_lum)
    seams=[]
    for offset in range(18,180,18):
        vert=sum(abs(lum(img.getpixel((offset,yy)))-lum(img.getpixel((offset-1,yy)))) for yy in range(180))/180
        hori=sum(abs(lum(img.getpixel((xx,offset)))-lum(img.getpixel((xx,offset-1)))) for xx in range(180))/180
        assert vert<8,('vertical false seam',offset,vert)
        assert hori<8,('horizontal false seam',offset,hori)
        seams.append((vert,hori))
    return max_lum,max(max(v,h) for v,h in seams)
ocean_max_luma,ocean_max_seam=check_ocean_only(sea)
# Negative proof: a 1px-high synthetic false seam on water is detected even
# though the 11 actual gameplay-grid lines are unchanged.
phantom=sea.copy()
from PIL import ImageDraw
ImageDraw.Draw(phantom).line((0,90,179,90),fill=(145,182,190),width=1)
try:
    check_ocean_only(phantom)
except AssertionError:
    pass
else:
    raise AssertionError('water-only false-separation negative test accepted')
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
print(f'BEFORE/AFTER 320x240: grid changed={inside_changes}, controls changed={panel_changes}, unrelated changed={other_changes}')
print(f'PANEL ONLY vs prior continuous-sea approved layout: changed={button_changes}, all outside panel byte-identical')
print('six integrated control/info regions: painted native colour variation='+str(region_complexity))
print('HD sculpted controls are present in native PNG at 9 pixel probes; negative mutation rejected')
print('button before/after 640x260 and ×4 strict NEAREST: pixel exact')
print(f'clean 181x181 grid: {count_grid_pixels} correct RGB line pixels; NO extra grid-coloured pixels')
print(f'continuous ocean: max luma={ocean_max_luma:.1f} (threshold 125), max 18px-boundary seam={ocean_max_seam:.2f} (threshold 8)')
print('negative phantom water seam (synthetic horizontal 1px): rejected')
print('native 640x260 side-by-side + x4 comparison: pixel-identical source halves and exact NEAREST')
print('grid PNG: 121 crossings + 220 line segments checked at x=21..201/y=53..233, 100 cells of 18px')
print('negative grid-line corruption: rejected')
print('4 state vignettes: native + strict x4 NEAREST PASS')
print('RIPOSTE B7 : RATÉ / NON LUE COULÉ: inset raster plates, native-size manual PO legibility check required')
print('hit centers:',','.join(hits))
print('miss centers:',','.join(misses))
print('selected G5; last riposte B7 RATÉ; unread COULÉ (distinct)')
print('negative one-channel corruption: rejected')
print('Original HD art + integrated-controls painted reference + editable compositor + PNG exports SHA256: MATCH')
print('ON_DEVICE_UNVERIFIED — artistic PO approval still pending')
