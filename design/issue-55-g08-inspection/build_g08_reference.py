#!/usr/bin/env python3
"""G08 / Issue #55 — PO Option A. Native 320×240 game mockups, layered ORA.

The PO illustration remains the *visual* reference. Scene layout, 10x10 grid,
steel ships, hits/misses and controls are redrawn at native resolution.
PC design preview ONLY; not an ESP32 runtime image contract.
Requires Python 3 + Pillow. Run from any directory.
"""
from __future__ import annotations
from pathlib import Path
import hashlib, io, json, random, tempfile, zipfile
import xml.etree.ElementTree as ET
from PIL import Image, ImageDraw, ImageFont

D = Path(__file__).resolve().parent
REF = D/'reference_design_PO.png'
W,H = 320,240
X0,Y0,S = 21,53,18
NAVAL = {
 'PORTE_AVIONS': ['B5','C5','D5','E5','F5'],
 'CROISEUR': ['F7','G7','H7','I7'],
 'ESCORTE_1': ['J2','J3','J4'],
 'ESCORTE_2': ['C6','C7','C8'],
 'PATROUILLEUR': ['B9','C9'],
}
UNTOUCHED = set('J4 D5 C8 I7 B9'.split())
CELLS = set(sum(NAVAL.values(), []))
HITS = CELLS - UNTOUCHED
MISSES = set('A1 B1 G1 H1 A3 D3 G3 F4 H5 J5 B6 E6 G6 A7 J8 D9 H9 E10 I10 J10'.split())
STATES = {'A':('VOIR SA FLOTTE','FLOTTE SECRÈTE','Navires cachés'),
          'B':('MASQUER','FLOTTE RÉVÉLÉE','5 navires ennemis'),
          'C':('AFFICHER','FLOTTE MASQUÉE','Retour possible')}
BG = '#051c35'
CREAM = '#fff0c5'
FONT = '/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf'
if not Path(FONT).exists(): FONT='/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'

def font(n): return ImageFont.truetype(FONT,n)
def blank(): return Image.new('RGBA',(W,H),(0,0,0,0))
def cell_xy(s):
    col=ord(s[0])-ord('A'); row=int(s[1:])-1
    return X0+col*S,Y0+row*S

def text(d,xy,label,n=9,color=CREAM,anchor='mm',stroke=0,shadow='#081d36'):
    d.text(xy,label,font=font(n),fill=color,anchor=anchor,
           stroke_width=stroke,stroke_fill=shadow)

def line(d,pts,fill,width=1):d.line(pts,fill=fill,width=width)
def polygon(d,pts,fill,outline=None,width=1):d.polygon(pts,fill=fill)

def decorative_sea():
    # Night sea from same PO palette, beneath the large existing G07 board.
    im=Image.new('RGBA',(W,H),'#051e3a');d=ImageDraw.Draw(im)
    swatches=['#082f51','#063352','#074163','#073a5e','#062f51','#062b48','#052a47','#052542','#041d39']
    for i,shade in enumerate(swatches):
        y=42+i*23
        d.rectangle((0,y,W-1,min(239,y+23)),fill=shade)
    r=random.Random(5608)
    for _ in range(390):
        y=r.randint(47,239);x=r.randint(0,319);w=r.choice((2,3,5,7,9,12))
        shade=r.choice(('#0b5070','#1a5a79','#0a4868','#2a6c82','#0a3e61'))
        d.line((x,y,min(319,x+w),y),fill=shade)
    # PO's illustrated Captain, golden victory-result header, ship silhouette
    # and nocturnal sky are preserved in place, without rescaling the grid.
    with Image.open(REF) as source:
        art=source.convert('RGBA')
        assert art.size==(1448,1086),art.size
        top=art.resize((W,H),Image.Resampling.LANCZOS).crop((0,0,W,42))
    im.paste(top,(0,0),top)
    # Subtle gold rule between header and ATTAQUE field.
    d=ImageDraw.Draw(im)
    d.rectangle((0,40,319,41),fill='#8e633e')
    return im

