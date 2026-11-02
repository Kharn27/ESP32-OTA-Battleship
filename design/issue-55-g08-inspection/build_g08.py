#!/usr/bin/env python3
"""I-009 CYD WARSHIP / G08 revision (Issue #55).
Original 320x240 pixel-oriented SVG, PNG (CairoSVG) and nearest x4 (Pillow).
This is a PC DESIGN_PREVIEW, not a firmware asset or a physical CYD test.
"""
from __future__ import annotations
from pathlib import Path
from collections import Counter
import random, html, hashlib, json
import xml.etree.ElementTree as ET
import cairosvg
from PIL import Image

OUT=Path(__file__).resolve().parent
W,H=320,240
COLORS={
 'navy':'#071a2c','deep':'#0c2d46','sea':'#0d5675','sea2':'#0c3d5d',
 'foam':'#d4e9e4','cream':'#fff1cf','gold':'#f5c16c','copper':'#c77d48',
 'steel':'#9cbdc6','steel_hi':'#d5deda','steel_dark':'#42657e',
 'hit':'#ef513b', 'miss':'#e1f2e9'
}
# Immutable, fictional DEFEAT example: human fleet is lost on DEFENSE,
# captain still has 5 intact segments on ATTAQUE. No new shot after verdict.
SHIPS=(
 ('porte-avions',tuple((c,1) for c in range(1,6))),
 ('croiseur',tuple((8,r) for r in range(3,7))),
 ('escorteur-1',tuple((c,5) for c in range(3,6))),
 ('escorteur-2',tuple((0,r) for r in range(7,10))),
 ('patrouilleur',tuple((c,8) for c in range(6,8))),
)
FLEET=frozenset(k for _,coords in SHIPS for k in coords)
UNTOUCHED=frozenset({(5,1),(8,6),(5,5),(0,9),(7,8)})
HITS=frozenset(FLEET-UNTOUCHED)
MISSES=frozenset({(0,0),(4,0),(7,0),(9,0),(6,1),(9,1),(0,2),(3,2),(7,2),
 (2,3),(5,3),(9,3),(1,4),(6,4),(3,4),(7,4),(0,5),(2,6),(4,6),
 (6,6),(9,6),(2,7),(4,7),(7,7),(9,8),(3,9),(9,9)})


def attrs(**kw):
    return ' '.join(f'{k.replace("_","-")}="{html.escape(str(v),quote=True)}"' for k,v in kw.items() if v is not None)
def rect(x,y,w,h,fill,**kw):return '<rect '+attrs(x=x,y=y,width=w,height=h,fill=fill,**kw)+'/>'
def line(x1,y1,x2,y2,color,w=1,**kw):return '<line '+attrs(x1=x1,y1=y1,x2=x2,y2=y2,stroke=color,stroke_width=w,**kw)+'/>'
def poly(points,fill,**kw):return '<polygon '+attrs(points=' '.join(f'{x},{y}' for x,y in points),fill=fill,**kw)+'/>'
def circ(cx,cy,r,fill,**kw):return '<circle '+attrs(cx=cx,cy=cy,r=r,fill=fill,**kw)+'/>'
def text(x,y,s,size=10,fill='#fff1cf',weight=800,anchor='start',**kw):
    return '<text '+attrs(x=x,y=y,font_family='DejaVu Sans, sans-serif',font_size=size,font_weight=weight,text_anchor=anchor,fill=fill,**kw)+'>'+html.escape(str(s))+'</text>'
def group(gid,s,**kw):return '<g '+attrs(id=gid,**kw)+'>'+''.join(s)+'</g>'
def svg(s):return '<svg xmlns="http://www.w3.org/2000/svg" width="320" height="240" viewBox="0 0 320 240" shape-rendering="crispEdges">'+''.join(s)+'</svg>\n'
def cstr(c):return f'{chr(65+c[0])}{c[1]+1}'


