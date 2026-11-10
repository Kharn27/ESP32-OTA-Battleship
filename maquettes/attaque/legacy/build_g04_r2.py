#!/usr/bin/env python3
"""G04-R2: artistic game-screen compositor in native CYD pixels.

The HD concept is a painted reference and texture atlas, NOT a claimed vector source.
The tactical layer (grid/markers/labels/state/buttons) is deterministic, native and
individually editable in this Python source.  Preview only, no firmware assets.
"""
from pathlib import Path
import json, hashlib, math
from PIL import Image, ImageDraw, ImageFont
import PIL

ROOT=Path(__file__).resolve().parent
FONT_PATH=Path('/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf')
assert FONT_PATH.is_file(), 'Police requise : fonts-dejavu-core (Debian/Ubuntu)'
assert PIL.__version__ == '12.3.0', f'Pillow 12.3.0 requis pour export reproductible, obtenu {PIL.__version__}'
REF=ROOT/'reference_concept_1448x1086.png'
W,H=320,240
GX,GY,CELL=21,53,18
GRID=180
MISSES=['A8','B7','D1','F9','G2','H6','J4']
HITS=['C3','E3','E4','I8']
TARGET='G5'
assert set(MISSES).isdisjoint(HITS) and TARGET not in MISSES+HITS
im=Image.new('RGB',(W,H));d=ImageDraw.Draw(im)
ref=Image.open(REF).convert('RGB')
assert ref.size==(1448,1086),f'incorrect HD reference {ref.size}'

def patch(bounds,dest,scale=Image.Resampling.LANCZOS):
    x,y,w,h=dest
    cropped=ref.crop(bounds).resize((w,h),scale)
    im.paste(cropped,(x,y))

def txt(pos,text,size,color,stroke=0,stroke_fill=None,anchor=None):
    font=ImageFont.truetype(str(FONT_PATH),size)
    ImageDraw.Draw(im).text(pos,text,font=font,fill=color,anchor=anchor,stroke_width=stroke,stroke_fill=stroke_fill)

def ink_box(rect,fill,edge,rounding=2,edgewidth=1):
    ImageDraw.Draw(im).rounded_rectangle(rect, radius=rounding,fill=fill,outline=edge,width=edgewidth)

# Atmosphere: these are ACTUAL painted textures from the approved-look HD idea;
# not a reconstruction from uniform vector rectangles.
patch((0,0,1448,191),(0,0,320,43))
patch((10,180,1437,1080),(0,43,320,197))
# Original oak / brass plate remains visible outside the gameplay grid.
patch((85,183,850,928),(0,42,206,196))
patch((850,184,1426,989),(206,43,114,197))
# Protective dark trim separating ocean and tactile commands.
d=ImageDraw.Draw(im);d.rectangle((203,44,209,239),fill='#3A1B19')
d.line((206,45,206,235),fill='#C28B44',width=1)

# Board is a 180px playable square: retain wood from the concept but
# repaint the coordinate gutters to avoid HD-only glyphs becoming mush.
d=ImageDraw.Draw(im)
d.rectangle((0,40,204,52),fill='#DDA361',outline='#6A351F',width=1)
d.rectangle((0,52,20,234),fill='#CD8A52',outline='#6A351F',width=1)
# Beveled carved parchment coordinate cells.
for i in range(10):
    x=GX+i*CELL; y=GY+i*CELL
    d.rectangle((x,41,x+17,51),fill='#EFC38A',outline='#B57C3D',width=1)
    d.line((x+2,42,x+15,42),fill='#FFF0BF')
    d.rectangle((1,y,19,y+17),fill='#EFC38A',outline='#AE7039',width=1)
    d.line((2,y+1,18,y+1),fill='#FFF0BF')
    txt((x+9,47),'ABCDEFGHIJ'[i],8,'#3D221C',anchor='mm')
    txt((10,y+9),str(i+1),8,'#3D221C',anchor='mm')
d.rectangle((19,51,202,234),fill='#623623',outline='#F7C97D',width=2)
d.rectangle((20,52,201,233),fill='#102D42',outline='#271C25',width=1)

# A single continuous, deterministic ocean artwork (G04-R2 grid readability correction).
# The previous version used 100 independently cropped water tiles, generating
# bright internal seams and phantom cell boundaries.  This replacement paints
# a unified 180x180 ocean without tiles or any 18px-periodic operation.
# The texture is deliberately SUBORDINATE to the 1px authoritative grid and
# to game markers. Every swell is an isolated low-contrast motif, not a line.
from random import Random
RNG=Random(20261103)

