#!/usr/bin/env python3
"""G04-R2: artistic game-screen compositor in native CYD pixels.

The HD concept is a painted reference and texture atlas, NOT a claimed vector source.
The tactical layer (grid/markers/labels/state/buttons) is deterministic, native and
individually editable in this Python source.  Preview only, no firmware assets.
"""
from pathlib import Path
import json, hashlib, math
from PIL import Image, ImageDraw, ImageFont

ROOT=Path(__file__).resolve().parent
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
    font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf',size)
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

# Reuse isolated clean water cells as native sea textures. Unlike a downscale
# of the whole HD art, the native field contains no embedded phantom markers.
x0,y0,x1,y1=154,248,830,900
cw=(x1-x0)/10; ch=(y1-y0)/10
orig_marked=[('A',8),('B',7),('D',1),('F',9),('G',2),('H',6),('J',4),('C',3),('E',3),('E',4),('I',8),('G',5)]
marked={(ord(r)-65,c-1) for r,c in orig_marked}
safe=[(rr,cc) for rr in range(10) for cc in range(10)
      if all(max(abs(rr-mr),abs(cc-mc))>=2 for mr,mc in marked)]
# Tiles with cleaner ocean and no baked symbols; 4px inset discards HD grid.
for yrow in range(10):
    for xcol in range(10):
        rr,cc=safe[(xcol*29+yrow*17)%len(safe)]
        box=(int(x0+cc*cw+5),int(y0+rr*ch+5),
             int(x0+(cc+1)*cw-5),int(y0+(rr+1)*ch-5))
        tex=ref.crop(box).resize((CELL,CELL),Image.Resampling.LANCZOS)
        sea_tint=Image.new('RGB',(CELL,CELL),'#073C5E')
        tex=Image.blend(tex,sea_tint,.14)
        im.paste(tex,(GX+xcol*CELL,GY+yrow*CELL))
d=ImageDraw.Draw(im)
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
txt((214,203),'RIPOSTE : B7',8,'#183F5A',anchor='lm')
txt((214,209),'RATÉ',7,'#183F5A',anchor='lm')
patch((874,878,1393,968),(210,215,107,20))
d=ImageDraw.Draw(im)
d.rounded_rectangle((210,216,317,235),radius=2,fill='#A62221',outline='#FFCC52',width=2)
d.polygon([(215,230),(219,220),(223,230)],fill='#FFE365',outline='#622F22')
txt((219,227),'!',7,'#63201E',anchor='mm')
txt((225,226),'NON LUE : COULÉ',7,'#FFEEA0',stroke=0,anchor='lm')

# Hardware framing; text in the right strip intentionally limited.
d=ImageDraw.Draw(im)
d.line((207,234,319,234),fill='#301A18',width=2)
d.line((0,238,319,238),fill='#2D211D',width=2)

out=ROOT/'g04_r2_attaque_320x240.png'
im.save(out,optimize=True)
zoom=im.resize((1280,960),Image.Resampling.NEAREST)
zoom.save(ROOT/'g04_r2_attaque_x4.png',optimize=True)
report={
 'status':'DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED',
 'concept_provenance':'source HD AI-generated painterly concept; photo-pixel cutouts, no firmware approval',
 'authoring':'Editable Python scene composition, 10x10 geometry and native lettering/impacts',
 'native':[W,H], 'x4':[1280,960],
 'grid':{'x':GX,'y':GY,'cell':CELL,'width':180,'height':180},
 'axis':'columns A-J; rows 1-10',
 'target':TARGET,'misses':MISSES,'hits':HITS,'enemy_ships_visible':False,
 'last_reply':'B7 RATÉ','unread_severity':'COULÉ',
 'files':{}
}
for name in ['reference_concept_1448x1086.png','build_g04_r2.py','g04_r2_attaque_320x240.png','g04_r2_attaque_x4.png']:
 p=ROOT/name
 report['files'][name]={'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size}
(ROOT/'generation-report.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(json.dumps(report,ensure_ascii=False,indent=2))
