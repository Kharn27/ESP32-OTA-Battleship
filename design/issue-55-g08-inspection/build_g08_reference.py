#!/usr/bin/env python3
"""G08 — reconstruction fidèle de la référence artistique PO (Issue #55).

La capture PO est exploitée comme référence d'habillage ; la grille, ses impacts,
les navires et les états sont recomposés en calques reproductibles.
PREVIEW PC : aucun engagement sur budget Flash, conversion runtime ou CYD réel.
"""
from __future__ import annotations
from pathlib import Path
import hashlib, json, os, zipfile, io, random, math
from PIL import Image, ImageDraw, ImageFont, ImageChops

HERE=Path(__file__).resolve().parent
W,H=1448,1086
SOURCE=HERE/'reference_design_PO.png'
assert SOURCE.exists(), f'Reférence PO manquante : {SOURCE}'
ROOT=Image.open(SOURCE).convert('RGBA')
assert ROOT.size==(W,H), ROOT.size
OUT=HERE
NAVAL={'PORTE_AVIONS':['B5','C5','D5','E5','F5'], 'CROISEUR':['F7','G7','H7','I7'],
       'ESCORTE_1':['J2','J3','J4'], 'ESCORTE_2':['C6','C7','C8'], 'PATROUILLEUR':['B9','C9']}
UNTOUCHED=set('J4 D5 C8 I7 B9'.split())
HITS=set(sum(NAVAL.values(), []))-UNTOUCHED
MISSES=set('A1 B1 G1 H1 A3 D3 G3 F4 H5 J5 B6 E6 G6 A7 J8 D9 H9 E10 I10 J10'.split())
assert len(NAVAL)==5 and sorted(map(len,NAVAL.values()))==[2,3,3,4,5]
assert len(HITS)==12 and len(UNTOUCHED)==5 and len(MISSES)==20
assert HITS.isdisjoint(MISSES) and MISSES.isdisjoint(set(sum(NAVAL.values(),[])))
assert len(set(sum(NAVAL.values(),[])))==17
GX,GY=121,227
DX,DY=64.2,58.0
fontp='/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf'
if not Path(fontp).exists():fontp='/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'

def font(n):return ImageFont.truetype(fontp,n)
def ij(s):return ord(s[0])-65,int(s[1:])-1
def center(s):
    c,r=ij(s);return (round(GX+(c+.5)*DX),round(GY+(r+.5)*DY))

def line_step(draw,coords,fill,width=1):draw.line(coords,fill=fill,width=width,joint='curve')
def center_text(d,box,label,size,color='#fff5d7',shadow='#071d30',stroke=2):
    x0,y0,x1,y1=box
    ft=font(size)
    while d.textbbox((0,0),label,font=ft)[2]>x1-x0-18 and size>20:
        size-=1;ft=font(size)
    bbox=d.textbbox((0,0),label,font=ft)
    xx=round((x0+x1-(bbox[2]-bbox[0]))/2);yy=round((y0+y1-(bbox[3]-bbox[1]))/2-bbox[1])
    d.text((xx,yy),label,font=ft,fill=color,stroke_fill=shadow,stroke_width=stroke)

# Carre d'eau recoloré sous la grille. Il épouse la gamme chromatique et le motif de la capture.
def grid_layer(visible:bool):
    layer=Image.new('RGBA',(W,H),(0,0,0,0));d=ImageDraw.Draw(layer,'RGBA')
    d.rectangle((GX,GY,GX+10*DX,GY+10*DY),fill=(2,42,77,255))
    rng=random.Random(5508)
    for r in range(10):
        for c in range(10):
            x=int(GX+c*DX);y=int(GY+r*DY)
            ink=(1,48+(c+r)%3*2,88+(r%4)*2,255)
            d.rectangle((x+3,y+3,x+DX-2,y+DY-2),fill=ink)
            for k in range(3):
                ox=rng.randint(5,round(DX)-15);oy=rng.randint(8,round(DY)-10)
                d.line((x+ox,y+oy,x+ox+rng.randint(5,18),y+oy),fill=(24,83,123,rng.randint(35,80)),width=1)
    for i in range(11):
        x=round(GX+i*DX);d.line([(x,GY),(x,round(GY+10*DY))],fill=(87,159,204,255),width=3)
        y=round(GY+i*DY);d.line([(GX,y),(round(GX+10*DX),y)],fill=(86,155,199,255),width=3)
    if visible:
        for name,cells in NAVAL.items():draw_ship(d,name,cells)
    for point in MISSES:draw_miss(d,*center(point))
    for point in HITS:draw_hit(d,*center(point))
    return layer

