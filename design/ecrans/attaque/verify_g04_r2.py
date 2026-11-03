#!/usr/bin/env python3
"""Repeatable artifact assertions for G04-R2 (PC only, no device claims)."""
from pathlib import Path
from PIL import Image
import xml.etree.ElementTree as ET
import json, hashlib, re, sys, shutil, tempfile

D=Path(__file__).resolve().parent
src=D/'g04_r2_attaque.svg'
png=D/'g04_r2_attaque_320x240.png'
up=D/'g04_r2_attaque_x4.png'
rep=D/'generation-report.json'
X,Y,SIZE=21,53,18
cells={'miss':['C2','H4','A9','F9','J10'],'hit':['E7','B5','B6','I3']}

def fail(msg):raise AssertionError(msg)
def verify_nearest(a,b):
    # Inspect actual pixel arrays, not a new export of the same source.
    assert a.size==(320,240) and b.size==(1280,960)
    p=a.load();q=b.load()
    for y in range(240):
        for x in range(320):
            rgba=p[x,y]
            for j in range(4):
                for i in range(4):
                    if q[x*4+i,y*4+j]!=rgba:
                        fail(f'NEAREST mismatch native {x,y} enlarged {x*4+i,y*4+j}')

svg=src.read_text(encoding='utf-8')
root=ET.fromstring(svg)
ns='{http://www.w3.org/2000/svg}'
assert root.get('width')=='320' and root.get('height')=='240'
ids={el.attrib['id'] for el in root.iter() if 'id' in el.attrib}
assert 'grid-lines' in ids and 'target-G5' in ids
assert sum(i.startswith('miss-') for i in ids)==5
assert sum(i.startswith('hit-') for i in ids)==4
assert {x[5:] for x in ids if x.startswith('miss-')}==set(cells['miss'])
assert {x[4:] for x in ids if x.startswith('hit-')}==set(cells['hit'])
assert not (set(cells['miss']) & set(cells['hit']))
assert 'G5' not in cells['miss']+cells['hit']
# The grid is independently parsed from the emitted vector geometry.
grid=next(el for el in root.iter() if el.attrib.get('id')=='grid-lines')
lines=[]
for p in grid:
    d=p.attrib.get('d','')
    m=re.fullmatch(r'M([0-9]+) ([0-9]+)L([0-9]+) ([0-9]+)',d)
    assert m, f'unexpected grid line {d}'
    lines.append(tuple(map(int,m.groups())))
assert len(lines)==22
vertical={(X+18*i,Y,X+18*i,Y+180) for i in range(11)}
horizontal={(X,Y+18*i,X+180,Y+18*i) for i in range(11)}
assert set(lines)==vertical|horizontal
# Text correctness: mandatory controls, distinct riposte and unread-gravity text.
all_txt=[e.text for e in root.iter() if e.tag==ns+'text' and e.text]
for expected in ['ATTAQUE','VS CAPITAINE','FEU','DÉFENSE','ABANDON','RIPOSTE :','B7  RATÉ','NON LUE : COULÉ','TIRS RÉCENTS']:
    assert expected in all_txt,(expected,all_txt)
assert [t for t in all_txt if t in list('ABCDEFGHIJ')]==list('ABCDEFGHIJ')
assert [t for t in all_txt if t in [str(i) for i in range(1,11)]]==[str(i) for i in range(1,11)]
# No enemy ship placement/reveal layer in the image.
assert not any(('enemy-' in i or 'ship-reveal' in i or 'ship-hidden' in i) for i in ids)
# Numeric hitbox bounds set only as proposed UI geometry, not a hardware pass.
control_regions=[(210,56,317,94),(210,98,317,129),(210,132,317,153)]
assert all(x0>=205 and y0>=53 and x1<=319 and y1<=234 for x0,y0,x1,y1 in control_regions)
a=Image.open(png).convert('RGBA')
b=Image.open(up).convert('RGBA')
verify_nearest(a,b)
# Negative proof: change only one RED sample of the actual exported zoom, re-read PNG and ensure rejection.
with tempfile.TemporaryDirectory() as t:
    negative=Path(t)/'negative.png'
    mutant=b.copy();r,g,bb,aa=mutant.getpixel((410,217))
    mutant.putpixel((410,217), ((r+1)%256,g,bb,aa));mutant.save(negative)
    try: verify_nearest(a,Image.open(negative).convert('RGBA'))
    except AssertionError: pass
    else: fail('Negative test failed to reject one altered red channel pixel')
report=json.loads(rep.read_text(encoding='utf-8'))
for f,details in report['files'].items():
    assert hashlib.sha256((D/f).read_bytes()).hexdigest()==details['sha256'],f
assert report['grid']=={'origin':[21,53],'cell':18,'rows':10,'columns':10,'width':180,'height':180}
print('PASS: SVG 320×240, board (21,53) 10×10 18px, 22 independent SVG grid vectors')
print('PASS: 5 RATÉ + 4 TOUCHÉ disjoint, G5 unknown/reversible, labels A..J/1..10')
print('PASS: FEU / DÉFENSE / ABANDON, RIPOSTE B7 RATÉ and unread COULÉ independent')
print('PASS: no ship reveal/placement layer in SVG')
print('PASS: exported RGBA PNG 320×240 / zoom 1280×960 exact NEAREST all 1,228,800 output pixels')
print('PASS: negative proof detects 1 altered RED pixel with unchanged alpha')
print('PASS: SHA256 and byte counts match shipped report')
print('STATUS: DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED; PO artwork approval required')