def board_water():
    im=blank();d=ImageDraw.Draw(im)
    # G07 exact 180×180 area, origin = (21,53), no translation or scale.
    d.rectangle((X0,Y0,X0+S*10-1,Y0+S*10-1),fill='#0a3858')
    r=random.Random(5518)
    for y in range(10):
        for x in range(10):
            xx,yy=X0+x*S,Y0+y*S
            d.rectangle((xx+1,yy+1,xx+S-1,yy+S-1),fill=('#114765' if (x+y)%3==0 else '#0c405f'))
            for _ in range(2):
                ox=r.randrange(3,12);oy=r.randrange(3,15)
                d.line((xx+ox,yy+oy,xx+ox+r.randrange(2,5),yy+oy),fill=r.choice(('#185672','#2c6b80','#176180')))
    # Grid lines occupy ONE pixel of every 18px boundary, never 14px cells.
    for i in range(11):
        px=X0+i*S; py=Y0+i*S
        d.line((px,Y0,px,Y0+180),fill='#61a9cc',width=1)
        d.line((X0,py,X0+180,py),fill='#61a9cc',width=1)
    # Navy/gold old brass chart frame with crisp-stepped corners.
    d.rectangle((19,51,203,235),outline='#061526',width=2)
    d.rectangle((20,52,202,234),outline='#efc580',width=1)
    d.line((22,235,200,235),fill='#ab7d48')
    for col in range(10):text(d,(X0+col*18+9,47),chr(65+col),10,CREAM)
    for row in range(10):text(d,(10,Y0+row*18+9),str(row+1),9,CREAM)
    # Marker showing the only (read-only) attack board.
    text(d,(24,39),'ATTAQUE',7,'#f9d894',anchor='lm')
    return im

def ship_layer(state):
    im=blank()
    if state!='B':return im
    d=ImageDraw.Draw(im)
    for name,ships in NAVAL.items():
        pts=[cell_xy(s) for s in ships]
        cols=[p[0] for p in pts]; rows=[p[1] for p in pts]
        left,right=min(cols),max(cols)+S
        top,bottom=min(rows),max(rows)+S
        horizontal=len(set(rows))==1
        if horizontal:
            mid=(top+bottom)//2
            # Continuous warship profile — pointed prow, layered armour and wake.
            poly=[(left+1,mid),(left+6,top+4),(right-5,top+4),
                  (right-1,mid),(right-5,bottom-4),(left+6,bottom-4)]
            polygon(d,poly,'#061a30')
            polygon(d,[(left+3,mid),(left+7,top+5),(right-6,top+5),
                  (right-3,mid),(right-6,bottom-5),(left+7,bottom-5)],'#a3b3bf')
            line(d,(left+8,top+6,right-8,top+6),'#f3ebd7',1)
            line(d,(left+7,bottom-6,right-8,bottom-6),'#38677f',2)
            d.rectangle((left+17,mid-2,right-16,mid+1),fill='#5c7f90')
            xmid=(left+right)//2
            d.rectangle((xmid-5,mid-5,xmid+5,mid-3),fill='#d7dfd8')
            d.rectangle((xmid-2,mid-8,xmid+2,mid-6),fill='#e8e9df')
            d.point((xmid,mid-9),fill='#f0ca8b')
            for cx in range(left+18,right-9,18):d.rectangle((cx,mid+2,cx+5,mid+3),fill='#294e69')
            line(d,(left+8,bottom-2,right-7,bottom-2),'#6ca7bf',1)
        else:
            mid=(left+right)//2
            poly=[(mid,top+1),(right-4,top+7),(right-4,bottom-6),
                  (mid,bottom-1),(left+4,bottom-6),(left+4,top+7)]
            polygon(d,poly,'#07182a')
            polygon(d,[(mid,top+3),(right-6,top+8),(right-6,bottom-7),
                  (mid,bottom-3),(left+6,bottom-7),(left+6,top+8)],'#9baebc')
            line(d,(left+7,top+10,left+7,bottom-10),'#f0eddf',1)
            line(d,(right-7,top+9,right-7,bottom-10),'#356a81',2)
            ym=(top+bottom)//2
            d.rectangle((mid-3,ym-8,mid+3,ym+9),fill='#5c7f90')
            d.rectangle((mid-2,ym-4,mid+2,ym-2),fill='#f1e5ce')
            d.rectangle((mid-1,ym-12,mid+1,ym-9),fill='#f8e4b1')
            line(d,(right-2,top+9,right-2,bottom-10),'#619cb1',1)
    return im

