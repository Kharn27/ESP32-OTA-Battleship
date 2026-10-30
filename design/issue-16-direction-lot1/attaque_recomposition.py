#!/usr/bin/env python3
"""G01 / CYD WARSHIP — ATTAQUE: LA MER EST LE PLATEAU.
Original native-pixel illustrative composition, editable independent named layers.
Usage: python attaque_recomposition.py [output_dir]
Requires Pillow on PC only. All art drawn at 320x240. No embedded or third-party art.
"""
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
from random import Random
from math import sin
import zipfile,io,xml.etree.ElementTree as ET,sys
O=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).parent
O.mkdir(parents=True,exist_ok=True)
W,H=320,240
PAL={'night':'#0A2337','deep':'#07354E','marine':'#0E5069','water':'#126E80','green':'#188E93','turquoise':'#30AFAB','foam':'#CDF5DE','white':'#F6F9D8','gold':'#F8CB72','amber':'#E99453','copper':'#AA593D','darkred':'#612D3C','ink':'#0A1C32','orange':'#F37733','shadow':'#060F22','sel':'#79F8F3'}
FONT_DIR='/usr/share/fonts/truetype/dejavu/'
FONT_BOLD=FONT_DIR+'DejaVuSansCondensed-Bold.ttf'
FONT_REG=FONT_DIR+'DejaVuSansCondensed.ttf'
F={size:ImageFont.truetype(FONT_BOLD,size) for size in [9,10,11,12,13,14,18,19,22,26]}
R={size:ImageFont.truetype(FONT_REG,size) for size in [9,10,11]}

def new():return Image.new('RGBA',(W,H))
def draw(im):return ImageDraw.Draw(im,'RGBA')
def rect(d,xy,fill):d.rectangle(xy,fill=fill)
def poly(d,pts,fill):d.polygon(pts,fill=fill)
def ln(d,p,fill,w=1):d.line(p,fill=fill,width=w)
def txt(d,xy,s,sz=10,color='#F6F9D8',bold=True):
    f=(F if bold else R)[sz]
    d.text((xy[0]+1,xy[1]+1),s,font=f,fill='#061526')
    d.text(xy,s,font=f,fill=color)

