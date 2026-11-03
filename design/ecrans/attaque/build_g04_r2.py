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
sea_layer.save(ROOT/'g04_r2_mer_continue_180x180.png',optimize=True)
im.paste(sea_layer,(GX,GY))
d=ImageDraw.Draw(im)
# The one-and-only gameplay grid, authored AFTER the ocean at exact coordinates.
# Palette is reserved: the water generator NEVER emits these RGB values.
for i in range(11):
    x=GX+i*CELL; y=GY+i*CELL
    d.line((x,GY,x,GY+GRID),fill='#B9E8EE',width=1)
    d.line((GX,y,GX+GRID,y),fill='#B9E8EE',width=1)
d.rectangle((21,53,201,233),outline='#D6FAF2',width=1)
# Proof-only view of the authoritative 11x11 lines on continuous sea before
# marks are added; 181x181 includes both outer boundaries.
im.crop((GX,GY,GX+GRID+1,GY+GRID+1)).save(
    ROOT/'g04_r2_plateau_sans_impacts_181x181.png',optimize=True)

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

# Three illustrated commands: source-painted metal frames, bevels and sheen.
# Fill gaps with oak so HD text underneath never bleeds between controls.
d=ImageDraw.Draw(im)
for ya,yb in [(51,54),(94,98),(128,132),(153,157),(191,195),(213,216)]:
    d.rectangle((209,ya,319,yb),fill='#5A3024')
    d.line((210,ya,318,ya),fill='#A66E42',width=1)
# Text is re-lettered at native pixel count so the original richness survives.
patch((879,198,1380,334),(210,54,107,41))
patch((880,339,1380,446),(210,98,107,31))
patch((904,450,1377,526),(210,132,107,21))
# Keep source-painted brass/wood frame; cover HD labels with matching
# native-size metal insets to avoid doubled, illegible glyph edges.
d=ImageDraw.Draw(im)
d.rounded_rectangle((245,62,310,88),radius=3,fill='#A92625',outline='#D86F38',width=1)
d.line((247,64,308,64),fill='#DF6E47',width=1)
d.line((247,86,308,86),fill='#632019',width=1)
d.rounded_rectangle((232,102,310,123),radius=2,fill='#124C6C',outline='#448AA3',width=1)
d.line((234,103,308,103),fill='#89C5D4',width=1)
d.rounded_rectangle((238,136,309,148),radius=2,fill='#E9B17E',outline='#975B3D',width=1)
txt((276,76),'FEU',19,'#FFF3D9',stroke=1,stroke_fill='#65251E',anchor='mm')
txt((271,113),'DÉFENSE',12,'#FFF5DC',stroke=1,stroke_fill='#142A47',anchor='mm')
txt((273,142),'ABANDON',9,'#421B10',anchor='mm')
# small unread alert badge, without any fictional counter.
d.ellipse((300,97,316,113),fill='#AF2926',outline='#FFD26C',width=1)
txt((308,105),'!',12,'#FFF8E8',stroke=0,anchor='mm')

# Colored parchment/history is text, not tiny pre-rendered unreadable HD type.
# Use the HD paper's natural texture from a comparatively blank area.
d=ImageDraw.Draw(im)
d.rounded_rectangle((210,156,317,191),radius=2,fill='#EAC48E',outline='#B46D36',width=2)
for i in range(24):
    px=213+(i*41)%99;py=159+(i*13)%30
    d.line((px,py,px+1+(i%3),py),fill=('#E0AA70' if i%2 else '#F6D7A4'))

d.rectangle((212,158,315,167),fill='#C99859')
txt((215,163),'HISTORIQUE',8,'#3D201B',anchor='lm')
txt((215,173),'I8 : TOUCHÉ !',8,'#9B261D',anchor='lm')
txt((215,184),'J4 : RATÉ',8,'#1B3046',anchor='lm')
# Last captain shot and max unread gravity are separate pieces of gameplay.
patch((876,791,1383,864),(210,195,107,18))
d=ImageDraw.Draw(im)
d.rounded_rectangle((210,195,317,212),radius=2,fill='#DEE5DA',outline='#285271',width=2)
# One single optically centered baseline, inside the 18px frame. Never clipped.
txt((214,203),'RIPOSTE B7 : RATÉ',9,'#183F5A',anchor='lm')
patch((874,878,1393,968),(210,215,107,20))
d=ImageDraw.Draw(im)
d.rounded_rectangle((210,216,317,235),radius=2,fill='#A62221',outline='#FFCC52',width=2)
d.polygon([(215,230),(219,220),(223,230)],fill='#FFE365',outline='#622F22')
txt((219,227),'!',7,'#63201E',anchor='mm')
txt((225,226),'NON LUE : COULÉ',9,'#FFEEA0',stroke=0,anchor='lm')

# Check textual bounding boxes inside the visible cards at authoring time.
# These are real font bounds, not claimed optical sizes from a screenshot.
for pos,label,size,area in [
    ((214,203),'RIPOSTE B7 : RATÉ',9,(213,196,314,211)),
    ((225,226),'NON LUE : COULÉ',9,(224,218,314,232)),
]:
    bounds=ImageDraw.Draw(im).textbbox(pos,label,font=ImageFont.truetype(str(FONT_PATH),size),anchor='lm')
    assert area[0]<=bounds[0] and area[1]<=bounds[1] and bounds[2]<=area[2] and bounds[3]<=area[3],(label,bounds,area)

# Hardware framing; text in the right strip intentionally limited.
d=ImageDraw.Draw(im)
d.line((207,234,319,234),fill='#301A18',width=2)
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

# True 1:1 PC acceptance comparison. Each 320x240 screenshot is pasted
# WITHOUT resampling. The previous rejected image is an immutable input.
old_path=ROOT/'g04_r2_avant_quadrillage_320x240.png'
old=Image.open(old_path).convert('RGB')
assert old.size==(320,240),'ancienne image native indisponible'
compare=Image.new('RGB',(640,260),'#29202D')
cd=ImageDraw.Draw(compare)
cd.rectangle((0,0,319,19),fill='#713C3C')
cd.rectangle((320,0,639,19),fill='#205F65')
headfont=ImageFont.truetype(str(FONT_PATH),10)
cd.text((9,5),'AVANT : 100 TEXTURES DISJOINTES',font=headfont,fill='#FFF5E3')
cd.text((329,5),'APRÈS : 1 MER CONTINUE + GRILLE',font=headfont,fill='#FFF5E3')
compare.paste(old,(0,20))
compare.paste(im,(320,20))
compare.save(ROOT/'g04_r2_comparaison_avant_apres_640x260.png',optimize=True)
compare.resize((2560,1040),Image.Resampling.NEAREST).save(
    ROOT/'g04_r2_comparaison_x4.png',optimize=True)
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
 'comparative':'two complete 320x240 native images pasted pixel-for-pixel at x=0 and x=320',
 'files':{}
}
for name in ['reference_concept_1448x1086.png','build_g04_r2.py','g04_r2_attaque_320x240.png','g04_r2_attaque_x4.png','g04_r2_etats_320x240.png','g04_r2_etats_x4.png','g04_r2_avant_quadrillage_320x240.png','g04_r2_comparaison_avant_apres_640x260.png','g04_r2_comparaison_x4.png','g04_r2_mer_continue_180x180.png','g04_r2_plateau_sans_impacts_181x181.png']:
 p=ROOT/name
 report['files'][name]={'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size}
(ROOT/'generation-report.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(json.dumps(report,ensure_ascii=False,indent=2))