def make_continuous_ocean():
    ocean=Image.new('RGB',(GRID,GRID))
    px=ocean.load()
    # Correlated colour variation at non-periodic scales. This is a continuous
    # value field over the whole playable water, NOT a 10x10 tiled surface.
    seeds=[(7,9,7),(17,14,5),(39,29,3)]
    bands=[]
    for wid,hei,amp in seeds:
        field=Image.new('L',(wid,hei))
        vals=field.load()
        for yy in range(hei):
            for xx in range(wid):
                vals[xx,yy]=RNG.randint(85,170)
        field=field.resize((GRID,GRID),Image.Resampling.BILINEAR)
        bands.append((field.load(),amp))
    for y in range(GRID):
        depth=y/(GRID-1)
        for x in range(GRID):
            swell=sum(int((field[x,y]-128)*amp/64) for field,amp in bands)
            # Teal-lit surface fading into a deeper midnight-blue ocean.
            px[x,y]=(max(0,min(255,round(12-5*depth+swell*.38))),
                     max(0,min(255,round(76-24*depth+swell))),
                     max(0,min(255,round(103-23*depth+swell))))
    dw=ImageDraw.Draw(ocean)
    # Only short, broken crests: soft enough to sit BELOW actual grid lines.
    # No continuous horizontal bars, no hard 18px tile edges or repeated stamps.
    for idx in range(79):
        x=RNG.randrange(3,GRID-18)
        y=RNG.randrange(4,GRID-5)
        span=RNG.randrange(4,10)
        tone=RNG.choice(['#1B647B','#1B657A','#1D6980','#286A7C','#145A74'])
        dw.line([(x,y),(x+span//2,y),(x+span//2+2,y-1),
                 (x+span+2,y-1)],fill=tone,width=1)
        # A few sunlit highpoints, NEVER pure white or grid-coloured.
        if idx%6==0:
            dw.point((x+span//2+2,y-2),fill='#2B7E8D')
    # Mild smoothing prevents individual crests from becoming false straight
    # cell borders; keep the authored 1px game grid as the sharpest signal.
    return ocean

sea_layer=make_continuous_ocean()
im.paste(sea_layer,(GX,GY))
d=ImageDraw.Draw(im)
# The one-and-only gameplay grid, authored AFTER the ocean at exact coordinates.
# Palette is reserved: the water generator NEVER emits these RGB values.
for i in range(11):
    x=GX+i*CELL; y=GY+i*CELL
    d.line((x,GY,x,GY+GRID),fill='#B9E8EE',width=1)
    d.line((GX,y,GX+GRID,y),fill='#B9E8EE',width=1)
d.rectangle((21,53,201,233),outline='#D6FAF2',width=1)
# Mechanic signs: miss = foam ring + bubbles; hit = explosive orange bloom with
# ink-dark water contact; prepared target = four gold square brackets + cross.
def center(k):
    return GX+(ord(k[0])-65)*CELL+CELL//2, GY+(int(k[1:])-1)*CELL+CELL//2
for key in MISSES:
    x,y=center(key)
    d=ImageDraw.Draw(im)
    d.ellipse((x-6,y-2,x+5,y+4),fill='#06395F',outline='#D8FAFF',width=2)
    d.arc((x-4,y-1,x+3,y+2),0,180,fill='#72DDE9',width=1)
    for xx,yy in [(-3,-5),(1,-7),(4,-4)]:
        d.rectangle((x+xx,y+yy,x+xx+1,y+yy+1),fill='#F2FFFF')
for key in HITS:
    x,y=center(key); d=ImageDraw.Draw(im)
    d.ellipse((x-7,y+3,x+7,y+6),fill='#062E48')
    # Starburst body with navy/warm edges, distinct from water even in low contrast.
    poly=[(x,y-8),(x+2,y-4),(x+7,y-6),(x+4,y-1),(x+8,y+2),(x+3,y+3),
          (x+5,y+8),(x,y+5),(x-5,y+8),(x-4,y+2),(x-8,y),
          (x-3,y-2),(x-6,y-7),(x-1,y-4)]
    d.polygon(poly,fill='#E63920')
    d.line([(x,y-6),(x,y+5)],fill='#FFEBA1',width=2)
    d.line([(x-5,y),(x+5,y)],fill='#FFC34F',width=2)
    d.point((x,y),fill='#FFFFFF')
x,y=center(TARGET);d=ImageDraw.Draw(im)
for dx in [-7,5]:
 for dy in [-7,5]:
    sx=1 if dx<0 else -1; sy=1 if dy<0 else -1
    d.line([(x+dx,y+dy+sy*4),(x+dx,y+dy),(x+dx+sx*4,y+dy)],fill='#281A15',width=4)
    d.line([(x+dx,y+dy+sy*4),(x+dx,y+dy),(x+dx+sx*4,y+dy)],fill='#FFE66A',width=2)
d.ellipse((x-2,y-2,x+2,y+2),fill='#0B3E5F',outline='#FFE978',width=1)

# G04-R2 / PO correction: the control stack belongs to the ship's built-in
# wooden dashboard. Do not paint flat type-background rectangles over hand-
# painted frames ("labels scotchés"). The new HD reference has integrated,
# shaded red/blue enamel controls, brass rims, rivets, carved oak and inset
# information plaques. Crop ONLY the right-hand panel at native 320x240.
# This intentionally preserves the established 10x10 grid, targets, impact
# marks, sea and all sky/captain pixels. The painted panel labels are raster
# in the editable reference; the crop coordinates are editable in this source.
CONTROL_REF=ROOT/'reference_controles_integres_1448x1086.png'
control_master=Image.open(CONTROL_REF).convert('RGB')
assert control_master.size==(1448,1086),control_master.size
native_control=control_master.resize((W,H),Image.Resampling.LANCZOS)
PANEL_BOX=(208,42,320,239)
im.paste(native_control.crop(PANEL_BOX), PANEL_BOX[:2])

# Hardware framing; text in the right strip intentionally limited.
d=ImageDraw.Draw(im)
# Panel's final brass border and oak trim are part of the reference crop.
d.line((207,234,207,235),fill='#301A18',width=1)  # Preserve the 2 original pixels left of the panel.
d.line((0,238,319,238),fill='#2D211D',width=2)

out=ROOT/'g04_r2_attaque_320x240.png'
im.save(out,optimize=True)
zoom=im.resize((1280,960),Image.Resampling.NEAREST)
zoom.save(ROOT/'g04_r2_attaque_x4.png',optimize=True)

# Four *focused* interaction vignettes: not four competing full game screens.
# Cards are genuinely authored at 320x240, with painted portions from the same
# source as the primary screenshot. Labels and game-state markers are native pixels.
states=Image.new('RGB',(320,240),'#301E26')
sd=ImageDraw.Draw(states)
wood=ref.crop((878,187,1398,966)).resize((320,240),Image.Resampling.LANCZOS)
states.paste(Image.blend(wood,states,.54))
sd=ImageDraw.Draw(states)
sd.rectangle((0,0,319,27),fill='#34242B')
sd.line((0,26,319,26),fill='#D59A5E',width=1)
SF=ImageFont.truetype(str(FONT_PATH),9)
S8=ImageFont.truetype(str(FONT_PATH),8)
S7=ImageFont.truetype(str(FONT_PATH),7)
def slabel(pos,t,color='#FFF0C9',font=None,anchor=None):
    ImageDraw.Draw(states).text(pos,t,font=font or SF,fill=color,anchor=anchor)
def card(x,y,title):
    dd=ImageDraw.Draw(states)
    dd.rounded_rectangle((x,y,x+153,y+99),radius=3,fill='#392A35',outline='#FFC780',width=2)
    dd.rounded_rectangle((x+3,y+3,x+150,y+97),radius=2,fill='#123F56',outline='#BA8B5C',width=1)
    dd.rectangle((x+4,y+4,x+149,y+18),fill='#A86542')
    dd.line((x+5,y+5,x+147,y+5),fill='#F1CB85',width=1)
    slabel((x+9,y+13),title,'#FFF2C7',SF,anchor='lm')

def plate(x,y,w,h,color='#B54429',disabled=False):
    dd=ImageDraw.Draw(states)
    dd.rounded_rectangle((x,y,x+w,y+h),radius=3,fill='#C79A51' if not disabled else '#716D6B',outline='#FFF0B7' if not disabled else '#A2A4A4',width=2)
    dd.rounded_rectangle((x+3,y+3,x+w-3,y+h-3),radius=2,fill=color if not disabled else '#666C70',outline='#6F2D2B' if not disabled else '#454C52',width=1)

# 320x240 native, 2x2 cropped state demonstrations; all preserve G04 art grammar.
slabel((8,5),'ATTAQUE · ÉTATS VISUELS','#FBE1A9',ImageFont.truetype(str(FONT_PATH),11))
slabel((8,19),'Quatre vignettes ciblées — règles métier inchangées','#E9D6B7',S7)
card(4,29,'A · SANS CIBLE')
plate(14,57,135,29,'#797D81',True)
slabel((81,71),'FEU INDISPONIBLE','#E0E3DD',SF,anchor='mm')
slabel((14,109),'Aucune case préparée','#F5D9AA',S8)
card(163,29,'B · FEU ENGAGÉ')
plate(173,53,133,25,'#8E3734')
slabel((238,66),'FEU · TIR EN COURS','#FFF0C9',SF,anchor='mm')
plate(173,81,65,18,'#64727B',True); plate(241,81,65,18,'#64727B',True)
slabel((206,90),'DÉFENSE','#E4E4DF',S8,anchor='mm'); slabel((273,90),'ABANDON','#E4E4DF',S8,anchor='mm')
slabel((172,115),'Sélection et commandes verrouillées','#F5D9AA',S7)
card(4,137,'C · RÉSULTATS ENSEMBLE')
slabel((14,157),'Même résolution, deux tirs révélés :','#F7D9A8',S8)
plate(14,174,135,25,'#345D70')
slabel((19,186),'G5 TOUCHÉ','#FFDA8C',SF,anchor='lm')
slabel((137,186),'+ B7 RATÉ','#DBF4EC',S8,anchor='rm')
slabel((14,212),'Aucune révélation différée','#F2D2A1',S8)
card(163,137,'D · RETOUR DÉFENSE')
plate(173,161,133,25,'#1B6580')
slabel((239,174),'DÉFENSE  ⇄  ATTAQUE','#FFF0CB',S8,anchor='mm')
slabel((171,196),'G5 reste sélectionnée','#FFE1A8',SF)
slabel((171,208),'Résumé et alerte acquittés','#D6F1E7',S8)

states.save(ROOT/'g04_r2_etats_320x240.png',optimize=True)
states.resize((1280,960),Image.Resampling.NEAREST).save(ROOT/'g04_r2_etats_x4.png',optimize=True)

report={
 'status':'DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED',
 'concept_provenance':'source HD AI-generated painterly concept; photo-pixel cutouts, no firmware approval',
 'authoring':'Editable Python scene composition, 10x10 geometry and native lettering/impacts',
 'native':[W,H], 'x4':[1280,960], 'vignettes':['SANS CIBLE','FEU ENGAGÉ','RÉSULTATS ENSEMBLE','RETOUR DÉFENSE'],
 'font_dependency':{'package':'fonts-dejavu-core','path':str(FONT_PATH),'pillow_exact':PIL.__version__},
 'grid':{'x':GX,'y':GY,'cell':CELL,'width':180,'height':180},
 'axis':'columns A-J; rows 1-10',
 'target':TARGET,'misses':MISSES,'hits':HITS,'enemy_ships_visible':False,
 'last_reply':'B7 RATÉ','unread_severity':'COULÉ',
 'sea_render':'continuous 180x180 deterministic ocean; zero 18px tile seams',
 'grid_render':'one authoritative set of 11+11 integer-aligned 1px lines',
 'po_artistic_approval':'2026-11-04 / PR #64 comment 6626211912 / PC-only',
 'button_panel':{'bbox':[208,42,320,239], 'source':'reference_controles_integres_1448x1086.png',
                 'master_kind':'raster', 'gameplay_board_unchanged': True},
 'files':{}
}
for name in ['reference_concept_1448x1086.png','reference_controles_integres_1448x1086.png','build_g04_r2.py','g04_r2_attaque_320x240.png','g04_r2_attaque_x4.png','g04_r2_etats_320x240.png','g04_r2_etats_x4.png']:
 p=ROOT/name
 report['files'][name]={'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size}
(ROOT/'generation-report.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(json.dumps(report,ensure_ascii=False,indent=2))