def atmosphere():
    im=new(); d=draw(im); rng=Random(290)
    # Full-bleed sunset, not a right-hand decorative inset.
    sky=['#172B44','#29334E','#423A57','#64475E','#965B63','#C87865','#F2A46A']
    for y in range(48):
        base=sky[min(len(sky)-1,(y*len(sky))//48)]
        rect(d,(0,y,319,y),base)
    # Step-edged distant cloud masses, from left to right: one coherent horizon.
    for pts,col in [
      ([(0,8),(17,8),(17,6),(39,6),(39,8),(53,8),(53,12),(69,12),(69,17),(52,17),(52,15),(22,15),(22,18),(0,18)],'#28364F'),
      ([(31,25),(39,25),(39,22),(61,22),(61,20),(89,20),(89,23),(111,23),(111,28),(97,28),(97,31),(47,31),(47,29),(31,29)],'#58475A'),
      ([(191,10),(201,10),(201,7),(223,7),(223,9),(252,9),(252,12),(271,12),(271,17),(248,17),(248,15),(213,15),(213,18),(191,18)],'#344257'),
      ([(234,29),(246,29),(246,25),(264,25),(264,23),(283,23),(283,27),(301,27),(301,30),(319,30),(319,34),(248,34)],'#8A5960'),
    ]:poly(d,pts,col)
    for x,y,w in [(7,19,22),(52,32,35),(108,22,19),(199,19,37),(246,35,30),(285,24,26)]:
        rect(d,(x,y,x+w,y),'#E7A06C');rect(d,(x+4,y+1,x+w-6,y+1),'#AE6C64')
    # Half-submerged blazing sun, partially framed by the visible sea horizon.
    cx,cy=146,40
    for rr,c in [(23,'#CE765C'),(19,'#F4AE69'),(16,'#FFD486'),(12,'#FFE8A4')]:
        for yy in range(-rr,rr+1):
            ww=int((max(0,rr*rr-yy*yy))**.5)
            rect(d,(cx-ww,cy+yy,cx+ww,cy+yy),c)
    # Low distant silhouettes with stepped warm rim, readable as coastline not enemy ships.
    poly(d,[(0,48),(0,43),(24,43),(32,41),(41,41),(45,39),(52,41),(62,41),(62,49)],'#213F53')
    poly(d,[(203,49),(217,45),(225,45),(230,42),(238,42),(243,40),(251,42),(262,43),(274,39),(286,40),(302,36),(319,33),(319,50)],'#243F51')
    for x,y,w in [(2,43,19),(218,45,14),(251,42,12),(289,40,12)]:rect(d,(x,y,x+w,y),'#C18C6B')
    # Water occupies one continuous canvas, with broad irregular shapes + light migration.
    for y in range(48,240):
        t=(y-48)/192
        color=(int(11+2*t),int(80+15*t),int(102+4*t))
        if y>123:color=(int(8+11*t),int(82+11*t),int(96+3*t))
        rect(d,(0,y,319,y),color)
    # Long broken bands extend through grid and scenic part without resetting per cell.
    for i in range(138):
        y=rng.randrange(50,238);x=rng.randrange(-16,318)
        w=rng.choice([8,13,17,24,34,46,61]); h=rng.choice([1,1,1,2,2,3])
        c=rng.choice(['#125A6E','#14697A','#187B84','#0D4862','#1D8189','#248E8F'])
        if x<185 and rng.random()<.24:c='#13758A'
        poly(d,[(x,y),(x+w//2,y),(x+w//2+4,y+1),(x+w,y+1),(x+w-4,y+h),(x+4,y+h)],c)
    # Sunlight: discontinuous lit currents on the sea, not a beam or ray.
    for i in range(145):
        y=rng.randrange(51,225)
        span=6+(y-50)//7
        x=146+rng.randrange(-span,span+1)
        w=rng.choice([2,3,5,7,10,12])
        c=rng.choice(['#F5C073','#E49E67','#F8D38A','#B79D7A','#56AD9C'])
        if rng.random()<.57:
            rect(d,(x,y,x+w,y),c)
    # A few hand-clustered wave crests; 100 tactical tiles share the same water.
    for x,y,scale in [(4,77,1),(46,102,1),(139,91,1),(177,118,2),(64,144,1),
                      (101,167,2),(14,201,1),(73,220,2),(188,198,1),(244,169,2),
                      (280,204,2),(220,224,1),(257,93,1)]:
        ww=10+scale*5
        ln(d,[(x,y+2),(x+3,y),(x+ww//2,y),(x+ww//2+4,y+2),(x+ww,y+2)],'#59BAAD')
        ln(d,[(x+2,y+3),(x+ww-5,y+3)],'#124E6A')
        rect(d,(x+5,y+1,x+8,y+1),'#CBDBC2')
    # Dithery organic foam and glints; no mechanical per-cell texture.
    for i in range(190):
        x=rng.randrange(0,320);y=rng.randrange(53,239)
        if rng.random() < .25:
            c='#66C9B4'
        else:c=rng.choice(['#3E9A98','#1E8A8D','#286F80','#2A9C9C'])
        w=rng.choice([1,2,3,5,7]);rect(d,(x,y,x+w,y),c)
    # Far lighthouse silhouette belongs to landscape; tiny dot of gold in upper right.
    poly(d,[(301,46),(305,24),(310,24),(313,45)],'#142D43')
    rect(d,(303,22,312,23),'#27374D');rect(d,(304,19,310,21),'#FFD88A')
    poly(d,[(302,19),(307,15),(313,19)],'#1A3045')
    rect(d,(306,29,308,31),'#F8CD88')
    for x,y in [(222,32),(240,22),(279,18)]:
        ln(d,[(x-3,y),(x-1,y-1),(x+1,y),(x+3,y-1)],'#2A3B53')
    return im

def ship():
    im=new();d=draw(im)
    # Decorative foreground vessel on sea OUTSIDE the tactical grid. No enemy identity or positions.
    # Big bow and deck, all water contact and shadows integrated into the environment.
    poly(d,[(217,130),(236,130),(247,126),(267,124),(304,127),(319,132),(319,144),(311,154),(254,158),(234,151),(222,142)],'#081D32')
    poly(d,[(221,136),(245,135),(256,132),(305,133),(319,137),(312,149),(257,152),(240,146)],'#213A4A')
    poly(d,[(226,138),(249,137),(264,136),(301,137),(315,140),(304,146),(260,147),(238,142)],'#3E4C50')
    # Planked edge and brass sheer line; stairs not sleek anti-aliased vector.
    ln(d,[(218,129),(246,128),(259,124),(303,125),(319,132)],'#CC8851',2)
    ln(d,[(227,134),(255,134),(287,131),(314,134)],'#F1BB6A')
    ln(d,[(239,147),(264,151),(310,149)],'#AA684B')
    for x,y in [(248,143),(263,145),(278,145),(295,144)]:
        rect(d,(x,y,x+3,y+2),'#081D32');rect(d,(x+1,y,x+2,y),'#E6AC69')
    # Wheelhouse and rigging: layered navy volumes, smokestack, pennant.
    poly(d,[(265,125),(268,108),(291,108),(297,125)],'#162D3F')
    rect(d,(269,106,291,109),'#B07B55')
    rect(d,(273,98,285,105),'#284255')
    rect(d,(276,92,283,98),'#1B3248')
    rect(d,(275,90,285,91),'#DFA260')
    for x in [270,279,287]:rect(d,(x,112,x+5,116),'#E7B36A');rect(d,(x+1,113,x+4,115),'#34546B')
    # Mast + two guy lines, no drawn ballistic trajectory.
    rect(d,(297,65,299,124),'#17263C')
    rect(d,(297,66,298,123),'#D29A5C')
    poly(d,[(299,69),(316,71),(309,77),(299,76)],'#D96B4B')
    ln(d,[(297,77),(285,113)],'#394556')
    ln(d,[(299,79),(313,130)],'#334556')
    # Forward turret + barrel.
    poly(d,[(230,130),(231,122),(246,122),(251,131)],'#1B3041')
    rect(d,(231,123,245,124),'#C9975B')
    poly(d,[(232,122),(222,112),(225,110),(242,122)],'#1B2E42')
    ln(d,[(222,111),(237,122)],'#D5A46B')
    # plume (from stack) dissipates into sky above ship.
    for x,y,w in [(281,86,7),(278,80,9),(273,74,11),(270,69,6)]:
        poly(d,[(x,y),(x+2,y-2),(x+w,y-2),(x+w+2,y),(x+w,y+3),(x+2,y+3)],(61,69,78,170))
        rect(d,(x+1,y,x+w-1,y),'#6A6D73')
    # Wake wraps hull with layered foamy curves, arises FROM waterline.
    poly(d,[(216,143),(221,144),(230,151),(242,154),(260,159),(299,159),(319,154),(319,159),(297,165),(260,164),(239,159),(222,151)],'#267A84')
    ln(d,[(211,144),(217,145),(227,153),(242,158),(258,161)],'#C9F3D7',2)
    ln(d,[(260,163),(288,165),(307,163),(319,160)],'#75D0BD',2)
    for x,y in [(214,151),(225,157),(239,162),(258,166),(287,167),(308,166),(313,157)]:
        rect(d,(x,y,x+5,y),'#E4F7D7');rect(d,(x+2,y-2,x+3,y-1),'#8EDCC3')
    return im

def missed(d,x,y,flip=0):
    # Sea-anchored water column and outward white ring, organic but bounded in ONE tile.
    ln(d,[(x-7,y+4),(x-4,y+3),(x-2,y+4),(x+2,y+4),(x+6,y+3)],'#105872',2)
    ln(d,[(x-6,y+3),(x-3,y+2),(x+4,y+2),(x+7,y+3)],'#9CDEC6')
    rect(d,(x-2,y-4,x+1,y+2),'#8CD6D1')
    rect(d,(x-1,y-6,x,y-1),'#E3F9EA')
    for dx,dy in [(-5,-3),(-4,-6),(3,-5),(6,-2),(-2,-8)]:
        rect(d,(x+dx,y+dy,x+dx+(flip==1),y+dy+1),'#EAFCEE')
    rect(d,(x+3,y+1,x+5,y+1),'#F7F9DF')

def hit(d,x,y):
    # Hot strike, asymmetrical flame plumes + scorched water ring; never a ship silhouette.
    poly(d,[(x-6,y+4),(x-5,y+1),(x-3,y),(x+4,y),(x+7,y+3),(x+5,y+5),(x-3,y+5)],'#522D43')
    rect(d,(x-5,y+2,x+6,y+3),'#F28645')
    for pts,c in [
        ([(x-4,y+1),(x-5,y-3),(x-2,y-4),(x-1,y-7),(x+1,y-4),(x+3,y-6),(x+4,y-3),(x+5,y+1)],'#EF573D'),
        ([(x-3,y+2),(x-2,y-2),(x,y-4),(x+2,y-1),(x+3,y+2)],'#FFC260'),
        ([(x-1,y+2),(x,y-2),(x+2,y+2)],'#FFF2AC')]:poly(d,pts,c)
    for dx,dy in [(-7,-2),(7,-4),(-5,-7),(4,-8),(7,2)]:rect(d,(x+dx,y+dy,x+dx+1,y+dy+1),'#FFB354')
    ln(d,[(x-7,y+5),(x-3,y+6)],'#61B8B2')
    ln(d,[(x+3,y+6),(x+7,y+5)],'#6EC6AD')

def grid():
    im=new();d=draw(im)
    gx,gy,cell=23,49,18
    # Optical marine map over living ocean. NO panel, no isolating background rectangle.
    for xx in range(11):
        x=gx+xx*cell
        ln(d,[(x,gy),(x,gy+180)],(129,219,203,175),1)
    for yy in range(11):
        y=gy+yy*cell
        ln(d,[(gx,y),(gx+180,y)],(129,219,203,170),1)
    # Gentle ocean-chart tick marks: no thick grid panel frame.
    for x,y in [(23,49),(203,49),(23,229),(203,229)]:
        rect(d,(x-1,y-1,x+1,y+1),'#F8C982')
    # Headers over the panorama, with bright relief.
    for col in range(10):
        ch=chr(65+col);w=d.textbbox((0,0),ch,font=F[10])[2]
        txt(d,(gx+col*cell+(18-w)//2,36),ch,10,'#FBE3AC')
    for row in range(10):
        s=str(row+1);w=d.textbbox((0,0),s,font=F[10])[2]
        txt(d,(20-w,gy+row*cell+2),s,10,'#FBE3AC')
    misses=[('B',1),('D',3),('F',1),('J',9),('A',8),('F',6),('H',4),('B',10)]
    hits=[('C',4),('E',6),('A',5),('C',8),('I',7)]
    for i,(c,r) in enumerate(misses):
        missed(d,gx+(ord(c)-65)*cell+9,gy+(r-1)*cell+9,i%2)
    for c,r in hits:
        hit(d,gx+(ord(c)-65)*cell+9,gy+(r-1)*cell+9)
    # selection G5, strongest aqua contrast but no paint-over of ocean or impact.
    sx=gx+6*cell;sy=gy+4*cell
    rect(d,(sx+1,sy+1,sx+16,sy+16),(10,48,63,100))
    for px,py,dx,dy in [(sx+1,sy+1,1,1),(sx+16,sy+1,-1,1),(sx+1,sy+16,1,-1),(sx+16,sy+16,-1,-1)]:
        ln(d,[(px,py),(px+dx*5,py)],'#F8FAE5',2)
        ln(d,[(px,py),(px,py+dy*5)],'#7AFBF4',2)
    rect(d,(sx+8,sy+8,sx+9,sy+9),'#F7FBE4')
    return im

def ui():
    im=new();d=draw(im)
    # ATTAQUE label on a sculpted floating maritime pennant, sky remains behind.
    poly(d,[(0,0),(124,0),(133,8),(129,28),(116,33),(0,31)],'#122C42')
    poly(d,[(0,0),(119,0),(128,7),(124,26),(115,29),(0,28)],'#6B4148')
    ln(d,[(0,28),(111,28),(121,24)],'#F5BB6F',2)
    rect(d,(0,0,121,1),'#E59D63')
    # Faint compass rose icon.
    ln(d,[(14,9),(14,23)],'#F7D6A1')
    ln(d,[(7,16),(21,16)],'#F7D6A1')
    poly(d,[(14,8),(17,16),(14,14),(11,16)],'#F3BD66')
    txt(d,(26,7),'ATTAQUE',14,'#FFF4CC')
    # DEFENSE command: ship's signal pennant/ribbon, not the old full-width info dashboard.
    poly(d,[(156,0),(319,0),(319,43),(306,45),(158,39),(151,31),(153,7)],'#0B263B')
    poly(d,[(161,2),(319,2),(319,40),(302,42),(161,36),(156,31)],'#15364A')
    ln(d,[(158,37),(303,43),(319,40)],'#E5A568')
    rect(d,(163,2,318,3),'#DBA166')
    # Shield at upper left, title and latest outcome.
    poly(d,[(165,9),(171,7),(177,9),(176,18),(171,23),(166,18)],'#F1BE72')
    poly(d,[(168,11),(171,10),(174,11),(173,17),(171,19),(169,17)],'#356679')
    txt(d,(182,4),'DÉFENSE',12,'#FAEAC4')
    txt(d,(164,24),'DERN. E5 TOUCHÉ',10,'#D6F6E6')
    # Distinct, persistent severity at right. Never a counter.
    rect(d,(263,6,316,36),'#74323E')
    rect(d,(264,7,315,7),'#FFA964')
    txt(d,(267,8),'NON LU',9,'#FFD9A3')
    txt(d,(266,18),'COULÉ',11,'#FFF6CF')
    # Separate focal area for target and firing, flowing into decorative ship/wake.
    poly(d,[(210,169),(319,166),(319,191),(210,192),(204,184)],'#091D32')
    ln(d,[(213,170),(319,168)],'#C48A58',2)
    txt(d,(218,169),'CIBLE  G5',13,'#FBE4B1')
    # FEU: actual hero action framed as copper-painted naval control, not a generic HUD tile.
    poly(d,[(214,187),(311,187),(319,194),(319,230),(308,238),(212,238),(205,230),(205,196)],'#361F2D')
    poly(d,[(214,190),(311,190),(316,194),(316,229),(309,235),(212,235),(208,229),(208,197)],'#B97845')
    poly(d,[(216,193),(309,193),(312,196),(312,228),(306,232),(214,232),(211,227),(211,198)],'#F47F3B')
    poly(d,[(215,195),(309,195),(311,198),(311,204),(213,204),(213,199)],'#FFB45A')
    rect(d,(214,228,308,230),'#B74335')
    # Two mechanical rivets and an expressive stylized cannon muzzle.
    for x,y in [(214,195),(306,195),(214,228),(306,228)]:
        rect(d,(x,y,x+2,y+2),'#F8D591')
        rect(d,(x+1,y+1,x+1,y+1),'#784137')
    poly(d,[(218,211),(228,208),(230,215),(220,218)],'#583B3C')
    rect(d,(221,213,229,214),'#FAD28F')
    txt(d,(239,193),'FEU',26,'#FFF7D7')
    # tiny horizon/reflection intrudes around command to keep it belonging to the scene
    for x,y in [(206,239),(225,239),(274,239),(294,239)]:rect(d,(x,y,x+12,y),'#57B3AC')
    return im

def data(im):
    b=io.BytesIO();im.save(b,format='PNG',optimize=True);return b.getvalue()

def ora(layers,result,path):
    root=ET.Element('image',{'w':str(W),'h':str(H),'name':'CYD WARSHIP G01 - ATTAQUE recomposition'});st=ET.SubElement(root,'stack')
    with zipfile.ZipFile(path,'w') as z:
        z.writestr('mimetype','image/openraster',compress_type=zipfile.ZIP_STORED)
        for i,(name,img) in enumerate(reversed(layers)):
            uri=f'data/layer{i}.png';z.writestr(uri,data(img))
            ET.SubElement(st,'layer',{'name':name,'src':uri,'opacity':'1.0','visibility':'visible','composite-op':'svg:src-over','x':'0','y':'0'})
        z.writestr('mergedimage.png',data(result));z.writestr('stack.xml',ET.tostring(root))
        z.writestr('Thumbnails/thumbnail.png',data(result))
    with zipfile.ZipFile(path) as z:
        assert Image.open(io.BytesIO(z.read('mergedimage.png'))).convert('RGB').tobytes()==result.convert('RGB').tobytes()

if __name__=='__main__':
    layers=[('01 | Horizon et mer continus',atmosphere()),('02 | Navire de premier plan',ship()),('03 | Grille, touches, ratés, cible',grid()),('04 | Bannières tactiles, FEU, DÉFENSE',ui())]
    out=new()
    for _,i in layers:out=Image.alpha_composite(out,i)
    rgb=out.convert('RGB')
    rgb.save(O/'attaque_recomposition_320x240.png',optimize=True)
    big=rgb.resize((1280,960),Image.Resampling.NEAREST)
    big.save(O/'attaque_recomposition_x4.png',optimize=True)
    ora(layers,rgb,O/'attaque_recomposition_layers.ora')
    assert rgb.size==(320,240) and big.size==(1280,960)
    assert big.tobytes()==rgb.resize((1280,960),Image.Resampling.NEAREST).tobytes()
    print('PASS: 320x240, ×4 nearest-neighbor exact; 4 layered ORA; composition matches; no asset runtime verified')
