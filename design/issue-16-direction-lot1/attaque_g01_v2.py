#!/usr/bin/env python3
"""CYD WARSHIP G01 exploratory ATTAQUE design preview (only).
All art layers are painted at 320x240; editable source + layered OpenRaster.
No firmware/runtime asset contract implied. MIT-like original code authored for this concept.
Usage: python attaque_g01_v2.py [output_directory]
Dependency: Pillow. Uses host DejaVu fonts for preview; no font binaries bundled.
"""
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
from random import Random
import sys, zipfile, io, xml.etree.ElementTree as ET

W, H = 320, 240
OUT = Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).parent
OUT.mkdir(parents=True, exist_ok=True)
C={
 'navy':'#07182A','night':'#081E32','deep':'#0A2840','panel':'#102F47',
 'blue':'#123F5A','sea':'#0D5771','sea2':'#10738D','teal':'#1C92A3',
 'teal2':'#32ABB6','foam':'#E9F9FF','ice':'#B9E6ED','sel':'#70F4FF',
 'darkgold':'#704029','brass':'#C8833A','gold':'#FFD27D','lightgold':'#FFE6A2',
 'orange':'#F57C35','flame':'#FFB446','scarlet':'#F34D42','reddeep':'#8B2B37',
 'white':'#FFF4D8','muted':'#91B8CB','slate':'#274861','shade':'#061524',
}
FONTS='/usr/share/fonts/truetype/dejavu/'
FONT_REG=FONTS+'DejaVuSansCondensed.ttf'
FONT_BOLD=FONTS+'DejaVuSansCondensed-Bold.ttf'
font={n:ImageFont.truetype(FONT_BOLD,n) for n in (9,10,11,12,13,16,18,22,28)}
reg={n:ImageFont.truetype(FONT_REG,n) for n in (9,10,11,12)}

def layer(): return Image.new('RGBA',(W,H),(0,0,0,0))
def line(d,coords,col,w=1):d.line(coords, fill=col, width=w)
def rect(d,coords,col):d.rectangle(coords,fill=col)
def poly(d,pts,col):d.polygon(pts,fill=col)
def label(d,xy,s,sz=10,color='white',bold=True,shadow=True):
    ft=(font if bold else reg)[sz]; col=C.get(color,color)
    if shadow: d.text((xy[0]+1,xy[1]+1),s,font=ft,fill=C['shade'])
    d.text(xy,s,font=ft,fill=col)
def patchwork(d,x,y,seed,n, colors, xmin=0,xmax=320,ymin=0,ymax=240):
    r=Random(seed)
    for _ in range(n):
        ax=x+r.randrange(-6,7);ay=y+r.randrange(-5,6)
        ax=max(xmin,min(xmax-2,ax)); ay=max(ymin,min(ymax-2,ay))
        pw=r.choice([2,3,4,5,7]);ph=r.choice([1,2,3])
        d.rectangle((ax,ay,min(xmax,ax+pw),min(ymax,ay+ph)),fill=C.get(r.choice(colors),colors[0]))