def check_data():
    assert sorted((len(q) for _,q in SHIPS),reverse=True)==[5,4,3,3,2]
    assert len(FLEET)==17 and len(HITS)==12 and len(UNTOUCHED)==5 and len(MISSES)==27
    assert HITS.isdisjoint(MISSES) and MISSES.isdisjoint(FLEET)
    assert not(UNTOUCHED & HITS) and HITS|UNTOUCHED==FLEET
    allseg=[]
    for nm,coords in SHIPS:
        assert len(set(coords))==len(coords)
        assert all(0<=c<10 and 0<=r<10 for c,r in coords)
        xs=sorted({c for c,r in coords}); ys=sorted({r for c,r in coords})
        assert len(xs)==1 or len(ys)==1
        z=xs if len(xs)>1 else ys
        assert z==list(range(z[0],z[-1]+1))
        allseg.extend(coords)
    assert len(set(allseg))==len(allseg)==17
    assert len(HITS)<17,'a defeat cannot be a captain fully sunk by the human'

# All graphics are authored on native 320x240 coordinate grid.
def sea_background():
    s=[]
    bands=[(0,14,'#102a44'),(14,32,'#173550'),(32,49,'#28425a'),
           (49,66,'#4e5263'),(66,81,'#75636c'),(81,96,'#ab7774'),
           (96,112,'#ca8b75'),(112,130,'#3e7286'),(130,166,'#0e708a'),
           (166,203,'#0b5b79'),(203,240,'#0a4462')]
    for y0,y1,col in bands:s.append(rect(0,y0,W,y1-y0,col))
    # Broken clouds and sunset integrated with the sea, not stickers.
    for x,y,w,h,col in [(9,55,32,3,'#38425d'),(0,59,55,4,'#48445f'),
         (41,64,50,3,'#53455b'),(95,63,36,3,'#684f62'),
         (245,49,40,4,'#39415b'),(270,54,43,5,'#3b465d'),
         (289,61,29,4,'#656078'),(1,86,71,2,'#d79b7c'),
         (118,93,38,2,'#ebac85'),(278,89,40,2,'#eba87d')]:
        s.append(rect(x,y,w,h,col))
    rng=random.Random(50808)
    pal=['#217e90','#2d8c94','#84b5ac','#346f85','#0b4562','#11556b']
    for _ in range(230):
        x=rng.randrange(0,319); y=rng.randrange(111,240)
        w=rng.choice((3,5,8,11));h=rng.choice((1,1,1,2))
        s.append(rect(x,y,min(w,W-x),h,rng.choice(pal),opacity=rng.choice(('.20','.30','.42','.54'))))
    # Gold reflections from sky on water.
    for i,(y,w) in enumerate([(110,43),(119,37),(132,31),(143,26),(153,18),(165,10)]):
        s.append(rect(271-w//2,y,w,2,'#e1ad7c',opacity='.28'))
    return group('mer-continue-coucher-de-soleil',s,data_scene='maritime-g01-g07')


def viewport_border():
    # Small corners and understated gold, not a generic dashboard bezel.
    s=[rect(7,43,198,196,'#061a2b',opacity='.88'),
       rect(8,44,196,194,'none',stroke='#e1b875',stroke_width=1.3),
       rect(10,46,192,190,'none',stroke='#345f75',stroke_width=1),
       rect(9,45,7,2,'#ffe2a3'),rect(197,45,6,2,'#ffe2a3'),
       rect(9,232,7,2,'#ffe2a3'),rect(197,232,6,2,'#ffe2a3')]
    return group('cadre-grille-cuivre',s)


def water_grid():
    s=[rect(21,53,180,180,'#0c4468'),rect(21,53,180,17,'#0d5273',opacity='.45')]
    rr=random.Random(24055)
    for r in range(10):
        for c in range(10):
            x,y=21+18*c,53+18*r
            s.append(rect(x+1,y+1,16,16,['#0b476a','#0b4264','#0b4c6d'][(r+c)%3],opacity='.78',data_cell=cstr((c,r))))
            if (c,r) not in HITS|MISSES:
                if (c+r)%2==0:
                    s.append(rect(x+rr.randrange(2,8),y+rr.randrange(4,12),rr.choice((2,3,4)),1,'#377c93',opacity='.64'))
    for i in range(11):
        s.append(line(21+18*i,53,21+18*i,233,'#5b99b0',.8,opacity='.89'))
        s.append(line(21,53+18*i,201,53+18*i,'#5b99b0',.8,opacity='.89'))
    # Letter and row numbers stay outside active 180x180 field.
    for c,ch in enumerate('ABCDEFGHIJ'):s.append(text(30+18*c,51,ch,8.4,'#ffe3ad',800,'middle'))
    for r in range(10):s.append(text(18,64+18*r,str(r+1),7.9,'#ffe3ad',800,'end'))
    return group('grille-10x10',s,data_columns='10',data_rows='10')


def ship_hull(name,coords):
    xs=[c for c,r in coords];ys=[r for c,r in coords]
    col=min(xs);row=min(ys);n=len(coords)
    x,y=21+18*col,53+18*row
    horizontal=len(set(ys))==1
    parts=[]
    if horizontal:
        w=n*18
        # 3-tier superstructure with bow and stern, overall silhouette integrated in water.
        parts.extend([poly([(x+1,y+9),(x+8,y+4),(x+w-7,y+4),(x+w-1,y+9),
                     (x+w-6,y+14),(x+7,y+14)],'#08283a'),
          poly([(x+2,y+9),(x+9,y+5),(x+w-8,y+5),(x+w-3,y+9),
                (x+w-8,y+12),(x+9,y+12)],'#8ca8b3'),
          line(x+9,y+6,x+w-9,y+6,'#e1e3d6',2),
          line(x+9,y+12,x+w-9,y+12,'#385c76',2),
          rect(x+w/2-11,y+3,23,3,'#48687c'),
          rect(x+w/2-8,y+1,17,3,'#bed2d0'),
          rect(x+w/2-4,y-1,9,3,'#748d9a'),
          rect(x+w/2-1,y-4,3,4,'#e6d9b6')])
        if n>=4:parts.extend([rect(x+15,y+7,6,2,'#567989'),rect(x+w-23,y+7,6,2,'#567989')])
        parts.append(line(x+7,y+15,x+w-7,y+15,'#c3e0d8',1,opacity='.65'))
    else:
        h=n*18
        parts.extend([poly([(x+9,y+1),(x+15,y+8),(x+15,y+h-9),(x+9,y+h-1),
                   (x+3,y+h-9),(x+3,y+8)],'#08283a'),
          poly([(x+9,y+3),(x+13,y+10),(x+13,y+h-10),(x+9,y+h-4),
                  (x+5,y+h-10),(x+5,y+10)],'#9aafb7'),
          rect(x+6,y+11,2,h-22,'#d4ded9'),rect(x+12,y+10,2,h-20,'#3c6476'),
          rect(x+6,y+h/2-9,6,18,'#526f81'),
          rect(x+7,y+h/2-5,4,10,'#bdcdc8'),
          rect(x+8,y+h/2-1,2,4,'#e6d9b6'),
          rect(x+7,y+12,4,2,'#466576'),rect(x+7,y+h-14,4,2,'#466576')])
        parts.append(line(x+15,y+11,x+15,y+h-11,'#c3e0d8',1,opacity='.6'))
    return group('navire-'+name,parts,data_ship=name,data_length=n,
                 data_cells=','.join(cstr(k) for k in coords),data_layer='coque-sous-marquage')


def fleets(visible):
    return group('flotte-capitaine-revelee' if visible else 'flotte-capitaine-secrete',
         [ship_hull(name,coords) for name,coords in SHIPS] if visible else [],
         data_visibility='visible' if visible else 'secret')


def hit(cx,cy):
    # G07-style burst: red-copper centre, cream foam at bow/waterline, recognizable X.
    # Cross stays above deck and is never interpreted as a different shot.
    return [rect(cx-6,cy+4,13,2,'#082c45',opacity='.8'),
     rect(cx-5,cy-3,11,9,'#f9d4a2',opacity='.90'),
     rect(cx-4,cy-4,9,2,'#fff5cb'),
     rect(cx-6,cy+4,3,2,'#d3ece6'),rect(cx+4,cy+5,4,1,'#cbe5df'),
     poly([(cx-4,cy-5),(cx,cy-1),(cx+4,cy-5),(cx+6,cy-3),
           (cx+2,cy+1),(cx+6,cy+5),(cx+3,cy+7),(cx,cy+3),
           (cx-4,cy+6),(cx-6,cy+3),(cx-2,cy),(cx-6,cy-3)],'#7f2029'),
     poly([(cx-4,cy-5),(cx,cy-1),(cx+4,cy-5),(cx+5,cy-3),
           (cx+1,cy+1),(cx+5,cy+5),(cx+3,cy+6),(cx,cy+3),
           (cx-4,cy+6),(cx-5,cy+4),(cx-1,cy),(cx-5,cy-3)],'#f3543f'),
     rect(cx-1,cy-1,3,3,'#ffc176')]

def miss(cx,cy):
    # Hollow circular splash, plus foam strokes: no mistaken untouched steel.
    return [circ(cx,cy,5.5,'#0a3c57',stroke='#f5f4db',stroke_width=2),
            rect(cx-7,cy+6,4,1,'#a9d3d2'),rect(cx+4,cy+6,4,1,'#a9d3d2'),
            rect(cx+5,cy-7,2,1,'#d9efdf')]

def shots():
    parts=[]
    for c,r in sorted(MISSES,key=lambda k:(k[1],k[0])):
        parts.append(group('rate-'+cstr((c,r)),miss(30+18*c,62+18*r),
                          data_shot='miss',data_cell_coord=cstr((c,r))))
    for c,r in sorted(HITS,key=lambda k:(k[1],k[0])):
        parts.append(group('touche-'+cstr((c,r)),hit(30+18*c,62+18*r),
                          data_shot='hit',data_cell_coord=cstr((c,r))))
    return group('historique-tirs-immuable',parts,data_layer='marques-au-dessus-des-coques')


def plate(x,y,w,h,gold=False):
    outer='#cf9954' if gold else '#b48a61'
    fill='#a85b39' if gold else '#163d56'
    dark='#382d30' if gold else '#061d32'
    return [rect(x+1,y+2,w,h,dark),rect(x,y,w,h,outer),rect(x+2,y+2,w-4,h-4,fill),
     rect(x+3,y+3,w-6,2,'#ffe0a0' if gold else '#5c8da2'),
     rect(x+3,y+h-5,w-6,2,'#693528' if gold else '#0d273d'),
     rect(x+2,y+2,2,h-4,'#d9a568' if gold else '#4c7890')]


def button(identifier,action,x,y,w,h,label,gold=False,size=9):
    parts=plate(x,y,w,h,gold)
    parts.append(text(x+w/2,y+h/2+3.5,label,size,'#fff1cf',900,'middle'))
    return group(identifier,parts,data_ui='tactile-command',data_action=action,
                 data_hitbox=f'{x},{y},{w},{h}')


def captain_portrait():
    # Small but purpose-drawn pixel character, not a scaled stock illustration.
    # An expressive naval portrait in navy, copper, warm skin and white beard.
    p=[]
    # Moon and warship horizon: all on same water.
    p.extend([rect(207,44,111,100,'#0a304d'),
       rect(210,47,105,65,'#163955'),
       rect(210,66,105,20,'#2f4c64'),
       circ(287,68,20,'#f7dc9e'),
       rect(265,50,13,2,'#657387',opacity='.7'),
       rect(273,47,27,2,'#6d788a',opacity='.6'),
       rect(294,87,20,2,'#93a0a1',opacity='.65'),
       rect(208,104,108,28,'#0e5373'),
       rect(208,122,108,8,'#0a3b5e')])
    # Distant military vessel silhouette on horizon (not a fishing boat).
    p.extend([poly([(266,98),(313,98),(308,106),(274,106)],'#071c31'),
       rect(278,92,26,7,'#071c31'),rect(285,86,14,6,'#0a2237'),
       rect(291,81,2,8,'#0a2237'),rect(303,92,5,2,'#071c31'),
       line(279,107,307,107,'#91c7c0',1),
       rect(270,109,11,1,'#74adbb'),rect(287,110,23,1,'#9cd0c8')])
    # Body first, behind head.
    p.extend([poly([(216,102),(221,98),(248,98),(256,104),(263,128),
                    (213,128)],'#07172b'),
       poly([(219,104),(229,99),(243,99),(255,106),(261,127),
             (214,127)],'#153c5a'),
       rect(219,103,7,5,'#e4aa50'),rect(245,103,8,5,'#e4aa50'),
       rect(217,107,11,2,'#ffe08d'),rect(244,107,13,2,'#ffe08d'),
       rect(231,106,5,19,'#e8e0cd'),
       rect(238,109,2,12,'#e4af54'),
       rect(227,113,3,3,'#cda35b'),rect(246,113,3,3,'#cda35b')])
    # Ears and face.
    p.extend([rect(226,75,24,20,'#e4b78f'),rect(225,80,3,9,'#d39672'),
       rect(248,80,4,9,'#d39672'),rect(228,77,19,12,'#f0c69c'),
       rect(228,81,5,3,'#c08a6b'),rect(241,81,5,3,'#c08a6b'),
       rect(230,80,3,2,'#172a38'),rect(242,80,3,2,'#172a38'),
       rect(235,82,3,8,'#d69c75'),
       rect(232,90,13,3,'#ab7359')])
    # Hair and beard: intentionally chunky pixel silhouette with shadow volume.
    p.extend([rect(224,74,5,18,'#9bb2bf'),rect(247,73,5,20,'#a0bac4'),
       poly([(224,87),(229,90),(232,93),(244,93),(250,88),
             (252,97),(248,103),(245,109),(229,109),(225,102)],'#617d91'),
       poly([(225,86),(231,90),(234,93),(241,93),(248,87),
             (251,95),(245,102),(242,107),(232,106),(226,100)],'#e1e8e6'),
       rect(232,93,12,3,'#fff3dc'),rect(229,98,5,4,'#c2d9df'),
       rect(244,98,3,3,'#bdd4db'),rect(234,103,7,2,'#b3c9d0')])
    # Naval cap, white crown with navy brim and gold icon.
    p.extend([poly([(219,66),(221,58),(227,54),(244,54),
                   (251,58),(255,66),(252,74),(222,74)],'#111f32'),
       poly([(221,64),(224,58),(231,55),(244,56),(251,60),
              (252,68),(247,70),(225,70)],'#e9e8dc'),
       rect(222,65,30,3,'#ffffff'),
       poly([(220,69),(255,69),(254,76),(217,76)],'#10263d'),
       rect(222,69,30,2,'#e8aa52'),
       poly([(219,74),(256,74),(252,78),(224,79),(217,77)],'#0a152a'),
       rect(236,58,2,10,'#e3a73e'),rect(233,62,8,2,'#e3a73e'),
       rect(234,65,6,2,'#e3a73e'),
       rect(215,128,100,2,'#d7a366')])
    return group('illustration-capitaine-et-escorte',p,data_role='maritime-pixel-art')


def header():
    p=[rect(0,0,320,41,'#081b30'),rect(0,39,320,2,'#ac754c'),
       rect(0,41,320,2,'#1b5065'),
       rect(3,3,317,1,'#27465f'),
       text(10,23,'ATTAQUE',11.5,'#fff2d1',900),
       text(11,34,'10 × 10',6.6,'#a5c6cb',700)]
    # DÉFENSE is raised and clearly tactile; verdict is ink ONLY.
    p.append(button('commande-defense','consulter-defense',96,5,108,30,'DÉFENSE',False,10.4))
    p.append(text(150,32,'',1))
    # secondary line inside defense plate, deliberately distinguishable.
    p.append(text(150,31,'RIPOSTE : COULÉ',6.8,'#d6e7dc',750,'middle'))
    p.extend([group('verdict-information',[
        text(261,12,'RÉSULTAT',6.8,'#9fc3c8',750,'middle'),
        text(261,27,'DÉFAITE',11.4,'#ffc6a4',900,'middle'),
        text(261,36,'VS CAPITAINE',6.6,'#fbe4c5',750,'middle')
    ],data_ui='information-only',data_interaction='none',data_outcome='defeat')])
    return group('barre-verdict-g07',p)


def right_panel(state):
    p=[rect(207,43,111,197,'#061b2f',opacity='.34')]
    p.append(captain_portrait())
    # Non-tactile status plaque; visible text strongly legible on small screen.
    label={'A':'FLOTTE SECRÈTE','B':'FLOTTE RÉVÉLÉE','C':'FLOTTE MASQUÉE'}[state]
    status=[rect(210,128,106,17,'#101f36'),rect(210,128,106,1,'#f1c16e'),
          rect(210,143,106,1,'#ab794f'),text(263,140,label,8.1,'#fff2d1',850,'middle')]
    p.append(group('indicateur-flotte-non-tactile',status,data_ui='information-only',data_interaction='none',data_visibility=state))
    p.append(button('commande-rejouer','retour-deploiement',211,149,105,26,'REJOUER',True,10.4))
    p.append(button('commande-accueil','sortie-sans-forfait',211,182,105,26,'ACCUEIL',False,10.2))
    label={'A':'VOIR SA FLOTTE','B':'MASQUER FLOTTE','C':'AFFICHER FLOTTE'}[state]
    action={'A':'voir-flotte-volontaire','B':'masquer-flotte','C':'afficher-flotte-sans-confirmation'}[state]
    p.append(button('commande-flotte',action,211,215,105,23,label,False,8.7))
    return group('panneau-capitaine-et-commandes',p)


def state_svg(state):
    assert state in 'ABC'
    s=[sea_background(),viewport_border(),water_grid(),
       fleets(state=='B'),shots(),right_panel(state),header()]
    return svg(s)


def sheet_svg(states):
    # Three reduced integrated screens + legible legend/flow at native 320x240.
    s=[rect(0,0,320,240,'#0b2339'),rect(0,0,320,29,'#17374b'),
       text(8,17,'G08  /  APRÈS DÉFAITE',11,'#fff2d1',900),
       text(311,25,'3 ÉTATS  •  MÊME ATTAQUE',7.1,'#f7c98b',750,'end'),
       line(6,29,314,29,'#e4aa68')]
    labels={'A':('A / SECRET','AVANT LE CHOIX'),
            'B':('B / RÉVÉLÉ','APRÈS APPUI'),
            'C':('C / MASQUÉ','RÉVERSIBLE')}
    for i,k in enumerate('ABC'):
        x=6+i*105
        s.append(text(x,43,labels[k][0],8.5,'#ffe1a9',850))
        # SVG source is 320x240, viewport is 99x74, no new full defense mockup.
        inner=states[k].split('>',1)[1].rsplit('</svg>',1)[0]
        s.append(f'<svg x="{x}" y="49" width="99" height="74" viewBox="0 0 320 240">{inner}</svg>')
        s.append(rect(x,49,99,74,'none',stroke='#caa16f',stroke_width=1))
        s.append(text(x+49.5,138,labels[k][1],7.7,'#cfe4de',750,'middle'))
    s.extend([line(8,147,312,147,'#638597'),
       text(9,164,'A → B : VOIR SA FLOTTE (volontaire)',8.5,'#fff0ce',850),
       text(9,180,'B → C : MASQUER FLOTTE',8.5,'#fff0ce',850),
       text(9,196,'C → B : AFFICHER FLOTTE',8.5,'#fff0ce',850),
       rect(8,205,304,29,'#153c55'),rect(8,205,304,1,'#f2bd72'),
       text(160,218,'ATTAQUE → DÉFENSE → ATTAQUE',8.6,'#ffdda3',900,'middle'),
       text(160,230,'Visibilité B ou C conservée · Aucun FEU',7.7,'#d1ece5',750,'middle')])
    return svg(s)


def verify_svg(states):
    ns='{http://www.w3.org/2000/svg}'
    shot_snap=[]
    for st,src in states.items():
        root=ET.fromstring(src)
        assert root.attrib['width']=='320' and root.attrib['height']=='240'
        cells=[e.attrib['data-cell'] for e in root.iter() if 'data-cell' in e.attrib]
        # Other elements may contain data-cell-coord but these are not tiles.
        assert len(cells)==100 and len(set(cells))==100, (st,len(cells))
        shot={e.get('data-cell-coord'):e.get('data-shot') for e in root.iter() if e.get('data-shot')}
        assert len(shot)==39, (st,len(shot))
        assert {k for k,v in shot.items() if v=='hit'} == {cstr(c) for c in HITS}
        assert {k for k,v in shot.items() if v=='miss'} == {cstr(c) for c in MISSES}
        shot_snap.append(shot)
        revealed=[e for e in root.iter() if e.get('data-ship')]
        if st=='B':
            assert len(revealed)==5 and sum(int(e.get('data-length')) for e in revealed)==17
            coords=[item for e in revealed for item in e.get('data-cells').split(',')]
            assert len(set(coords))==17 and set(coords)=={cstr(c) for c in FLEET}
            assert src.index('id="flotte-capitaine-revelee"')<src.index('id="historique-tirs-immuable"')
        else:
            assert not revealed, 'A/C must not reveal fleet'
            assert 'id="flotte-capitaine-revelee"' not in src
        verdict=[e for e in root.iter() if e.get('id')=='verdict-information']
        assert len(verdict)==1 and verdict[0].get('data-ui')=='information-only'
        assert not any(ch.tag==ns+'rect' for ch in verdict[0].iter())
        commands={e.get('data-action') for e in root.iter() if e.get('data-ui')=='tactile-command'}
        assert commands == {'consulter-defense','retour-deploiement','sortie-sans-forfait',
                    {'A':'voir-flotte-volontaire','B':'masquer-flotte','C':'afficher-flotte-sans-confirmation'}[st]}
        assert 'FEU' not in src and 'VICTOIRE' not in src and 'MATCH NUL' not in src
        assert 'RIPOSTE : COULÉ' in src
    assert shot_snap[0]==shot_snap[1]==shot_snap[2]
    print('PASS SVG 3 états: 100 cases uniques, 5 coques B / 0 A,C, 17 segments 5-4-3-3-2')
    print('PASS Tirs : 12 touches, 27 ratés, coordonnées identiques A/B/C, impacts sur coques')
    print('PASS Résultat DÉFAITE informatif, commandes tactiles, riposte COULÉ, aucun FEU')

def export(name,source):
    native=OUT/(name+'_320x240.png')
    cairosvg.svg2png(bytestring=source.encode('utf-8'),write_to=str(native),output_width=W,output_height=H)
    with Image.open(native) as im:im=im.convert('RGBA')
    assert im.size==(320,240)
    nearest=im.resize((1280,960),Image.Resampling.NEAREST)
    z=OUT/(name+'_x4.png');nearest.save(z)
    with Image.open(z) as zoom:
        zoom=zoom.convert('RGBA')
        assert zoom.size==(1280,960)
        # Each source pixel is replicated into an exact 4x4 block.
        px=im.load();zp=zoom.load()
        for y in range(240):
            for x in range(320):
                original=px[x,y]
                assert all(zp[4*x+dx,4*y+dy]==original for dx in range(4) for dy in range(4)), (name,x,y)
    print('PASS EXPORT '+name+': 320x240 and 1280x960 NEAREST pixel-for-pixel')

def main():
    check_data()
    states={state:state_svg(state) for state in 'ABC'}
    verify_svg(states)
    for st in 'ABC':
        (OUT/f'g08_etat_{st.lower()}_320x240.svg').write_text(states[st],encoding='utf-8')
    (OUT/'g08_inspection_320x240.svg').write_text(states['B'],encoding='utf-8')
    sheet=sheet_svg(states)
    (OUT/'g08_variantes_320x240.svg').write_text(sheet,encoding='utf-8')
    export('g08_inspection',states['B'])
    export('g08_variantes',sheet)
    export('g08_etat_a',states['A'])
    export('g08_etat_c',states['C'])
    generated=sorted(OUT.glob('g08_*.svg'))+sorted(OUT.glob('g08_*.png'))
    evidence=['G08 #55 — design preview PC, ON_DEVICE_UNVERIFIED',
      'python3 design/issue-55-g08-inspection/build_g08.py',
      'PASS structural scenario and 100-grid / shots / 5 ships checks',
      'PASS main & variants & A/C PNG, strict 4x4 nearest pixel check',
      'Scenario fictif: capitaine 5 navires (5,4,3,3,2), 12 TOUCHÉ, 27 RATÉ, 5 segments intacts',
      'Files SHA256:']+[hashlib.sha256(path.read_bytes()).hexdigest()+'  '+path.name for path in generated]
    (OUT/'generation-report.txt').write_text('\n'.join(evidence)+'\n',encoding='utf-8')
    print('PASS exports et SHA256 vérifiés; aucune validation matérielle')

if __name__=='__main__':main()