def impacts_layer():
    im=blank();d=ImageDraw.Draw(im)
    for s in sorted(MISSES):
        x,y=cell_xy(s);x+=9;y+=9
        d.ellipse((x-5,y-5,x+5,y+5),outline='#09243c',width=3)
        d.ellipse((x-4,y-4,x+4,y+4),outline='#f1f4e8',width=2)
        d.line((x-5,y+5,x+5,y+5),fill='#80c4d1',width=1)
    for s in sorted(HITS):
        x,y=cell_xy(s);x+=9;y+=9
        # Embers + foam, much more distinctive than the hull highlight.
        line(d,(x-5,y-5,x+5,y+5),'#2c283b',5)
        line(d,(x+5,y-5,x-5,y+5),'#2c283b',5)
        line(d,(x-4,y-4,x+4,y+4),'#fc4b2e',3)
        line(d,(x+4,y-4,x-4,y+4),'#fc4b2e',3)
        d.point((x,y),fill='#ffd591')
    return im

def octagon(d,xy,fill,border='#e9bd6b',top='#fff4c6'):
    x0,y0,x1,y1=xy
    p=[(x0+4,y0),(x1-4,y0),(x1,y0+4),(x1,y1-4),
       (x1-4,y1),(x0+4,y1),(x0,y1-4),(x0,y0+4)]
    d.polygon(p,fill=fill)
    d.line(p+[p[0]],fill=border,width=2)
    d.line((x0+5,y0+3,x1-5,y0+3),fill=top,width=1)
    d.line((x0+5,y1-3,x1-5,y1-3),fill='#123047',width=1)