def background():
    im=layer();d=ImageDraw.Draw(im)
    # Original art: navy framing and cinematic pixel-water/sky vignette on the RIGHT only.
    for y in range(H):
        t=y/(H-1)
        rgb=(int(7+6*t), int(25+35*t),int(42+51*t))
        line(d,[(0,y),(319,y)],rgb)
    # Small controlled sunset, contained in its own scenic gutter (right of the grid).
    x0,x1,y0,y1=207,319,45,139
    for y in range(y0,y1+1):
        a=(y-y0)/(y1-y0)
        if a<.58:
            t=a/.58
            co=(int(31+209*t),int(60+66*t),int(93+22*t))
        else:
            t=(a-.58)/.42
            co=(int(240-203*t),int(126+11*t),int(115+25*t))
        line(d,[(x0,y),(x1,y)],co)
    # cloud silhouettes with stair-stepped illuminated undersides, no blur
    clouds=[[(209,60),(220,60),(220,58),(240,58),(240,60),(253,60),(253,64),(258,64),(258,69),(236,69),(236,67),(209,67)],
            [(271,51),(281,51),(281,49),(295,49),(295,52),(306,52),(306,54),(320,54),(320,61),(293,61),(293,59),(271,59)],
            [(207,79),(221,79),(221,76),(237,76),(242,80),(250,80),(250,84),(206,84)]]
    for q in clouds:poly(d,q,'#785F70')
    for p in [(209,67,231,68),(275,59,308,60),(207,83,239,84)]:rect(d,p,'#FFAE70')
    # Sun with native stepped circle and warm reflected water
    cx,cy=256,100
    for rr,co in [(23,'#F79B55'),(20,'#FFCD75'),(15,'#FFE49A')]:
        for yi in range(-rr,rr+1,2):
            width=int((max(0,rr*rr-yi*yi))**.5)
            rect(d,(cx-width,cy+yi,cx+width,cy+yi+1),co)
    # Horizon and sea depth
    rect(d,(207,116,319,139),'#0B6174')
    line(d,[(207,116),(319,116)],'#FFE1A4')
    rng=Random(44)
    for i in range(96):
        py=rng.randrange(117,138); xx=rng.randrange(207,320)
        width=rng.choice([2,3,5,7,11]); col=rng.choice(['#0D7388','#23889C','#50B9BB','#E3B382'])
        if abs(xx-256)<(24-(py-117)) and rng.random()<.7:col='#F7BD6B'
        rect(d,(xx,py,min(319,xx+width),py),col)
    # Coast: not a visible enemy ship, a lit headland & lighthouse.
    poly(d,[(275,139),(276,129),(281,129),(281,124),(289,124),(289,121),(298,121),(298,114),(304,114),(306,109),(319,105),(319,139)],'#192C41')
    poly(d,[(284,137),(293,129),(301,131),(305,123),(312,126),(319,121),(319,139)],'#3B4456')
    for (x,y) in [(289,127),(305,116),(315,119),(297,135)]:rect(d,(x,y,x+3,y+1),'#C47546')
    # lighthouse silhouette and light chamber - slender, in the far background
    poly(d,[(300,119),(303,89),(309,89),(311,119)],'#142D40')
    rect(d,(302,88,311,89),'#0D2538');rect(d,(303,84,309,87),'#24344B')
    rect(d,(304,85,308,86),'#FFE69F');rect(d,(301,82,312,83),'#0D2538')
    poly(d,[(300,82),(306,76),(313,82)],'#1B293F')
    rect(d,(305,95,307,98),'#FFCD78')
    # seabirds scale 1 pixel to add maritime character
    for x,y in [(235,75),(269,69)]:
        line(d,[(x-4,y),(x-2,y-2),(x,y),(x+2,y-2),(x+4,y)],'#172E48')
    # foreground spray at shore and corner -- not underneath text
    for x,y in [(285,136),(291,137),(310,137),(297,138),(318,134)]:
        rect(d,(x,y,x+4,y),'#E1EFF1')
        rect(d,(x+1,y-2,x+2,y-1),'#8FD6D9')
    return im

