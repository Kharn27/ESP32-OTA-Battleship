#!/usr/bin/env python3
"""GRAPH-ACC-001 — reproducible editable SVG and 320x240 pixel-art exports.
No external artwork; SVG shapes and vectorized bitmap glyphs generated locally.
"""
from PIL import Image, ImageDraw, ImageFont
import cairosvg
from pathlib import Path
import random, math, html

OUT=Path(__file__).resolve().parent
random.seed(626)
svg=['<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="0 0 320 240" width="320" height="240" shape-rendering="crispEdges">',
'<title>CYD WARSHIP — accueil LOT 1 — GRAPH-ACC-001</title>',
'<desc>Concept original pixel art. Sunset, foreground friendly ship fires at damaged horizon enemy; enemy misses by splashing nearby. Two action overlays.</desc>']

def group(id): svg.append(f'<g id="{id}">')
def end(): svg.append('</g>')
def rect(x,y,w,h,c,opacity=None):
 if w<=0 or h<=0: return
 op='' if opacity is None else f' opacity="{opacity}"'
 svg.append(f'<rect x="{int(x)}" y="{int(y)}" width="{int(w)}" height="{int(h)}" fill="{c}"{op}/>')
def poly(points,c):
 s=' '.join(f'{int(x)},{int(y)}' for x,y in points)
 svg.append(f'<polygon points="{s}" fill="{c}"/>')
def line(x1,y1,x2,y2,color,width=1):
 svg.append(f'<path d="M{int(x1)} {int(y1)}L{int(x2)} {int(y2)}" stroke="{color}" stroke-width="{width}" fill="none"/>')
def circle(cx,cy,r,c,step=1):
 for yy in range(-r,r+1,step):
  xx=int(math.sqrt(max(0,r*r-yy*yy)))
  rect(cx-xx,cy+yy,xx*2+1,step,c)

def font_mask(text,size,weight='bold',spacing=0):
 f= '/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf' if weight=='bold' else '/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf'
 font=ImageFont.truetype(f,size)
 w=int(ImageDraw.Draw(Image.new('1',(1,1))).textlength(text,font=font))+8+len(text)*spacing
 im=Image.new('L',(w,size+12),0)
 d=ImageDraw.Draw(im)
 x=0
 for char in text:
  d.text((x,0),char,font=font,fill=255,stroke_width=0)
  x+=round(d.textlength(char,font=font))+spacing
 bbox=im.getbbox()
 return im.crop(bbox) if bbox else im

def bitmap_text(text,x,y,size,color,spacing=0,shadow=None,scale=1):
 im=font_mask(text,size,spacing=spacing)
 im=im.point(lambda p: 255 if p >= 128 else 0)
 def paint(px,py,col):
  # Horizontal runs per row, 1-bit crisp glyph, SVG editable vector paths
  pix=im.load()
  for yy in range(im.height):
   start=None
   for xx in range(im.width+1):
    on=xx<im.width and pix[xx,yy]>0
    if on and start is None: start=xx
    if not on and start is not None:
     rect(px+start*scale,py+yy*scale,(xx-start)*scale,scale,col)
     start=None
 if shadow: paint(x+1,y+2,shadow)
 paint(x,y,color)
 return im.width*scale,im.height*scale

# 1. Sky: reduced discrete palette with 4px atmospheric tiers.
group('sky-color-bands')
stops=[(0,(12,22,43)),(25,(25,33,57)),(50,(55,45,77)),(70,(111,59,87)),(91,(187,86,92)),(109,(238,134,99)),(129,(246,184,120))]
for y in range(0,131,3):
 for (a,ca),(b,cb) in zip(stops,stops[1:]):
  if a<=y<=b:
   k=(y-a)/(b-a); rgb=tuple(round(v+(w-v)*k) for v,w in zip(ca,cb)); break
 rect(0,y,320,3,'#%02x%02x%02x'%rgb)
end()

