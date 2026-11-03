#!/usr/bin/env python3
"""G04-R2 / CYD WARSHIP — 320x240 vector authoring and PNG proof builder.
Concept inspired by the supplied 1448x1086 sunset navigation-table illustration.
All geometric/gameplay layers are drawn natively, without any bitmap background.
Requires: cairosvg, Pillow (host-only preview/export, not firmware tooling).
"""
from pathlib import Path
import math, hashlib, json
from PIL import Image
import cairosvg

HERE=Path(__file__).resolve().parent
W,H=320,240
GX,GY,CELL=21,53,18
MISSES=['C2','H4','A9','F9','J10']
HITS=['E7','B5','B6','I3']
TARGET='G5'

def el(s): pieces.append(s)
def rect(x,y,w,h,c,rx=0,stroke=None,sw=1, extra=''):
    el(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{c}"'+(f' stroke="{stroke}" stroke-width="{sw}"' if stroke else '')+f' {extra}/>')
def path(d,c,stroke=None,sw=1,extra=''):
    el(f'<path d="{d}" fill="{c}"'+(f' stroke="{stroke}" stroke-width="{sw}"' if stroke else '')+f' {extra}/>')
def txt(x,y,t,size=9,c='#F6F9D8',weight=700,spacing=0,extra=''):
    el(f'<text x="{x}" y="{y}" font-family="DejaVu Sans Condensed, DejaVu Sans, sans-serif" font-size="{size}" font-weight="{weight}" letter-spacing="{spacing}" fill="{c}" {extra}>{t}</text>')
def line(x1,y1,x2,y2,c,sw=1,opacity=1):
    el(f'<path d="M{x1} {y1}L{x2} {y2}" fill="none" stroke="{c}" stroke-width="{sw}" opacity="{opacity}" shape-rendering="crispEdges"/>')
def dot(x,y,c,s=1):rect(x,y,s,s,c)

pieces=[]
el('<svg xmlns="http://www.w3.org/2000/svg" width="320" height="240" viewBox="0 0 320 240">')
el('''<title>G04 R2 — ATTAQUE vs Capitaine — preview 320x240</title>
<desc>Mer plateau, grille 10 par 10, cible G5, tirs passés, riposte B7 ratée, alerte ancienne coulé non lue. Aucun navire adverse visible.</desc>
<defs>
<linearGradient id="sky" x2="0" y2="1"><stop stop-color="#613F68"/><stop offset=".5" stop-color="#B66E70"/><stop offset="1" stop-color="#F8B66C"/></linearGradient>
<linearGradient id="sea" x2="0" y2="1"><stop stop-color="#188E93"/><stop offset=".34" stop-color="#126E80"/><stop offset="1" stop-color="#07354E"/></linearGradient>
<linearGradient id="fire" x2="0" y2="1"><stop stop-color="#F6B55C"/><stop offset=".4" stop-color="#DE713A"/><stop offset="1" stop-color="#893B36"/></linearGradient>
<linearGradient id="def" x2="0" y2="1"><stop stop-color="#278892"/><stop offset=".48" stop-color="#185A6D"/><stop offset="1" stop-color="#093146"/></linearGradient>
<clipPath id="board"><rect x="21" y="53" width="180" height="180"/></clipPath>
</defs>''')
# Scenic continuity: sky is compact, ocean extends under the game grid.
rect(0,0,320,43,'url(#sky)')
rect(0,35,320,205,'url(#sea)')
# sunset and horizon: drawn as short pixel bands rather than glossy dashboard.
rect(221,17,23,3,'#F8D58A');rect(217,20,31,7,'#F8CB72');rect(220,27,25,3,'#FBD998')
rect(217,34,32,1,'#F8CB72');rect(211,37,43,1,'#D9DDB0')
rect(198,6,18,2,'#EB9C92');rect(204,4,12,2,'#EB9C92');rect(250,12,23,2,'#DCA8B1')
rect(263,10,15,2,'#DCA8B1');rect(169,26,25,1,'#FED79E')
for x,y,w in [(206,40,15),(235,42,17),(257,39,26),(10,42,19),(39,37,10),(122,42,15),(155,40,18)]:rect(x,y,w,1,'#75C5B8')
# lighthouse silhouette (non interactive) at upper left
rect(8,14,9,17,'#EBD4A7');rect(10,16,5,10,'#FEF4D3');rect(9,20,2,3,'#AA593D');rect(15,20,2,3,'#AA593D')
rect(7,12,11,2,'#663E48');rect(9,9,7,3,'#D87755');rect(11,6,3,3,'#F7D77A')
path('M2 34 H20 V36 H27 V39 H0 V36 H2Z','#2E5868')
# small distant boat far from grid: decorative, not enemy location
path('M267 35H290L285 38H271Z','#18334C');rect(278,26,1,9,'#18334C');path('M278 27 L289 33 H279Z','#E9BF95')
# title plate (carved wood/copper) x32-190; captain cameo x286-319.
rect(27,3,166,34,'#382B40',3);rect(29,5,162,30,'#955E56',2)
rect(34,7,152,1,'#F8CB72');rect(34,32,152,1,'#462B3A')
txt(42,21,'ATTAQUE',16,'#F6F9D8',800,0.6)
txt(43,31,'VS CAPITAINE',8,'#F9D7A6',700,0.5)
# Captain tiny bust: detailed enough to belong, not dominate.
rect(282,5,35,34,'#292D42',3);rect(286,29,29,9,'#982F45')
rect(291,14,19,16,'#DFAA80',3)
path('M290 23h21v6h-4v3h-13v-3h-4Z','#F0E9D7');rect(295,22,13,1,'#FFFFFF');rect(297,27,7,1,'#B6BBB4')
path('M286 12h29v3h-29Z','#17283B');path('M292 9h19v4h-19Z','#17283B');path('M298 5h8v5h-8Z','#17283B');rect(301,7,2,5,'#F8CB72');rect(289,14,24,1,'#E9B56B')
dot(298,19,'#1A2638',2);dot(306,19,'#1A2638',2);rect(297,23,10,1,'#7C5147')
rect(285,35,33,2,'#F1BA65');rect(284,38,34,2,'#563A39')
# one continuous maritime frame; single grid follows G08 18px grid origin.
rect(0,45,206,195,'#163448');rect(2,46,201,189,'#73473D')
rect(18,50,186,185,'#F7C779',1);rect(20,52,182,182,'#0A2337')
rect(GX,GY,180,180,'url(#sea)')
# sea texture is organic, large strokes across cell boundaries, under impact symbols
el('<g clip-path="url(#board)" opacity=".67">')
for i in range(23):
    y=56+i*8; x=24+((i*43)%29)
    path(f'M{x} {y} h{16+(i*3)%19} l5 -2 h{10+(i*7)%24} l5 2 h{18+(i*5)%17}', 'none','#3DA9A4',1.1,'opacity=".35"')
    if i%3==0:
        path(f'M{32+(i*17)%92} {y+3} h14 l4 -1 h21', 'none','#C0EAD1',1,'opacity=".28"')
for x,y,w in [(38,67,14),(76,79,8),(151,96,15),(40,130,23),(81,147,14),(158,161,24),(48,198,9),(119,215,21)]:
    rect(x,y,w,1,'#7ACBC2',extra='opacity=".35"')
el('</g>')
# grid strokes: absolutely 18px; no pseudo cells.
el('<g id="grid-lines" shape-rendering="crispEdges">')
for i in range(11):
    line(GX+i*CELL,GY,GX+i*CELL,GY+180,'#B2E8DE',1,.45)
    line(GX,GY+i*CELL,GX+180,GY+i*CELL,'#B2E8DE',1,.45)
el('</g>')
# top and left coordinate strips, navy+ivory, high readability.
rect(20,40,183,12,'#244A58')
rect(0,52,20,182,'#254858')
for i,c in enumerate('ABCDEFGHIJ'):
    txt(30+i*CELL,49,c,8.2,'#FFE4AB',800,0,extra='text-anchor="middle"')
for i in range(10):
    txt(11,65+i*CELL,str(i+1),8.2,'#FFE4AB',800,0,extra='text-anchor="middle"')
# light amber divider at game border.
rect(20,51,182,1,'#EEC17D');rect(20,233,183,2,'#A85D48')

def center(cell):
    cx=GX+(ord(cell[0])-65)*CELL+CELL//2
    cy=GY+(int(cell[1:])-1)*CELL+CELL//2
    return cx,cy

# Miss is an organic closed splash ring, hit an anchored hot metal/spray motif.
for key in MISSES:
    x,y=center(key)
    el(f'<g id="miss-{key}" shape-rendering="crispEdges">')
    rect(x-4,y+2,9,2,'#092C46');rect(x-6,y,3,3,'#E5F9DF');rect(x+4,y,3,3,'#E5F9DF')
    rect(x-4,y-1,9,2,'#C4F4E5');rect(x-2,y-2,5,1,'#E5F9DF')
    dot(x-4,y-5,'#D2F8E7');dot(x+3,y-6,'#C8F8E6');dot(x,y-7,'#E5F9DF')
    el('</g>')
for key in HITS:
    x,y=center(key)
    el(f'<g id="hit-{key}" shape-rendering="crispEdges">')
    rect(x-5,y+4,11,2,'#0A2538')
    path(f'M{x-1} {y-7}h3v5h4v3h-4v5h-3v-5h-5v-3h5Z','#FFD078')
    path(f'M{x-6} {y-5}h3v3h3v3h-3v3h-3v-3h2v-3h-2Z','#FF813F')
    dot(x+5,y-5,'#FDE9BA',2);dot(x-6,y+5,'#F5B967',2)
    el('</g>')
# selection must be pure target (not result!) : four turquoise corners.
x,y=center(TARGET)
el('<g id="target-G5" shape-rendering="crispEdges">')
for sx,sy in [(-1,-1),(1,-1),(-1,1),(1,1)]:
    bx=x+(-7 if sx<0 else 4);by=y+(-7 if sy<0 else 4)
    rect(bx,by,4,2,'#FDF3B3');rect(bx,by,2,4,'#79F8F3')
rect(x-1,y-1,3,3,'#79F8F3');el('</g>')
# RIGHT - no overdraw on 10x10 board; illustrated copper and cloth controls.
rect(205,46,115,194,'#2D263A');rect(207,48,111,189,'#69453F')
# navigation strip top / hairline to keep hierarchy.
rect(210,52,106,1,'#DCA871')
# FEU active: dominates with 35px high / 105px wide
rect(210,56,107,38,'#342638',3);rect(212,58,103,34,'#F8C776',2);rect(214,60,99,30,'url(#fire)',2)
rect(216,61,95,2,'#FFD58A');rect(216,88,95,2,'#7F332D')
# cannon icon left x219..237, no firing trajectory on sea
rect(220,71,12,5,'#3C3440');rect(222,69,12,4,'#E1B970');rect(226,76,2,5,'#3C3440');rect(221,80,15,2,'#42313F')
dot(228,81,'#E1B970',2);dot(218,68,'#FBD77C',2)
txt(260,80,'FEU',18.5,'#FDF6CE',800,0,extra='text-anchor="middle"')
txt(294,87,'G5',7.5,'#FEEDB7',800)
# defense fabric/copper
rect(210,98,107,31,'#332D3A',3);rect(212,100,103,27,'#F7C67F',2);rect(214,102,99,23,'url(#def)',2)
path('M222 106h8v8l-4 4-4-4Z','#F1D39A');path('M225 109h2v6h-2Z','#153A50')
txt(234,118,'DÉFENSE',11.3,'#F8F5D3',800)
# unread marker is ! only, not a number
rect(302,101,12,12,'#692E40',2);rect(305,103,2,5,'#FFDE9A');dot(305,109,'#FFDE9A',2)
# abandonment subordinate
rect(210,132,107,21,'#382C3C',2);rect(212,134,103,17,'#BF956B',2);rect(213,135,101,15,'#765A54',2)
rect(221,137,1,10,'#F2DDB5');path('M223 138h8v5h-8Z','#EEE3BC')
txt(235,146,'ABANDON',9,'#FFF3C8',800)
# history: copper parchment but compact; text >=7px
rect(210,156,107,34,'#463240',2);rect(212,158,103,30,'#E4BC86',1)
rect(213,159,101,8,'#8A5B4F');txt(217,166,'TIRS RÉCENTS',7.2,'#FDF4D4',800,0.15)
txt(217,176,'E7  TOUCHÉ',8,'#682F3F',800)
txt(217,185,'C2  RATÉ',8,'#173F53',800)
# last riposte is a separate neutral slate, not coupled to red alert
rect(210,193,107,18,'#18364E',2);rect(212,195,103,14,'#DDEAD5',1)
txt(215,201,'RIPOSTE :',6.5,'#174156',800)
txt(215,207,'B7  RATÉ',7.8,'#174156',800)
# historical gravity unread should remain separate, no counter
rect(210,214,107,20,'#482638',2);rect(212,216,103,16,'#9B3C43',1)
path('M215 228L219 219L223 228Z','#F7CE78');rect(218,221,1,4,'#623442');dot(218,226,'#623442')
txt(226,227,'NON LUE : COULÉ',7.2,'#FFF4CE',800)
# crisp corners / hand-crafted timber seams; no overloaded overlays
rect(205,235,115,5,'#3A2938')
el('</svg>')
svg='\n'.join(pieces)+'\n'
(HERE/'g04_r2_attaque.svg').write_text(svg,encoding='utf-8')
out=HERE/'g04_r2_attaque_320x240.png'
cairosvg.svg2png(bytestring=svg.encode(),write_to=str(out),output_width=320,output_height=240)
image=Image.open(out).convert('RGBA')
assert image.size==(320,240)
zoom=image.resize((1280,960),Image.Resampling.NEAREST)
zoom.save(HERE/'g04_r2_attaque_x4.png',optimize=True)
assert [zoom.getpixel((j,0)) for j in range(4)]==[image.getpixel((0,0))]*4
# validate true nearest by checking one tile block and the inverse every 4px
assert zoom.resize((320,240),Image.Resampling.NEAREST).tobytes()==image.tobytes()
report={
 'status':'DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED',
 'dimensions_native':[320,240], 'dimensions_zoom':[1280,960],
 'grid':{'origin':[GX,GY], 'cell':CELL, 'rows':10,'columns':10,'width':180,'height':180},
 'selected':TARGET,'selected_not_fired': TARGET not in MISSES+HITS,
 'misses':MISSES,'hits':HITS,'enemy_ships_visible':False,
 'riposte_latest':'B7 RATÉ','unread_max_severity':'COULÉ',
 'buttons':['FEU','DÉFENSE','ABANDON'],'styles':'sunset sea / timber / bronze',
 'font':'DejaVu Sans Condensed (host), not approved for firmware',
 'files':{}
}
for filename in ['g04_r2_attaque.svg','g04_r2_attaque_320x240.png','g04_r2_attaque_x4.png']:
    p=HERE/filename
    report['files'][filename]={'sha256':hashlib.sha256(p.read_bytes()).hexdigest(), 'bytes':p.stat().st_size}
(HERE/'generation-report.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(json.dumps(report,indent=2,ensure_ascii=False))