def chrome():
    im=layer();d=ImageDraw.Draw(im)
    # subtly ornate bronze perimeter. Deliberate naval plaques instead of plain panels.
    rect(d,(0,0,319,41),'#071B2D')
    line(d,[(0,41),(319,41)],C['brass']);line(d,[(0,42),(319,42)],'#20435B')
    for x in [2,317]:
        rect(d,(x,2,x+1,39),C['brass']);rect(d,(x,222,x+1,237),C['darkgold'])
    # active attack plate x=4..100
    poly(d,[(6,5),(94,5),(100,11),(100,37),(95,40),(5,40),(3,37),(3,11)],'#3B2631')
    poly(d,[(8,6),(93,6),(98,12),(98,35),(92,38),(7,38),(5,35),(5,12)],'#C06B2D')
    rect(d,(8,10,94,34),'#ED8536');line(d,[(9,9),(93,9)],'#FFDD81',2)
    line(d,[(8,36),(94,36)],'#8B3F30',2)
    # hand-built crosshair icon and short title at 11-14 px
    cx,cy=18,22
    d.ellipse((cx-4,cy-4,cx+4,cy+4),outline=C['white'],width=1)
    line(d,[(cx-7,cy),(cx-3,cy)],C['white']);line(d,[(cx+3,cy),(cx+7,cy)],C['white'])
    line(d,[(cx,cy-7),(cx,cy-3)],C['white']);line(d,[(cx,cy+3),(cx,cy+7)],C['white'])
    label(d,(30,13),'ATTAQUE',12,color='white')
    # DÉFENSE is a real single navigation command, includes two independent pieces of information.
    poly(d,[(103,5),(314,5),(318,9),(318,36),(314,40),(104,40),(101,37),(101,9)],'#061524')
    rect(d,(104,7,315,37),'#16364C')
    line(d,[(104,7),(315,7)],'#E6AF60')
    line(d,[(104,38),(315,38)],'#543B35')
    # shield badge
    poly(d,[(110,12),(117,10),(124,12),(123,21),(117,27),(111,22)],'#EAB45E')
    poly(d,[(113,14),(117,13),(121,14),(120,20),(117,23),(114,20)],'#305B71')
    label(d,(127,8),'DÉFENSE',12,'white')
    label(d,(127,23),'Dernier : raté',10,'ice',False)
    line(d,[(221,11),(221,34)],'#427083')
    rect(d,(225,10,313,35),'#632D32')
    line(d,[(226,10),(312,10)],'#F29A60')
    label(d,(226,10),'ALERTE NON LUE',10,'lightgold')
    label(d,(244,20),'COULÉ',12,'white')
    # spare warning triangle (no event counter)
    poly(d,[(230,31),(235,20),(240,31)],'#FF9B4F')
    line(d,[(235,23),(235,27)],'#73353C',2);rect(d,(234,29,236,29),'#73353C')
    # small metallic frame and edge flourishes separate grid from scenic window
    # Scenic field from the editable background layer must remain visible
    line(d,[(206,46),(317,46)],C['gold'])
    line(d,[(206,46),(206,138)],C['brass'])
    line(d,[(317,46),(317,138)],'#583A32')
    for x,y in [(207,47),(316,47),(207,137),(316,137)]:rect(d,(x,y,x+1,y+1),'#F5C17B')
    # vignette must remain visible: no overpainting of center.
    # target panel below scenic area: small, calm, clear
    rect(d,(207,143,317,175),'#071C31')
    line(d,[(207,143),(317,143)],C['gold'],1)
    line(d,[(207,174),(317,174)],C['brass'])
    rect(d,(208,146,209,171),'#C88744')
    label(d,(214,146),'CIBLE CHOISIE',10,'lightgold')
    label(d,(214,155),'G5',18,'white')
    label(d,(252,161),'PRÊTE',10,'ice')
    # FEU is THE main CTA: layered brass bezel, glowing enamel and shading.
    poly(d,[(211,179),(312,179),(318,185),(318,230),(312,236),(210,236),(205,231),(205,185)],'#361D25')
    poly(d,[(213,180),(311,180),(316,185),(316,229),(311,233),(211,233),(208,229),(208,185)],'#E1A04B')
    poly(d,[(214,183),(310,183),(313,187),(313,226),(310,229),(214,229),(211,226),(211,187)],'#AE462D')
    rect(d,(215,186,309,221),'#F07931')
    rect(d,(215,186,309,191),'#FFAB47')
    line(d,[(214,228),(311,228)],'#672A30',2)
    for x in [213,309]:
        rect(d,(x,187,x+1,190),'#FFE7AC')
        rect(d,(x,225,x+1,227),'#D79950')
    # main action label deliberately >20 px
    label(d,(231,193),'FEU',28,'white')
    # FEU label stands alone: avoid cramped explanatory text inside the button.
    # engraved crosshair icon, separated from FEU text
    cx,cy=219,206
    d.ellipse((cx-5,cy-5,cx+5,cy+5),outline='#FFDC88',width=1)
    line(d,[(cx-8,cy),(cx-4,cy)],'#FFE6AA',2)
    line(d,[(cx+4,cy),(cx+8,cy)],'#FFE6AA',2)
    line(d,[(cx,cy-8),(cx,cy-4)],'#FFE6AA',2)
    line(d,[(cx,cy+4),(cx,cy+8)],'#FFE6AA',2)
    # thin foam reflection at bottom right to tie the scene to the frame
    for x,y in [(214,239),(227,238),(257,239),(284,239),(306,238)]: rect(d,(x,y,x+6,y),'#39758A')
    return im