group('sky-clouds')
# Pixel-art cloud shapes; open sky in title region
poly([(0,50),(24,50),(24,48),(44,48),(44,52),(69,52),(69,55),(43,55),(43,57),(4,57),(4,55),(0,55)],'#333453')
poly([(0,61),(38,61),(38,59),(51,59),(51,62),(74,62),(74,65),(23,65),(23,67),(0,67)],'#5a3a62')
poly([(143,76),(162,76),(162,72),(187,72),(187,74),(213,74),(213,77),(228,77),(228,81),(170,81),(170,84),(143,84)],'#934b6a')
poly([(248,55),(271,55),(271,53),(293,53),(293,56),(320,56),(320,61),(296,61),(296,63),(263,63),(263,61),(248,61)],'#503553')
poly([(253,82),(277,82),(277,79),(301,79),(301,82),(320,82),(320,86),(266,86),(266,88),(253,88)],'#b45b6d')
poly([(0,100),(24,100),(24,97),(49,97),(49,100),(69,100),(69,103),(0,103)],'#d57c7c')
poly([(7,112),(35,112),(35,110),(57,110),(57,112),(79,112),(79,115),(7,115)],'#eea37e')
poly([(272,104),(293,104),(293,101),(318,101),(318,108),(279,108)],'#df8d77')
# Cloud highlight strips
rect(149,76,29,1,'#da827a');rect(171,73,18,1,'#ce7975');rect(263,82,27,1,'#e8a184')
end()

group('sun-and-haze')
# Halo in low-detail pixel rings, partly disappearing into horizon
circle(231,106,37,'#ed9877',step=2)
circle(231,105,31,'#f0ad7f',step=1)
circle(231,105,25,'#ffcf91',step=1)
circle(231,105,19,'#ffe1ab',step=1)
for y,w in [(96,8),(102,5),(109,13),(116,7),(123,20)]: rect(231-w,y,w*2,1,'#ffdea2')
# Atmospheric stripes along low horizon
for x,y,w in [(173,115,26),(272,114,40),(149,121,33),(257,122,42),(19,120,29)]: rect(x,y,w,2,'#f0ae86')
end()

group('horizon-islands')
# Dark shallow islands and haze nearer the horizon
poly([(0,125),(13,121),(34,122),(48,119),(66,122),(89,121),(95,125),(139,125),(161,127),(0,132)],'#775d76')
poly([(253,126),(276,123),(286,125),(310,119),(319,122),(320,134),(253,134)],'#695c75')
rect(0,131,320,3,'#32516a')
rect(0,134,320,2,'#31506b')
end()

# Water from horizon to bottom, muted colors at top -> dark.
group('water-strata')
for y0,y1,c in [(136,145,'#335875'),(145,155,'#2b506b'),(155,166,'#234761'),(166,178,'#1a3e58'),(178,192,'#173750'),(192,205,'#11324b'),(205,222,'#0d2c45'),(222,240,'#0b263d')]: rect(0,y0,320,y1-y0,c)
# Small waves controlled density; bottom-left quiet for the two UI controls
colors=['#54738a','#6d7b89','#4a738a','#b3918d','#dda183','#355c71']
for idx in range(310):
 y=random.randrange(137,239)
 x=random.randrange(0,321)
 if y>170 and x<164: continue
 w=random.choices([2,3,5,7,10],[3,5,5,3,1])[0]
 c=random.choice(colors if y<177 else colors[:3]+['#41677e'])
 rect(x,y,w,1,c)
for x,y,w in [(186,137,15),(211,139,34),(242,142,29),(206,146,24),(253,149,17),(206,158,25),(231,165,19),(270,175,17),(198,185,23),(285,226,29),(165,225,24)]:
 rect(x,y,w,2,'#b18783' if y<150 else '#50768a')
end()

# Enemy vessel to the left of the sun
# High silhouette near horizon, not an in-game hidden fleet.
group('distant-damaged-enemy')
# smoke stacked dark pixels rising from the burning deck
for x,y,w,h,c in [
(102,96,8,6,'#5b445e'),(96,90,12,8,'#54415b'),(99,81,12,10,'#57405c'),
(91,74,14,9,'#463c55'),(95,66,9,7,'#3a354d'),(89,61,8,6,'#38354c'),
(111,99,6,5,'#6f4b5e'),(110,90,5,5,'#65465b')]: rect(x,y,w,h,c)
# ember accents smoke
rect(105,102,2,3,'#f7b46f');rect(110,107,3,4,'#fce5a1')
# enemy silhouette from x70-135, y122-140
poly([(67,127),(137,127),(134,133),(123,139),(84,138),(72,133)],'#171f38')
rect(78,124,50,3,'#25314a'); rect(83,121,42,3,'#25263d')
rect(94,117,23,5,'#293047');rect(100,110,12,7,'#292d46')
rect(102,105,5,6,'#2c3148');rect(105,100,2,7,'#212b42')
rect(91,120,7,2,'#7b5557');rect(116,121,12,2,'#b66b65')
# second small gun and mast
rect(117,118,13,2,'#25273d');rect(89,117,2,5,'#242d45');rect(88,116,9,2,'#30344b')
# enemy on-board fire visibly damaged
poly([(96,122),(100,112),(103,119),(108,107),(111,122)],'#f6a35e')
poly([(101,122),(104,114),(107,122)],'#ffe0a0')
rect(112,124,3,2,'#e86952')
# Reflection / foam beneath target
for x,y,w in [(82,141,21),(91,144,27),(75,147,16),(110,148,13)]:rect(x,y,w,1,'#6a7b85')
end()