def draw_ship(d,name,cells):
    coords=[ij(x) for x in cells]
    cc=[x for x,y in coords];rr=[y for x,y in coords]
    x1=round(GX+min(cc)*DX+7);x2=round(GX+(max(cc)+1)*DX-7)
    y1=round(GY+min(rr)*DY+8);y2=round(GY+(max(rr)+1)*DY-8)
    h=len(set(rr))==1
    if h:
        cy=(y1+y2)//2
        outline=[(x1-3,cy),(x1+18,y1-4),(x2-17,y1-4),(x2+2,y1+6),(x2+2,y2-6),(x2-17,y2+4),(x1+18,y2+4)]
        main=[(x1,cy),(x1+22,y1),(x2-15,y1),(x2-1,y1+8),(x2-1,y2-7),(x2-15,y2),(x1+22,y2)]
        d.polygon(outline,fill=(7,26,43,255));d.polygon(main,fill=(151,172,194,255))
        d.line([(x1+23,y1+2),(x2-20,y1+2)],fill=(245,245,235,255),width=5)
        d.line([(x1+24,y2-4),(x2-20,y2-4)],fill=(56,87,111,255),width=4)
        d.rectangle((x1+40,cy-12,x2-40,cy+9),fill=(108,129,152,255))
        d.rectangle((x1+int((x2-x1)*.45),cy-27,x1+int((x2-x1)*.58),cy-14),fill=(228,232,224,255))
        d.rectangle((x1+int((x2-x1)*.52),cy-42,x1+int((x2-x1)*.54),cy-27),fill=(241,237,209,255))
        for x in range(x1+55,x2-40,40):d.rectangle((x,cy-7,x+18,cy-2),fill=(55,87,111,255))
        d.line((x1+20,y2+5,x2-15,y2+5),fill=(190,221,233,180),width=3)
    else:
        cx=(x1+x2)//2
        outline=[(cx,y1-5),(x2+3,y1+24),(x2+3,y2-22),(cx,y2+5),(x1-3,y2-22),(x1-3,y1+24)]
        main=[(cx,y1),(x2,y1+28),(x2,y2-25),(cx,y2),(x1,y2-25),(x1,y1+28)]
        d.polygon(outline,fill=(8,27,43,255));d.polygon(main,fill=(151,171,192,255))
        d.line((x1+6,y1+27,x1+6,y2-28),fill=(230,236,234,255),width=5)
        d.line((x2-6,y1+27,x2-6,y2-28),fill=(56,89,118,255),width=5)
        d.rectangle((cx-14,(y1+y2)//2-24,cx+13,(y1+y2)//2+25),fill=(100,124,147,255))
        d.rectangle((cx-10,(y1+y2)//2-30,cx+10,(y1+y2)//2-12),fill=(222,226,217,255))
        d.rectangle((cx-3,(y1+y2)//2-47,cx+3,(y1+y2)//2-30),fill=(244,232,203,255))
        d.line((x2+5,y1+34,x2+5,y2-33),fill=(185,221,228,180),width=3)

def draw_hit(d,cx,cy):
    k=15
    # Les tirs sont au-dessus des coques ; même motif avec ou sans révélation.
    for col,w in [((7,22,37,240),15),((92,5,8,255),11),((242,27,22,255),8)]:
        d.line((cx-k,cy-k,cx+k,cy+k),fill=col,width=w)
        d.line((cx+k,cy-k,cx-k,cy+k),fill=col,width=w)
    for x in [-21,20]:d.line((cx+x-5,cy+20,cx+x+3,cy+20),fill=(181,214,223,220),width=2)

def draw_miss(d,cx,cy):
    d.ellipse((cx-15,cy-15,cx+15,cy+15),outline=(10,34,52,255),width=10)
    d.ellipse((cx-14,cy-14,cx+14,cy+14),outline=(244,250,248,255),width=6)
    d.line((cx-10,cy+16,cx+11,cy+16),fill=(122,194,223,175),width=2)

def panel_layer(state:str):
    l=Image.new('RGBA',(W,H),(0,0,0,0));d=ImageDraw.Draw(l,'RGBA')
    # Recouvre intégralement les textes et le cartouche de la référence HD avant
    # de dessiner les états. Aucun ancien texte ne doit dépasser à droite.
    d.rectangle((788,200,1420,446),fill=(3,26,51,255))
    frame=[(811,202),(1378,202),(1405,224),(1405,423),(1381,441),(814,441),(790,423),(790,225)]
    d.polygon(frame,fill=(4,30,58,255),outline=(248,211,141,255),width=6)
    inner=[(818,211),(1374,211),(1396,230),(1396,417),(1377,431),(817,431),(801,417),(801,231)]
    d.polygon(inner,fill=(4,27,52,255),outline=(19,71,103,255),width=4)
    # Information non-tactile. Le contour vert reprend littéralement la capture PO.
    x0,y0,x1,y1=815,245,1367,366
    outside=[(x0+20,y0),(x1-13,y0),(x1,y0+12),(x1,y1-14),(x1-14,y1),(x0+17,y1),(x0,y1-16),(x0,y0+18)]
    inside=[(x0+29,y0+7),(x1-18,y0+7),(x1-7,y0+18),(x1-7,y1-18),(x1-20,y1-8),(x0+24,y1-8),(x0+7,y1-20),(x0+7,y0+24)]
    d.polygon(outside,fill=(15,136,27,255),outline=(231,255,122,255),width=6)
    d.polygon(inside,fill=(2,92,16,255),outline=(87,228,50,255),width=3)
    d.line((x0+36,y0+5,x1-40,y0+5),fill=(171,253,95,255),width=5)
    # Icon window, diamonded corners.
    d.rounded_rectangle((844,273,921,340),radius=12,fill=(4,106,14,255),outline=(204,250,128,255),width=5)
    if state=='A':
        # Lock icon for secret fleet.
        d.arc((863,280,903,318),180,360,fill=(252,255,244,255),width=8)
        d.rounded_rectangle((858,300,907,328),radius=7,fill=(254,250,217,255))
        d.ellipse((880,310,887,318),fill=(4,71,14,255))
        label='FLOTTE SECRÈTE'; subtitle='Aucune position révélée'
    elif state=='B':
        d.line([(854,306),(876,326),(909,286)],fill=(245,255,235,255),width=12,joint='curve')
        label='5 NAVIRES RÉVÉLÉS'; subtitle='Les tirs restent visibles'
    else:
        d.ellipse((859,294,907,322),outline=(255,251,227,255),width=6)
        d.ellipse((876,302,890,316),fill=(255,250,225,255))
        d.line((852,327,913,286),fill=(255,236,186,255),width=10)
        label='FLOTTE MASQUÉE';subtitle='Les tirs restent visibles'
    center_text(d,(934,260,1348,346),label,42,stroke=3)
    # Text line under panel (on navy card).
    d.rectangle((811,367,1375,435),fill=(5,25,48,255))
    center_text(d,(816,372,1365,423),subtitle,32,'#d8e5f3',stroke=2)
    # Quatre commandes de fin de partie, toutes dans les cadres vitrés et cuivrés
    # de la capture. Nécessaire pour conserver DÉFENSE et ACCUEIL sans sous-menu.
    d.rectangle((0,854,1448,1086),fill=(4,31,58,255))
    water=random.Random(82055)
    for _ in range(260):
        ox=water.randrange(0,1448);oy=water.randrange(860,1085)
        d.line((ox,oy,min(1447,ox+water.randrange(8,40)),oy),fill=(16,72,111,water.randrange(90,220)),width=2)
    def button(x0,x1,kind,label,icon):
        y0,y1=879,1021
        gold=kind=='gold'
        outer=(255,222,145,255) if gold else (211,235,255,255)
        bright=(255,250,186,255) if gold else (167,213,246,255)
        middle=(255,183,19,255) if gold else (21,77,125,255)
        dark=(159,86,11,255) if gold else (7,39,72,255)
        d.rounded_rectangle((x0+2,y0+8,x1+5,y1+9),radius=20,fill=(0,8,16,255))
        pts=[(x0+29,y0),(x1-21,y0),(x1,y0+22),(x1,y1-29),(x1-25,y1),(x0+25,y1),(x0,y1-23),(x0,y0+27)]
        d.polygon(pts,fill=dark,outline=outer,width=7)
        pts=[(x0+36,y0+11),(x1-31,y0+11),(x1-11,y0+34),(x1-11,y1-35),(x1-36,y1-12),(x0+34,y1-12),(x0+11,y1-34),(x0+11,y0+37)]
        d.polygon(pts,fill=middle,outline=bright,width=5)
        d.line((x0+35,y0+16,x1-37,y0+16),fill=bright,width=5)
        d.line((x0+32,y1-18,x1-38,y1-18),fill=(12,32,55,255) if gold else (10,44,78,255),width=6)
        icx=x0+57;icy=950;ink=(3,32,61,255) if gold else (246,248,245,255)
        if icon=='eye':
            d.arc((icx-25,icy-16,icx+25,icy+17),195,350,fill=ink,width=7)
            d.arc((icx-25,icy-16,icx+25,icy+17),15,172,fill=ink,width=7)
            d.ellipse((icx-8,icy-8,icx+8,icy+8),fill=ink)
            if state=='B':d.line((icx-29,icy+23,icx+28,icy-25),fill=ink,width=7)
        elif icon=='replay':
            d.arc((icx-27,icy-24,icx+27,icy+26),40,300,fill=ink,width=10)
            d.polygon([(icx+27,icy-23),(icx+38,icy-7),(icx+17,icy-8)],fill=ink)
        elif icon=='defense':
            d.polygon([(icx,icy-29),(icx+24,icy-18),(icx+19,icy+14),(icx,icy+32),(icx-19,icy+14),(icx-24,icy-18)],fill=ink)
            d.polygon([(icx,icy-16),(icx+12,icy-10),(icx+10,icy+8),(icx,icy+18),(icx-10,icy+8),(icx-12,icy-10)],fill=middle)
        elif icon=='home':
            d.polygon([(icx-28,icy-3),(icx,icy-30),(icx+28,icy-3),(icx+20,icy-3),(icx+20,icy+26),(icx-20,icy+26),(icx-20,icy-3)],fill=ink)
            d.rectangle((icx-5,icy+9,icx+7,icy+26),fill=middle)
        center_text(d,(x0+94,912,x1-9,991),label,51,
            '#0a2942' if gold else '#fff4dc',stroke=2 if gold else 4)
    button(24,480,'blue',{'A':'VOIR SA FLOTTE','B':'MASQUER','C':'AFFICHER'}[state],'eye')
    button(501,799,'gold','REJOUER','replay')
    button(820,1139,'blue','DÉFENSE','defense')
    button(1160,1443,'blue','ACCUEIL','home')
    return l

# Layered OpenRaster file: each layer editable in GIMP/Krita, original atmosphere untouched.
def ora(base,grid,panel,merged,path):
    import xml.etree.ElementTree as E
    root=E.Element('image',{'w':str(W),'h':str(H),'name':'G08 — référence PO intégrée'})
    stack=E.SubElement(root,'stack')
    for file,name in [('data/layer2.png','03 • État et commande'),('data/layer1.png','02 • Grille et tirs'),('data/layer0.png','01 • Référence PO')]:
        E.SubElement(stack,'layer',{'name':name,'src':file,'x':'0','y':'0','opacity':'1.0','visibility':'visible','composite-op':'svg:src-over'})
    with zipfile.ZipFile(path,'w') as z:
        z.writestr('mimetype','image/openraster',compress_type=zipfile.ZIP_STORED)
        z.writestr('stack.xml',E.tostring(root,encoding='utf-8'),compress_type=zipfile.ZIP_DEFLATED)
        for file,im in [('data/layer0.png',base),('data/layer1.png',grid),('data/layer2.png',panel),('mergedimage.png',merged)]:
            b=io.BytesIO();im.save(b,format='PNG',optimize=True);z.writestr(file,b.getvalue(),compress_type=zipfile.ZIP_DEFLATED)

def build():
    imgs={}; report=[]
    for state in 'ABC':
        grid=grid_layer(state=='B')
        panel=panel_layer(state)
        merged=Image.alpha_composite(Image.alpha_composite(ROOT,grid),panel).convert('RGB')
        native=merged.resize((320,240),Image.Resampling.LANCZOS)
        imgs[state]=native
        native.save(OUT/f'g08_etat_{state.lower()}_320x240.png',optimize=True)
        native.resize((1280,960),Image.Resampling.NEAREST).save(OUT/f'g08_etat_{state.lower()}_x4.png',optimize=True)
        # Source layers at native: recreate through resampling with alpha preserve.
        baseN=ROOT.resize((320,240),Image.Resampling.LANCZOS)
        gridN=grid.resize((320,240),Image.Resampling.LANCZOS)
        panelN=panel.resize((320,240),Image.Resampling.LANCZOS)
        # Source high-resolution layered ORA retains intricate sea and frame details.
        ora(ROOT,grid,panel,merged,OUT/f'g08_etat_{state.lower()}_layers.ora')
        report.append(f'etat {state}: {len(HITS)} TOUCHÉ, {len(MISSES)} RATÉ, {len(UNTOUCHED)} segments intacts, coques={state=="B"}')
    imgs['B'].save(OUT/'g08_inspection_320x240.png',optimize=True)
    imgs['B'].resize((1280,960),Image.Resampling.NEAREST).save(OUT/'g08_inspection_x4.png',optimize=True)
    row=Image.new('RGB',(960,240))
    for i,s in enumerate('ABC'):row.paste(imgs[s],(320*i,0))
    row.save(OUT/'g08_variantes_320x240.png',optimize=True)
    row.resize((3840,960),Image.Resampling.NEAREST).save(OUT/'g08_variantes_x4.png',optimize=True)
    data={'scenario':'Fictif — défaite contre Capitaine','fleet':NAVAL,'hits':sorted(HITS),'misses':sorted(MISSES),'untouched':sorted(UNTOUCHED),'states':{'A':'flotte secrète','B':'flotte révélée','C':'flotte masquée après inspection'},'historical_shots_unchanged':True,'source':'Illustration PO transmise en conversation le 03/11/2026'}
    (OUT/'scenario_G08.json').write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')
    outputs=[p for p in OUT.glob('g08_*.png') if p.name!='reference_down_320.png' and not p.name.startswith('ref_')]
    report += ['NATIVE 320x240 PASS','NEAREST x4 1280x960 PASS','100 cases (10x10) ; navires 5/4/3/3/2, 17 cases, aucun recouvrement','UNE grille ATTAQUE ; 12 TOUCHÉ, 20 RATÉ, 5 segments adverses intacts','TIRS IDENTIQUES A/B/C, couches coques sous tirs','États A/C sans coque ; B avec 5 coques','SOURCE utilisateur : reference_design_PO.png']
    for p in outputs:
        x=Image.open(p)
        assert x.size in {(320,240),(1280,960),(960,240),(3840,960)},(p.name,x.size)
        report.append(f'{p.name} {x.size} {p.stat().st_size} octets sha256:{hashlib.sha256(p.read_bytes()).hexdigest()}')
    for s in 'abc':
        n=Image.open(OUT/f'g08_etat_{s}_320x240.png')
        z=Image.open(OUT/f'g08_etat_{s}_x4.png')
        assert list(n.resize(z.size,Image.Resampling.NEAREST).getdata())==list(z.getdata())
    (OUT/'generation-report.txt').write_text('\n'.join(report)+'\n')
    print('\n'.join(report[:9]))

if __name__=='__main__':build()
