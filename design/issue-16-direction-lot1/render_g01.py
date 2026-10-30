#!/usr/bin/env python3
"""CYD WARSHIP G01 — editable pixel-native concept source.

Run: python render_g01.py [output_directory]
Dependency: Pillow (design tooling only; not firmware).
The hand-authored geometry is intentionally rendered directly at 320x240.
All x4 outputs are generated from the native PNG with nearest-neighbor.
"""
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
import hashlib, sys, math

OUT = Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parent
OUT.mkdir(exist_ok=True,parents=True)
W,H=320,240
P={
 'navy':'#081E31','navy2':'#0C2D45','deep':'#102C45','slate':'#224E67',
 'sea':'#0C5070','sea2':'#156686','foam':'#C3E8EF','white':'#F3F1DC',
 'sun':'#F6BE68','brass':'#D59C50','shadow':'#07111F',
 'orange':'#E97935','orange2':'#FFA75D','red':'#ED6650',
 'cyan':'#5BDCF0','cyan2':'#1C92B8','silver':'#B6CAD2',
 'sub':'#8BA6AD','green':'#73D0A8','amber':'#FFCA65'
}
FONT='/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf'
FONT_REG='/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf'
def f(size=10,bold=True):return ImageFont.truetype(FONT if bold else FONT_REG,size)

def t(d,x,y,s,size=10,c='white',bold=True,anchor=None):
 d.text((x,y),s,font=f(size,bold),fill=P.get(c,c),anchor=anchor,stroke_width=0)

def box(d,xy,fill,edge=None):
 d.rectangle(xy,fill=P.get(fill,fill),outline=P.get(edge,edge) if edge else None)
def hl(d,x1,y1,x2,y2,col):d.line([(x1,y1),(x2,y2)],fill=P.get(col,col),width=1)
def panel(d,rect,fill='navy2',selected=False):
 x1,y1,x2,y2=rect
 box(d,(x1,y1,x2,y2),'shadow')
 box(d,(x1+1,y1+1,x2-1,y2-1),fill)
 hl(d,x1+2,y1+1,x2-2,y1+1,'brass' if selected else 'slate')
 hl(d,x1+1,y1+2,x1+1,y2-2,'brass' if selected else 'slate')
 hl(d,x1+2,y2-1,x2-2,y2-1,'shadow')
 for xx,yy in [(x1,y1),(x2-2,y1),(x1,y2-2),(x2-2,y2-2)]:
  box(d,(xx,yy,xx+2,yy+2),'brass' if selected else 'slate')

def badge(d,x,y,color='orange'):
 box(d,(x,y,x+10,y+9),'shadow');box(d,(x+1,y+1,x+9,y+8),color)