# Main ship: placed lower right with bow facing left, readable modern military silhouette.
group('main-ship')
# hull shadow under waterline
poly([(199,204),(310,202),(320,203),(320,221),(241,222),(213,216),(203,211)],'#0a1a30')
# prominent steel-blue hull
poly([(201,190),(211,187),(230,187),(266,184),(320,188),(320,205),(313,212),(219,213),(208,207),(204,204)],'#273a52')
poly([(202,191),(212,190),(262,189),(320,192),(320,196),(216,195),(207,198)],'#576276')
poly([(205,198),(220,197),(320,197),(320,201),(214,202),(210,201)],'#344d65')
# bow stepped angled, crisp tip
poly([(201,190),(211,190),(218,199),(220,207),(204,204)],'#435970')
line(208,190,320,189,'#87909a',1)
# waterline and wake
rect(201,210,118,2,'#101f37');rect(231,213,89,1,'#5a7183')
# superstructure background
rect(237,173,60,14,'#253750');rect(252,161,35,12,'#3d4c66')
rect(260,154,28,8,'#43536b');rect(267,147,15,7,'#3a4a63')
rect(272,140,2,9,'#a19b99');rect(274,132,2,18,'#313f58')
rect(271,135,6,2,'#cba991');rect(272,144,9,2,'#8192a0')
# radar and antenna
rect(258,153,23,2,'#222d44');rect(269,129,2,11,'#2f354b');rect(265,137,12,2,'#586777')
rect(266,136,8,1,'#e4ab89')
# wheelhouse windows
for x in [260,267,274,281]: rect(x,164,4,3,'#b8a79d')
rect(258,170,30,2,'#1d2c43')
# funnels smoke stack
rect(294,155,11,19,'#273249');rect(295,152,8,4,'#121b31');rect(297,157,2,15,'#617081')
# radar fencing and rivets
for x in range(239,314,10): rect(x,181,5,1,'#8c92a0')
for x in [224,238,252,266,280,295,309]: rect(x,200,4,2,'#a1a6a6')
# Turret foredeck, pointing to upper left
poly([(210,174),(232,174),(239,179),(238,186),(205,186),(202,181)],'#1e2d44')
poly([(208,175),(229,175),(236,179),(209,179)],'#7c8392')
rect(204,179,32,2,'#90939c')
# double naval guns angled upper-left
line(208,176,184,156,'#1a283e',4)
line(215,176,191,156,'#293950',4)
line(208,175,185,154,'#9aa0a2',1)
line(215,175,191,154,'#d0aaa0',1)
# second battery
rect(303,176,16,10,'#28364d');rect(310,175,10,2,'#7b8192')
line(306,176,291,164,'#222c44',3)
# deck lights
rect(241,177,4,2,'#f9d19e');rect(251,177,3,2,'#c2bab0')
# painted emblem
rect(270,203,10,1,'#b6b1a8');rect(274,201,2,5,'#b6b1a8')
# wake by bow
for x,y,w in [(162,207,24),(178,212,22),(196,216,35),(233,222,31),(266,224,25),(300,219,18)]:rect(x,y,w,2,'#55788a')
for x,y,w in [(172,213,13),(205,220,19),(244,226,15),(287,229,22)]:rect(x,y,w,1,'#91a19d')
end()

# ballistics as clear different trajectories. Main ship golden shot to enemy, coral return into splash.
group('outgoing-friendly-salvo')
# shell trail from main left-pointing gun x185,154 towards distant warship x115,121
for x,y,w,h,c in [
(181,150,7,5,'#fff2bc'),(176,148,6,4,'#ffc980'),(173,145,4,3,'#f5a46c'),
(166,141,6,3,'#ffdc94'),(159,138,5,3,'#f5ba77'),(151,135,5,3,'#ffd98a'),
(143,131,5,3,'#ef9f62'),(135,128,4,2,'#ffe3a0'),(127,125,5,2,'#f0b078'),
(119,122,4,3,'#ffe09b')]: rect(x,y,w,h,c)
# Bright muzzle flash away from turret, tightly contained
poly([(181,153),(189,148),(191,152),(197,150),(193,156),(198,160),(188,159)],'#ffd78d')
poly([(183,153),(190,151),(189,155),(193,158),(185,158)],'#fff0ba')
end()