def splash(d,x,y,variant=0):
    # splash shares the tile's perspective and comes from water, NOT a floating icon sticker.
    rect(d,(x-5,y+4,x+5,y+4),'#4CB7CA')
    rect(d,(x-3,y+3,x+3,y+3),'#C6F1F8')
    for dx,dy in [(-4,-1),(-3,-4),(0,-7),(3,-3),(5,0)]:
        rect(d,(x+dx,y+dy,x+dx+(1 if dx==0 else 0),y+dy+1),'#E9FCFF')
    rect(d,(x-1,y,x+1,y+2),'#E0F8FF')
    if variant%2:rect(d,(x+6,y-4,x+6,y-3),'#98E3EA')

def hit(d,x,y):
    # scorched center, glow and sharply pixelated arms: mark stays inside a single cell.
    rect(d,(x-5,y-3,x+5,y+3),'#722F3E')
    for dx,dy in [(-4,-4),(-3,-3),(-2,-2),(-1,-1),(1,1),(2,2),(3,3),(4,4),
                  (-4,4),(-3,3),(-2,2),(-1,1),(1,-1),(2,-2),(3,-3),(4,-4)]:
        rect(d,(x+dx,y+dy,x+dx+1,y+dy+1),'#F9573E')
    rect(d,(x-2,y-2,x+2,y+2),'#F9953A')
    rect(d,(x-1,y-1,x+1,y+1),'#FFF4B2')
    rect(d,(x,y-7,x,y-6),'#FFBC53')
    rect(d,(x+6,y-4,x+6,y-3),'#F99A45')

def board():
    im=layer();d=ImageDraw.Draw(im)
    gx,gy,cell=23,55,18
    # frame: copper side corners, cyan seawater. very little competing animation.
    rect(d,(19,51,204,238),'#08192E')
    rect(d,(21,53,202,236),'#0D3D56')
    line(d,[(19,51),(204,51)],'#DA8B45')
    line(d,[(19,51),(19,238)],'#A46838')
    line(d,[(20,237),(204,237)],'#4EA3AB')
    # headers, readable 10 px with shadow
    for col in range(10):
        x=gx+col*cell+6
        label(d,(x,41),chr(65+col),10,'white',True)
    for row in range(10):
        y=gy+row*cell+3
        v=str(row+1)
        label(d,((7 if row<9 else 1),y),v,10,'white',True)
    # graphical ocean, 100 cells remain flat enough to read a 10x10 game.
    rng=Random(109)
    for row in range(10):
        for col in range(10):
            x=gx+col*cell;y=gy+row*cell
            n=(col*7+row*5+((col*row)%3))%6
            base=['#0D4560','#104F68','#0D4964','#0F4B64','#11516A','#104860'][n]
            rect(d,(x,y,x+17,y+17),base)
            if (row+2*col)%3==0:
                rect(d,(x+3,y+5,x+7,y+5),'#16657C')
                rect(d,(x+9,y+12,x+13,y+12),'#18586D')
            if rng.random()<.28:rect(d,(x+1,y+15,x+4,y+15),'#155C73')
    # grid lines are visible without becoming the scene
    for n in range(11):
        xx=gx+n*cell; yy=gy+n*cell
        line(d,[(xx,gy),(xx,gy+180)],'#3D8595')
        line(d,[(gx,yy),(gx+180,yy)],'#3D8595')
    for x,y in [(21,53),(201,53),(21,235),(201,235)]:
        rect(d,(x,y,x+2,y+2),'#D1A15C')
    # history distinct; no silhouette or grouping of sunk enemy cells.
    misses=[('B',1),('D',3),('F',1),('J',9),('A',8),('F',6),('H',4),('B',10)]
    hits=[('C',4),('E',6),('A',5),('C',8),('I',7)]
    for i,(c,r) in enumerate(misses):
        xx=gx+(ord(c)-65)*cell+9; yy=gy+(r-1)*cell+9
        splash(d,xx,yy,i)
    for c,r in hits:
        xx=gx+(ord(c)-65)*cell+9; yy=gy+(r-1)*cell+9
        hit(d,xx,yy)
    # G5 is selected, in vivid cyan illuminated corners, no intrusive fill covering water.
    sx=gx+6*cell;sy=gy+4*cell
    rect(d,(sx+1,sy+1,sx+16,sy+16),'#19748B')
    rect(d,(sx+3,sy+3,sx+14,sy+14),'#14637C')
    corners=[(sx+1,sy+1,1,1),(sx+16,sy+1,-1,1),(sx+1,sy+16,1,-1),(sx+16,sy+16,-1,-1)]
    for x,y,dx,dy in corners:
        line(d,[(x,y),(x+dx*5,y)],'#E0FDFF',2)
        line(d,[(x,y),(x,y+dy*5)],'#6AF4FF',2)
    # subtle targeting star center
    rect(d,(sx+8,sy+8,sx+9,sy+9),'#E6FDFF')
    return im