def tab(d,x,y,w,label,active=False,warning=False):
 panel(d,(x,y,x+w-2,y+27),'orange' if active else 'navy2',active)
 if active:
  box(d,(x+4,y+4,x+7,y+22),'orange2')
 textlen=d.textbbox((0,0),label,font=f(11))[2]
 t(d,x+(w-textlen)//2,y+6,label,11,'white' if active else 'silver')
 if warning:
  # NOT an event counter; severity is indicated by red corner only on ATTAQUE
  d.polygon([(x+w-15,y+1),(x+w-3,y+1),(x+w-3,y+13)],fill=P['red'])
  box(d,(x+w-8,y+4,x+w-7,y+9),'white')

def background(im,variant):
 d=ImageDraw.Draw(im)
 box(d,(0,0,319,239),'navy')
 # restrained sunset; only outside the readable grid and main controls
 for y in range(35,239):
  color=('#1C3E57' if y<66 else '#21556B' if y<89 else '#0C435D' if y<130 else '#07334B')
  hl(d,209,y,319,y,color)
 d.ellipse((275,45,307,77),fill=P['sun'])
 for y in range(78,116,5):
  hl(d,266+(y%11),y,314-(y%7),y,'orange' if y<90 else 'sea2')
 # cliff & lighthouse in distant right margin
 d.polygon([(276,113),(284,99),(291,105),(300,93),(319,107),(319,148),(277,143)],fill='#102F42')
 box(d,(300,84,305,105),'navy');box(d,(299,81,306,85),'brass');box(d,(301,77,304,82),'navy')
 box(d,(302,89,303,91),'sun')
 # wave details along the bottom edge, deliberately behind the panel edges
 for y in (222,227,232,237):
  for x in range(207+(y%4),318,23):
   hl(d,x,y,x+7,y,'sea2')
   hl(d,x+9,y+1,x+12,y+1,'foam')
 # selection of low-frequency texture in the game grid; no photographic overlay
 for yy in range(48,229):
  for xx in range(22,202):
   v=(xx*37+yy*19+(xx//7)*53+(yy//6)*31)%163
   if v<5:
    d.point((xx,yy),fill='#176082')
   elif v==5:
    d.point((xx,yy),fill='#0A4666')
 # ripples repeated beneath gameplay symbols (under the fleet and grid lines)
 for row in range(10):
  for col in range(10):
   q=(row*19+col*31)%9
   if q in (0,3):
    x,y=22+col*18,48+row*18
    hl(d,x+3,y+12,x+7,y+12,'slate')
    d.point((x+9,y+11),fill=P['sea2'])

def grid(d):
 panel(d,(18,43,206,232),'deep')
 box(d,(22,48,201,227),'sea')
 # Illustrated ocean fills the *actual* cells; quiet enough for silhouettes/targets.
 for yy in range(48,228):
  col=('#145E7B' if yy<83 else '#125A76' if yy<119 else
       '#0D506D' if yy<155 else '#0A4967' if yy<190 else '#0A425F')
  hl(d,23,yy,200,yy,col)
 for row in range(10):
  for col in range(10):
   xx,yy=22+col*18,48+row*18
   phase=(row*13+col*7)%11
   if phase in (0,2,5,7):
    hl(d,xx+3,yy+13,xx+7,yy+13,'slate')
   if phase in (3,6):
    hl(d,xx+9,yy+6,xx+11,yy+6,'sea2')
   if phase==4:
    d.point((xx+5,yy+9),fill=P['cyan2'])
 for i in range(11):
  hl(d,22+i*18,48,22+i*18,228,'slate' if i%5 else 'cyan2')
  hl(d,22,48+i*18,202,48+i*18,'slate' if i%5 else 'cyan2')
 for i,ch in enumerate('ABCDEFGHIJ'):
  t(d,31+i*18,35,ch,9,'foam',anchor='mt')
 for i in range(10):
  t(d,12,52+i*18,str(i+1),9,'foam',anchor='mt')

def cell(c,r):return 22+c*18,48+r*18

def miss(d,c,r):
 x,y=cell(c,r);cx=x+9;cy=y+10
 # spray emerges from sea in same tile; small foam pixels plus anchored ripples
 for dx,dy in [(-6,1),(-4,0),(-2,0),(0,0),(2,0),(4,1),(6,1)]:
  box(d,(cx+dx,cy+dy+3,cx+dx+1,cy+dy+3),'foam')
 for dx,dy in [(-3,-3),(0,-6),(3,-3),(-1,-1),(2,-2)]:
  box(d,(cx+dx,cy+dy,cx+dx+1,cy+dy+1),'foam')
 box(d,(cx-1,cy+1,cx+1,cy+2),'white')

def hit(d,c,r,sunk=False):
 x,y=cell(c,r);cx=x+9;cy=y+9
 # embedded impact splash; legible red-or-orange cross plus sea-ring
 d.line([(cx-5,cy+5),(cx+5,cy-5)],fill=P['red'],width=3)
 d.line([(cx-5,cy-5),(cx+5,cy+5)],fill=P['red'],width=3)
 box(d,(cx-2,cy-2,cx+1,cy+1),'sun')
 for dx,dy in [(-6,6),(6,6),(-6,-6),(6,-6)]:
  d.point((cx+dx,cy+dy),fill=P['foam'])
 if sunk:
  # on own DÉFENSE grid only: sunk hull receives an ashen border
  box(d,(x+2,y+14,x+15,y+15),'brass')

def select(d,c,r):
 x,y=cell(c,r)
 box(d,(x+1,y+1,x+16,y+16),'sea2')
 # bright, unequivocal four-corner reticle, not an artillery trajectory
 for px,py,dx,dy in [(x+1,y+1,1,1),(x+16,y+1,-1,1),(x+1,y+16,1,-1),(x+16,y+16,-1,-1)]:
  hl(d,px,py,px+dx*5,py,'white')
  hl(d,px,py,px,py+dy*5,'cyan')
 box(d,(x+8,y+8,x+9,y+9),'white')

def ship(d,start,length,orientation='h',sunk=False):
 # One continuous hull per vessel, not unrelated metallic sticks per cell.
 x,y=cell(*start)
 L=18*length
 hull='slate' if sunk else 'silver'
 deck='sub' if sunk else 'white'
 if orientation=='h':
  # dark water shadow, multi-tone bow/stern, deckhouses and portholes
  box(d,(x+2,y+14,x+L-3,y+15),'navy')
  d.polygon([(x+1,y+7),(x+5,y+4),(x+L-8,y+4),(x+L-2,y+8),
             (x+L-5,y+13),(x+5,y+13)],fill=P[hull])
  hl(d,x+6,y+5,x+L-9,y+5,deck)
  hl(d,x+5,y+12,x+L-7,y+12,'slate' if not sunk else 'navy')
  mid=x+L//2
  box(d,(mid-6,y+1,mid+4,y+4),'slate')
  box(d,(mid-3,y,mid+1,y+1),deck)
  for px in range(x+9,x+L-9,8): d.point((px,y+10),fill=P['navy2'])
 else:
  box(d,(x+13,y+2,x+14,y+L-3),'navy')
  d.polygon([(x+8,y+1),(x+4,y+6),(x+4,y+L-6),(x+8,y+L-2),
             (x+13,y+L-6),(x+13,y+6)],fill=P[hull])
  hl(d,x+6,y+7,x+6,y+L-8,deck)
  hl(d,x+12,y+7,x+12,y+L-8,'slate' if not sunk else 'navy')
  mid=y+L//2
  box(d,(x+8,mid-6,x+12,mid+5),'slate')
  box(d,(x+9,mid-3,x+11,mid+2),deck)
  for py in range(y+10,y+L-9,8): d.point((x+9,py),fill=P['navy2'])

def render_attack():
 im=Image.new('RGB',(W,H));background(im,'attack');d=ImageDraw.Draw(im)
 tab(d,3,3,100,'ATTAQUE',True)
 tab(d,103,3,100,'DÉFENSE',False,warning=True)
 panel(d,(207,3,317,31),'navy2')
 t(d,214,7,'MANCHE 05',11,'sun')
 t(d,214,18,'À VOUS DE JOUER',9,'white')
 grid(d)
 for c,r in [(1,0),(4,0),(0,3),(7,3),(2,6),(8,7),(5,8),(9,9),(6,0)]:miss(d,c,r)
 for c,r in [(2,2),(3,4),(4,4),(2,5),(7,6)]:hit(d,c,r)
 select(d,6,4)
 panel(d,(209,42,317,100),'navy2')
 t(d,215,48,'CIBLE',9,'sub')
 t(d,216,61,'G5',24,'white')
 t(d,215,87,'Sélection prête',9,'foam',False)
 panel(d,(209,107,317,175),'navy2')
 t(d,215,113,'DERNIER BILAN',9,'sun')
 t(d,215,130,'Croiseur coulé',11,'white')
 t(d,215,146,'Type annoncé',9,'foam',False)
 t(d,215,158,'épave secrète',9,'sub',False)
 # target panel must NEVER reveal which hit cells belong to a sunk enemy ship
 panel(d,(208,181,317,231),'orange',True)
 box(d,(213,185,312,188),'orange2')
 t(d,261,189,'FEU',24,'white',anchor='mt')
 t(d,261,218,'VALIDER LE TIR',9,'white',anchor='mt')
 return im

def render_defense():
 im=Image.new('RGB',(W,H));background(im,'defense');d=ImageDraw.Draw(im)
 tab(d,3,3,100,'ATTAQUE',False)
 tab(d,103,3,100,'DÉFENSE',True)
 panel(d,(207,3,317,31),'navy2')
 t(d,214,7,'MANCHE 05',11,'sun')
 t(d,214,18,'FLOTTE DU JOUEUR',9,'white')
 grid(d)
 # Exactly five ships: lengths 5,4,3,3,2. The own fleet may be seen.
 ship(d,(0,1),5,'h');ship(d,(7,3),4,'v');ship(d,(1,5),3,'v')
 ship(d,(5,8),3,'h');ship(d,(3,3),2,'h',sunk=True)
 # player ship A2 damaged, H5 damaged, D4/E4 sunk, others intact
 for c,r in [(0,1),(7,4)]:hit(d,c,r)
 for c,r in [(3,3),(4,3)]:hit(d,c,r,True)
 for c,r in [(2,0),(5,1),(8,1),(9,5),(4,7),(8,8)]:miss(d,c,r)
 panel(d,(209,42,317,112),'navy2')
 t(d,215,48,'DERNIÈRE RIPOSTE',9,'sun')
 t(d,215,62,'H5 : TOUCHÉ',12,'white')
 t(d,215,79,'Consultée',10,'green')
 t(d,215,94,'Alerte acquittée',9,'sub',False)
 panel(d,(209,118,317,231),'navy2')
 t(d,215,125,'ÉTAT DE LA FLOTTE',9,'sun')
 # Counts are intentionally fully consistent with five visible ships
 for y,num,label,c in [(146,'2','intacts','foam'),(171,'2','endommagés','amber'),(196,'1','coulé','red')]:
  box(d,(214,y+3,222,y+11),c)
  t(d,230,y,num,14,'white')
  t(d,245,y+3,label,9,'white')
 t(d,215,218,'TOTAL : 5 NAVIRES',9,'sub')
 return im

def component_board():
 im=Image.new('RGB',(W,H),P['navy']);d=ImageDraw.Draw(im)
 box(d,(0,0,319,29),'navy2');hl(d,1,28,318,28,'brass')
 t(d,10,5,'CYD WARSHIP  /  G01',14,'white')
 t(d,10,19,'LANGAGE VISUEL  —  PROPOSITION',8,'sun')
 # three state tiles, upper row
 states=[('CIBLE',0),('RATÉ',1),('TOUCHÉ',2)]
 for i,(name,kind) in enumerate(states):
  x=5+i*105
  panel(d,(x,37,x+100,107),'navy2')
  t(d,x+7,42,name,10,'sun')
  box(d,(x+37,57,x+59,79),'sea')
  for k in range(0,23,11):
   hl(d,x+37+k,57,x+37+k,79,'slate')
   hl(d,x+37,57+k,x+59,57+k,'slate')
  # scratch individual graphics for sample, same semantic as screens
  if kind==0:
   for xx,yy,dx,dy in [(x+40,60,1,1),(x+56,60,-1,1),(x+40,77,1,-1),(x+56,77,-1,-1)]:
    hl(d,xx,yy,xx+dx*4,yy,'white');hl(d,xx,yy,xx,yy+dy*4,'cyan')
  elif kind==1:
   d.line((x+43,75,x+54,75),fill=P['foam'],width=2)
   d.line((x+48,69,x+50,62),fill=P['foam'],width=2)
  else:
   d.line((x+42,61,x+55,74),fill=P['red'],width=3)
   d.line((x+55,61,x+42,74),fill=P['red'],width=3)
  t(d,x+7,89,['Inconnue','Écume','Impact'][i],9,'foam',False)
 # common alert severity samples: no counter
 for i,(lab,color) in enumerate([('RATÉ','slate'),('TOUCHÉ','amber'),('COULÉ','red')]):
  x=5+i*105
  panel(d,(x,113,x+100,147),'navy2')
  box(d,(x+4,118,x+8,142),color)
  t(d,x+13,119,lab,10,'white')
  t(d,x+13,133,'non consulté',8,'sub',False)
 t(d,7,153,'ALERTE DÉFENSE : gravité max, sans compteur',9,'foam',False)
 panel(d,(5,169,311,233),'navy2')
 t(d,12,176,'TYPO / BOUTONS / FIN DE PARTIE',10,'sun')
 panel(d,(12,193,79,224),'orange',True);t(d,45,202,'FEU',14,'white',anchor='mt')
 panel(d,(85,193,166,224),'navy2');t(d,125,202,'AUTO',12,'white',anchor='mt')
 panel(d,(173,193,303,224),'navy2');t(d,238,197,'VERDICT',9,'sun',anchor='mt')
 t(d,238,208,'VICTOIRE',12,'white',anchor='mt')
 return im

def save(name,im):
 native=OUT/f'{name}_320x240.png';zoom=OUT/f'{name}_x4.png'
 im.save(native,optimize=True)
 im.resize((1280,960),Image.Resampling.NEAREST).save(zoom,optimize=True)
 check=Image.open(native).convert('RGB');large=Image.open(zoom).convert('RGB')
 assert check.size==(320,240) and large.size==(1280,960)
 assert large.tobytes()==check.resize((1280,960),Image.Resampling.NEAREST).tobytes()
 print(f'{name}  native={native.stat().st_size}B  zoom={zoom.stat().st_size}B  sha256={hashlib.sha256(native.read_bytes()).hexdigest()}')

if __name__=='__main__':
 save('attaque',render_attack());save('defense',render_defense());save('conventions',component_board())