group('enemy-return-and-near-miss')
# Return fire is slightly rose-red; descends to sea in FRONT of our own bow.
for x,y,w,h in [(128,133,4,2),(134,137,4,2),(140,143,4,2),(147,149,5,2),(154,156,4,3),(159,162,4,3),(164,168,4,3)]:rect(x,y,w,h,'#fc917b')
rect(167,173,3,3,'#ffcea2')
# tall separated splash centered x170-180; distinct from hull and overlay UI
poly([(162,187),(165,180),(167,175),(170,183),(173,165),(175,180),(180,174),(180,186),(185,191),(181,193),(164,193)],'#a6cbd0')
poly([(165,188),(169,181),(171,187),(175,173),(176,187),(181,181),(180,191)],'#f3ecda')
rect(173,169,2,4,'#ffffff');rect(160,180,3,3,'#bfd9d8');rect(184,182,3,3,'#aac8ca')
rect(158,195,32,2,'#86aeba');rect(164,198,22,1,'#d3d9d3')
rect(155,192,5,1,'#d7e4dd');rect(189,192,7,1,'#b4c9c9')
# Water droplets
for x,y,w in [(160,173,2),(183,170,2),(185,177,2),(157,184,3),(189,183,2),(168,163,2)]:rect(x,y,w,w,'#e2e6dc')
end()

# Crisp UI typography and simple UI overlay. No mass background panel.
group('title-and-rule')
# small title titleplate at upper left, height 27, unobtrusive
rect(13,13,2,23,'#ffc584')
bitmap_text('CYD WARSHIP',20,15,20,'#fff2d4',spacing=0,shadow='#30304a')
rect(20,37,113,1,'#f7b176')
rect(20,40,46,1,'#b36d74')
end()

group('primary-touch-action')
# tap target x=12 y=174 w=151 h=29
rect(12,177,151,29,'#071b30')
rect(14,179,147,25,'#182f45')
rect(14,179,147,2,'#f0af77')
rect(14,202,147,2,'#325673')
rect(17,184,18,16,'#e8a96d')
rect(19,186,14,12,'#9a5d5c')
# triangular play cursor
poly([(24,187),(24,196),(30,192)],'#fff2d6')
bitmap_text('JOUER CONTRE MOUSSE',36,188,10,'#fff3dc',shadow='#0c2035')
end()

group('secondary-touch-action')
# tap target x=12 y=209 w=121 h=24
rect(12,210,123,23,'#071b30')
rect(14,212,119,19,'#22384f')
rect(14,212,119,1,'#86939c')
# minimalist bars icon to suggest score/history
rect(21,221,3,6,'#e3a47d');rect(26,217,3,10,'#e3a47d');rect(31,219,3,8,'#e3a47d')
bitmap_text('PALMARÈS',41,218,11,'#f2e7d3',shadow='#0a1d30')
end()

svg.append('</svg>')
source='\n'.join(svg)+'\n'
(OUT/'accueil_cyd_warship.svg').write_text(source,encoding='utf-8')
path=OUT/'accueil_cyd_warship_320x240.png'
cairosvg.svg2png(bytestring=source.encode('utf-8'),write_to=str(path),output_width=320,output_height=240)
img=Image.open(path).convert('RGB')
# Exact indexed palette (lossless) reduces Flash/storage size for review exports.
colors=sorted(set(img.getdata()))
assert len(colors) <= 256
lookup={c:i for i,c in enumerate(colors)}
indexed=Image.new('P',img.size)
indexed.putpalette([v for rgb in colors for v in rgb]+[0]*(768-3*len(colors)))
indexed.putdata([lookup[c] for c in img.getdata()])
indexed.save(path,optimize=True)
indexed.resize((1280,960),resample=Image.Resampling.NEAREST).save(OUT/'accueil_cyd_warship_x4.png',optimize=True)
print('SVG SVGbytes',len(source.encode('utf-8')),'PNG',path.stat().st_size,'ZOOM',(OUT/'accueil_cyd_warship_x4.png').stat().st_size)