def overlay_waves():
    im=layer();d=ImageDraw.Draw(im)
    # environmental decoration only in outer margin, not over cell marks
    for x,y in [(0,235),(20,237),(54,236),(89,238),(151,235),(179,238)]:
        rect(d,(x,y,x+14,y),'#1C697D')
        rect(d,(x+5,y-1,x+9,y-1),'#58A7AC')
    return im

def png_bytes(im):
    f=io.BytesIO();im.save(f,format='PNG',optimize=True);return f.getvalue()

def openraster(layers,final,path):
    stack=ET.Element('image',attrib={'w':str(W),'h':str(H),'name':'CYD WARSHIP - G01 ATTAQUE v2'})
    root=ET.SubElement(stack,'stack')
    # OpenRaster spec stack top-to-bottom
    with zipfile.ZipFile(path,'w') as z:
        z.writestr('mimetype','image/openraster',compress_type=zipfile.ZIP_STORED)
        z.writestr('mergedimage.png',png_bytes(final))
        for i,(title,img) in enumerate(reversed(layers)):
            src=f'data/layer{i:02d}.png'
            z.writestr(src,png_bytes(img))
            ET.SubElement(root,'layer',{'name':title,'src':src,'opacity':'1.0','visibility':'visible','composite-op':'svg:src-over','x':'0','y':'0'})
        z.writestr('stack.xml',ET.tostring(stack,encoding='utf-8',xml_declaration=True))

layers=[('Ambiance - coucher de soleil et phare',background()),('Mer tactique - cases, historique, selection',board()),('Navigation et commandes - texte, plaques, FEU',chrome()),('Ecume en marge',overlay_waves())]
final=Image.new('RGBA',(W,H))
for name,layerimg in layers:final=Image.alpha_composite(final,layerimg)
native=final.convert('RGB')
native_path=OUT/'attaque_expression_320x240.png'
zoom_path=OUT/'attaque_expression_x4.png'
ora_path=OUT/'attaque_expression_layers.ora'
native.save(native_path)
zoom=native.resize((W*4,H*4),Image.Resampling.NEAREST)
zoom.save(zoom_path)
openraster(layers,final,ora_path)
assert native.size==(320,240)
assert zoom.size==(1280,960)
assert zoom.tobytes()==native.resize((1280,960),Image.Resampling.NEAREST).tobytes()
with zipfile.ZipFile(ora_path) as z:
  assert len([n for n in z.namelist() if n.startswith('data/layer')])==4
  assert Image.open(io.BytesIO(z.read('mergedimage.png'))).convert('RGB').tobytes()==native.tobytes()
print('PASS G01 DESIGN_PREVIEW 320x240, nearest x4, 4 layered ORA; ON_DEVICE_UNVERIFIED')
for p in (native_path,zoom_path,ora_path):print(p.name,p.stat().st_size)