def control_layer(state):
    im=blank();d=ImageDraw.Draw(im)
    # Right-side deck fits x>=207, preserving all 180×180 board pixels.
    d.rectangle((207,43,319,239),fill='#092442')
    d.line((207,43,207,239),fill='#ddab60',width=2)
    d.line((209,43,209,239),fill='#214d6c')
    d.rectangle((211,46,317,86),fill='#041b35')
    octagon(d,(211,50,317,83),'#0b4c37',border='#f6d17e',top='#a9ed9f')
    text(d,(265,58),'INSPECTION',8,'#ffe29e')
    if state=='B':small='FLOTTE RÉVÉLÉE';color='#f4e2ac'
    elif state=='C':small='FLOTTE MASQUÉE';color='#e5deb9'
    else:small='FLOTTE SECRÈTE';color='#f4e2ac'
    text(d,(265,69),small,9,color)
    text(d,(265,78),'✕ touché  ○ raté',7,'#cce6e7')
    # Four tactile commands with copper / enamel edges, not informational text.
    placements=[('toggle',(212,92,316,122),STATES[state][0],False),
                ('defense',(212,128,316,158),'DÉFENSE',False),
                ('rejouer',(212,164,316,194),'REJOUER',True),
                ('accueil',(212,200,316,230),'ACCUEIL',False)]
    for k,(cmd,xy,label,gold) in enumerate(placements):
        x0,y0,x1,y1=xy
        outer='#f6d275' if gold else '#c5ddec'
        fill='#f4b536' if gold else '#175681'
        if cmd=='toggle':
            fill='#117d47' if state=='A' else '#1c5b73'
            outer='#bbed86' if state=='A' else '#e4bd73'
        d.rectangle((x0+1,y0+3,x1+1,y1+3),fill='#061a2b')
        octagon(d,xy,fill,border=outer,top='#fdf2bb' if gold else '#9ac4e2')
        if cmd=='toggle':
            text(d,(222,(y0+y1)//2+1),'◉' if state=='B' else '◇',10,'#fff7c9',stroke=0)
        if cmd=='defense':text(d,(220,(y0+y1)//2+1),'◆',9,'#ffe2a8')
        if cmd=='rejouer':text(d,(220,(y0+y1)//2+1),'↻',12,'#12334a')
        if cmd=='accueil':text(d,(220,(y0+y1)//2+1),'⌂',14,'#fff2d5')
        labelsize=7.5 if state=='A' and cmd=='toggle' else 10
        if label=='VOIR SA FLOTTE':
            text(d,(273,(y0+y1)//2+1),'VOIR SA FLOTTE',8,'#fff7d5')
        else:text(d,(272,(y0+y1)//2+1),label,10,'#102a48' if gold else '#fff6d5')
    # Small informational note only, below action controls.
    text(d,(266,235),'VS CAPITAINE  ·  FIN',6,'#c5ddcf')
    return im

def composite(layers):
    a=blank()
    for layer in layers:a=Image.alpha_composite(a,layer)
    return a.convert('RGBA')

def save_ora(path,layers,merged):
    entries=[('00 Décor PO — portrait / mer',layers[0]),
             ('01 Plateau ATTAQUE 180 x 180',layers[1]),
             ('02 Coques révélées (B)',layers[2]),
             ('03 Historique des tirs invariant',layers[3]),
             ('04 Commandes et information',layers[4])]
    root=ET.Element('image',{'w':'320','h':'240','name':'G08 native — Option A'})
    stack=ET.SubElement(root,'stack')
    for idx,(name,_) in reversed(list(enumerate(entries))):
        ET.SubElement(stack,'layer',{'name':name,'src':f'data/layer{idx}.png',
                                    'x':'0','y':'0','opacity':'1.0','visibility':'visible',
                                    'composite-op':'svg:src-over'})
    with zipfile.ZipFile(path,'w') as z:
        z.writestr('mimetype','image/openraster',compress_type=zipfile.ZIP_STORED)
        z.writestr('stack.xml',ET.tostring(root,encoding='utf-8'))
        for idx,(_,im) in enumerate(entries):
            b=io.BytesIO();im.save(b,format='PNG')
            z.writestr(f'data/layer{idx}.png',b.getvalue(),compress_type=zipfile.ZIP_DEFLATED)
        b=io.BytesIO();merged.save(b,format='PNG')
        z.writestr('mergedimage.png',b.getvalue(),compress_type=zipfile.ZIP_DEFLATED)

def valid_scenario():
    assert X0==21 and Y0==53 and S==18 and X0+10*S==201 and Y0+10*S==233
    assert [len(p) for p in NAVAL.values()]==[5,4,3,3,2]
    assert len(CELLS)==17 and len(HITS)==12 and len(MISSES)==20
    assert len(UNTOUCHED)==5 and HITS|UNTOUCHED==CELLS
    assert not CELLS&MISSES and HITS.isdisjoint(MISSES)
    assert all(0<=ord(p[0])-65<10 and 1<=int(p[1:])<=10 for p in CELLS|MISSES)
    for name,points in NAVAL.items():
        coords=[cell_xy(p) for p in points]
        xs={x for x,y in coords};ys={y for x,y in coords}
        assert len(xs)==1 or len(ys)==1,(name,coords)
        vals=sorted(xs if len(ys)==1 else ys)
        assert vals==[vals[0]+18*i for i in range(len(vals))],name

def verify_pixel_file(path,native):
    with Image.open(path) as im:
        stored=im.convert('RGBA');assert stored.size==(1280,960)
        expected=native.resize((1280,960),Image.Resampling.NEAREST).convert('RGBA')
        # BYTE COMPARISON of FILE READ BACK, not alpha-sensitive bbox, not RAM-only.
        if stored.tobytes()!=expected.tobytes():
            raise AssertionError('RGBA PNG x4 mismatch — RGB or alpha corrupted: '+path.name)
        # Full 4x nearest shape verified by the equality, independently of encoding.
        assert stored.getpixel((4*23+3,4*57+2))==native.convert('RGBA').getpixel((23,57))
    return True

def negative_rgb_test(path,native):
    # Make a COPY of the actual file, mutate ONE red component with alpha unchanged.
    with Image.open(path) as im:
        altered=im.convert('RGBA')
    before=altered.getpixel((117,185))
    altered.putpixel((117,185),((before[0]+1)%256,before[1],before[2],before[3]))
    assert altered.getpixel((117,185))[3]==before[3]
    with tempfile.TemporaryDirectory() as temp:
        corrupted=Path(temp)/'corrupted_rgb_only.png'
        altered.save(corrupted,format='PNG')
        try:verify_pixel_file(corrupted,native)
        except AssertionError as e:
            assert 'RGBA PNG x4 mismatch' in str(e)
            return 'PASS: negative RGB-only corruption (alpha unchanged) correctly REJECTED'
    raise AssertionError('FAIL: RGB-only tampering went undetected')

def variants_sheet(images):
    out=Image.new('RGBA',(320,240),'#071f39');d=ImageDraw.Draw(out)
    d.rectangle((0,0,319,21),fill='#6e1124')
    text(d,(160,11),'G08   •   INSPECTION POST-DÉFAITE',10,'#ffe0a3')
    infos=[('A','SECRET','Premier appui volontaire'),('B','VISIBLE','5 coques sous les tirs'),
           ('C','MASQUÉ','Réaffichage possible')]
    for i,(s,title,desc) in enumerate(infos):
        y=26+i*67
        thumbnail=images[s].resize((90,67),Image.Resampling.LANCZOS)
        out.alpha_composite(thumbnail,(8,y))
        d.rectangle((7,y-1,99,y+67),outline='#e0b773',width=1)
        text(d,(109,y+13),s+' / '+title,10,'#ffdc98',anchor='lm')
        text(d,(109,y+30),desc,8,'#e3eee2',anchor='lm')
        text(d,(109,y+46),('VOIR SA FLOTTE' if s=='A' else 'MASQUER' if s=='B' else 'AFFICHER'),8,'#a8d4e4',anchor='lm')
    text(d,(160,233),'B / C conservé : ATTAQUE → DÉFENSE → ATTAQUE',7,'#f5dcab')
    return out

def build():
    valid_scenario()
    assert REF.is_file(),f'PO image missing: {REF}'
    base=decorative_sea();board=board_water();impact=impacts_layer()
    # Structural pixel proof: board layer has a 180x180 field and 18px boundaries.
    for row in (0,3,7,10):
        assert board.getpixel((X0,Y0+row*S))[:3]==(0x61,0xa9,0xcc)
    assert board.getpixel((X0+10*S,Y0+12))[:3]==(0x61,0xa9,0xcc)
    imgs={};reports=[]; impact_hashes=[]
    for state in STATES:
        ships=ship_layer(state)
        assert (ships.getbbox() is not None)==(state=='B')
        layers=[base,board,ships,impact,control_layer(state)]
        merged=composite(layers)
        imgs[state]=merged
        name=f'g08_etat_{state.lower()}'
        native=D/f'{name}_320x240.png'; zoom=D/f'{name}_x4.png'
        merged.save(native);merged.resize((1280,960),Image.Resampling.NEAREST).save(zoom)
        verify_pixel_file(zoom,merged)
        save_ora(D/f'{name}_layers.ora',layers,merged)
        with zipfile.ZipFile(D/f'{name}_layers.ora') as z:
            assert z.read('mimetype')==b'image/openraster'
            assert len([f for f in z.namelist() if f.startswith('data/layer')])==5
            grid_saved=Image.open(io.BytesIO(z.read('data/layer1.png'))).convert('RGBA')
            assert grid_saved.size==(320,240) and grid_saved.tobytes()==board.tobytes()
            impact_saved=Image.open(io.BytesIO(z.read('data/layer3.png'))).convert('RGBA')
            impact_hashes.append(hashlib.sha256(impact_saved.tobytes()).hexdigest())
            assert Image.open(io.BytesIO(z.read('mergedimage.png'))).convert('RGBA').tobytes()==merged.tobytes()
        reports.append(f'PASS STATE {state}: 320x240; x4 NEAREST; 5 ORA layers; coques visible={state=="B"}')
    assert all(imgs[s].size==(320,240) for s in STATES)
    # A and C have NO ship layer, exactly the same hits+misses in every state.
    assert impact.tobytes()==impacts_layer().tobytes()
    assert len(impact_hashes)==3 and len(set(impact_hashes))==1, 'A/B/C impact layers differ'
    assert impact_hashes[0]==hashlib.sha256(impact.tobytes()).hexdigest(), 'stored impacts mismatch'
    (D/'g08_inspection_320x240.png').write_bytes((D/'g08_etat_b_320x240.png').read_bytes())
    (D/'g08_inspection_x4.png').write_bytes((D/'g08_etat_b_x4.png').read_bytes())
    sheet=variants_sheet(imgs)
    sheet.save(D/'g08_variantes_320x240.png')
    sheet.resize((1280,960),Image.Resampling.NEAREST).save(D/'g08_variantes_x4.png')
    verify_pixel_file(D/'g08_variantes_x4.png',sheet)
    rgb_report=negative_rgb_test(D/'g08_etat_b_x4.png',imgs['B'])
    assert all((cell_xy(k)==(X0+(ord(k[0])-65)*S,Y0+(int(k[1:])-1)*S)) for k in HITS|MISSES)
    scenario={"scenario":"FICTION — DÉFAITE VS Capitaine","board":{"col_count":10,"row_count":10,"cell_px":18,"width_px":180,"height_px":180,"x":21,"y":53},
      "fleet":NAVAL,"hits":sorted(HITS),"misses":sorted(MISSES),"untouched":sorted(UNTOUCHED),
      "states":{"A":"secret, VOIR SA FLOTTE","B":"revealed, MASQUER","C":"hidden again, AFFICHER"},
      "navigation_visibility":"B/C conserved across ATTAQUE > DEFENSE > ATTAQUE (UX contract only)",
      "source":"Image de référence artistique fournie par le PO"}
    (D/'scenario_G08.json').write_text(json.dumps(scenario,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    reports.extend([
      'PASS GEOMETRY: board x21..201 y53..233; 100 cells; exact 18x18; 180x180',
      'PASS SCENARIO: fleet 5/4/3/3/2 = 17 occupied; 12 hits / 20 misses; 5 enemy segments intact; no overlap',
      'PASS COMPOSITING: ships in B only; ships layer BELOW immutable historic impacts',
      'PASS UX: no FEU, on same ATTAQUE; A=VOIR, B=MASQUER, C=AFFICHER; DEFENSE/REJOUER/ACCUEIL',
      'PASS VARIANTS: actual 320x240 + x4 actual 1280x960',rgb_report,
      'STATUS: DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED'])
    outnames=['build_g08_reference.py','README.md','scenario_G08.json','reference_design_PO.png']+[p.name for p in D.glob('g08_*.png')]+[p.name for p in D.glob('g08_*.ora')]
    for name in sorted(set(outnames)):
        p=D/name
        if p.is_file():reports.append(f'SHA256 {hashlib.sha256(p.read_bytes()).hexdigest()} {name}')
    (D/'generation-report.txt').write_text('\n'.join(reports)+'\n',encoding='utf-8')
    print('\n'.join(reports[:12]))

if __name__=='__main__':build()
