#!/usr/bin/env python3
"""FLOT-01 R3-A: hand-authored 54x18 submarine and separate water wake.

This is a CANDIDATE, not an approved firmware asset. The ship is drawn using
opaque individual pixel strokes on transparent RGBA, not extracted from a sea
background or resized from the comparison poster. Python 3.11+, Pillow 12.3.0.
Usage: python3 build_r3a.py [--check]
"""
from __future__ import annotations
import argparse
import hashlib
import math
import random
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
SHIP_SIZE=(54,18)
WAKE_SIZE=(66,22)
WAKE_ORIGIN=(10,0)  # where 54x18 submarine overlays the 66x22 wake effect
SEED=11303

def rgba(hexcode: str) -> tuple[int,int,int,int]:
    return tuple(bytes.fromhex(hexcode.removeprefix('#'))) + (255,)

def paint_ship() -> Image.Image:
    im=Image.new('RGBA', SHIP_SIZE, (0,0,0,0)); pix=im.load()
    rng=random.Random(SEED)
    hull_rows={8:(10,44),9:(7,48),10:(5,50),11:(3,51),12:(2,53),13:(3,52),14:(5,50),15:(8,48),16:(11,43)}
    metal=['263b43','697b7d','718386','7f9293','687c7e','4d6368','374e57','2b414b','24333b']
    for y,(left,right) in hull_rows.items():
        for x in range(left,right+1):
            edge = x in (left,right) or (y>8 and (x<left+1 or x>right-1))
            if edge: pix[x,y]=rgba('16282e' if y>9 else '253039'); continue
            q = math.sin(x*0.65+y*0.9)*0.5 + math.cos(x*1.43-y)*0.23 + rng.uniform(-0.5,0.5)
            base = metal[y-8]
            rgb=tuple(bytes.fromhex(base))
            # Multi-value modeling: edge-to-center reflection, textured yet legible at 1x.
            delta=int(q*7 + (2 if 18<x<45 else -2))
            if y == 10 and 11<=x<=46: delta += 13 if x%7 not in (0,1) else 2
            if y == 11 and 8<=x<=47: delta += 4
            if y >= 14: delta -= 3
            pix[x,y]=tuple(max(0,min(255,v+delta)) for v in rgb)+(255,)
    # Tail planes and rudders, occluded by hull; crisp stern, no flat SVG contour.
    tail=ImageDraw.Draw(im)
    tail.polygon([(5,10),(5,7),(9,7),(12,10)],fill=rgba('29383c'))
    tail.line([(5,7),(9,7),(11,9)],fill=rgba('849295'),width=1)
    tail.point((7,8),fill=rgba('66787a'))
    tail.polygon([(5,14),(5,17),(9,17),(12,14)],fill=rgba('243238'))
    tail.line([(6,16),(9,16),(11,14)],fill=rgba('72868a'),width=1)
    tail.rectangle([1,11,5,13],fill=rgba('253840'))
    tail.line([(2,11),(5,11)],fill=rgba('71858b'))
    tail.point((0,12),fill=rgba('b0b5aa'))
    # Re-tint small tail junction to blend the fin roots into the hydrodynamic hull.
    tail.line([(10,10),(10,14)],fill=rgba('4d6266'))
    tail.point((10,10),fill=rgba('879a9b'))
    # Copper/orange age marks and panel seam interruptions, not broad vector bands.
    for x in (13,20,26,36,42,47):
        for y in (9,11,12,13,14):
            if x<52 and im.getpixel((x,y))[3] and (x+y)%4 != 0:
                pix[x,y]=rgba('4b5255' if y<13 else '2e4046')
    rust={(15,11):'745b4b',(15,12):'855c49',(16,13):'634a3d',
          (21,11):'6e5145',(22,13):'755240',(27,12):'8b6650',
          (28,13):'75513e',(29,14):'68473a',(37,11):'80644e',
          (38,13):'7e5a47',(39,14):'694b3e',(46,12):'675349'}
    for (x,y),col in rust.items():
        if im.getpixel((x,y))[3]: pix[x,y]=rgba(col)
    # Bow plates, curved metallic nose, black waterline, and glancing light.
    tail.line([(47,9),(49,10),(51,11)],fill=rgba('b3b8ad'))
    tail.point((51,12),fill=rgba('8b9f9e'))
    tail.point((52,12),fill=rgba('3a5055'))
    tail.line([(12,9),(19,9)],fill=rgba('a0adad'))
    tail.line([(37,9),(45,9)],fill=rgba('9eaba9'))
    tail.line([(11,10),(15,10)],fill=rgba('b8c1b8'))
    tail.line([(31,10),(35,10)],fill=rgba('8ca4a4'))
    tail.line([(8,13),(48,13)],fill=rgba('34464a'))
    tail.line([(11,15),(42,15)],fill=rgba('1c3039'))
    # Panel highlights: spaced gunmetal rivets in surface texture.
    for x,y,c in [(17,10,'ccd0bc'),(23,10,'a6bbb5'),(39,10,'b0b9ae'),
                  (12,12,'6e8486'),(19,12,'5f777b'),(35,12,'80918e'),
                  (46,11,'afbab4'),(49,12,'788b8b'),(12,14,'465b60')]:
        if im.getpixel((x,y))[3]: pix[x,y]=rgba(c)
    # Vent openings with their metal upper lip, avoid eye-like portholes.
    for x1,x2 in [(39,42),(46,49)]:
        tail.rectangle([x1,13,x2,13],fill=rgba('15262e'))
        tail.point((x1+1,13),fill=rgba('344c54'))
        tail.point((x2,12),fill=rgba('6c7d7d'))
    # Sail / kiosk modeled with 4 bands and bevel (aft left, forward right).
    tower_rows={5:(25,32),6:(23,34),7:(22,35),8:(22,35),9:(23,34)}
    tower_col=['a9b5af','7e9290','627b7d','50686d','3a555c']
    for y,(left,right) in tower_rows.items():
        for x in range(left,right+1):
            if x in (left,right): col='1d3038'
            elif x in (left+1,right-1): col='849798' if y<8 else '546b70'
            else:
                base=bytes.fromhex(tower_col[y-5]); d=(-2 if x%3 else 7) + (3 if x<29 else -4)
                pix[x,y]=tuple(max(0,min(255,b+d)) for b in base)+(255,)
                continue
            pix[x,y]=rgba(col)
    tail.line([(25,5),(32,5)],fill=rgba('d0cdc0'))
    tail.point((24,6),fill=rgba('8ba9a5'))
    tail.line([(23,8),(34,8)],fill=rgba('324e54'))
    tail.line([(24,9),(33,9)],fill=rgba('203940'))
    tail.point((31,7),fill=rgba('91a8a6'))
    tail.point((27,8),fill=rgba('688e90'))
    tail.line([(31,7),(31,9)],fill=rgba('354c55'))
    # Multi-mast silhouette and bent metallic snorkel, not featureless vertical bars.
    for x,top,end in [(25,3,5),(28,2,5),(31,1,5),(34,3,6)]:
        tail.line([(x,top),(x,end)],fill=rgba('111e27'))
        if x!=34: tail.point((x-1,top+1),fill=rgba('8fa49e'))
        if x!=25: tail.point((x,top),fill=rgba('c4c8b7'))
    tail.point((32,0),fill=rgba('e3d8bc'))
    tail.point((33,0),fill=rgba('e3d8bc'))
    tail.point((34,1),fill=rgba('a4b1a9'))
    tail.point((31,0),fill=rgba('3f4f51'))
    tail.point((34,3),fill=rgba('d0c8ae'))
    # Very narrow horizon reflection running lengthwise over the top of the hull.
    for x in range(14,49):
        if x%5 in (0,2) and im.getpixel((x,10))[3]:
            tail.point((x,10),fill=rgba('849b9d'))
    return im

def paint_wake() -> Image.Image:
    """Separate optional water interaction: no water is embedded into the ship PNG."""
    im=Image.new('RGBA',WAKE_SIZE,(0,0,0,0)); d=ImageDraw.Draw(im)
    # A wake streaming to the left/stern, tapering out rather than white scribble.
    foam=[(2,12,'456f74'),(4,13,'5b8c8d'),(6,12,'82a4a1'),(7,14,'b6c9b9'),
          (10,13,'82a8a6'),(12,15,'9bbab2'),(15,16,'c8d7c4'),
          (19,17,'658e94'),(22,18,'9bb3aa'),(26,19,'79afb5'),
          (31,18,'abc9c4'),(35,19,'759da1'),(39,20,'adcaca'),
          (43,19,'6f9c9e'),(47,18,'b6d3c9'),(51,19,'88aead'),
          (55,16,'bbd4cd'),(58,15,'668d91'),(61,14,'9bb8b8'),
          (63,13,'73999b')]
    for x,y,c in foam:
        if 0<=x<66 and 0<=y<22: d.point((x,y),fill=rgba(c))
    # Surface rills run *parallel* to the hull, broken where turbulence fades.
    for a,b,y,col in [(5,11,16,'528a90'),(9,16,17,'6b9d9e'),
                      (19,24,19,'608e94'),(30,34,20,'547f87'),
                      (39,44,19,'659b9d'),(47,52,18,'80b4b5')]:
        d.line([(a,y),(b,y)],fill=rgba(col))
    # Sparse foam near contact edges, including the stern/aft fin junction.
    for pt in [(8,11),(9,10),(14,14),(17,15),(20,16),(24,17),(28,17),
               (36,18),(42,18),(48,17),(55,15),(62,12)]:
        d.point(pt,fill=rgba('97c4c2'))
    return im

def make_sea(light:bool)->Image.Image:
    im=Image.new('RGBA',(320,240),(0,0,0,255)); p=im.load()
    rng=random.Random(12023 if light else 9023)
    for y in range(240):
        for x in range(320):
            wave=math.sin(x*.135+y*.19)+.4*math.sin(x*.053-y*.081)+.25*math.sin(x*.24+y*.051)
            grain=rng.uniform(-6,6)
            shine = (max(0,wave-1.15)*17)
            if light: r,g,b=16,113,118
            else: r,g,b=5,73,83
            delta=wave*11 + grain*0.65 + shine*1.5
            p[x,y]=(int(max(0,min(255,r+delta*.65))),int(max(0,min(255,g+delta))),int(max(0,min(255,b+delta*1.06))),255)
    return im

def make_screen(ship:Image.Image,wake:Image.Image,light:bool)->Image.Image:
    canvas=make_sea(light);d=ImageDraw.Draw(canvas,'RGBA')
    # Decorative context ONLY, not G05 legacy reproduction or a gameplay screen.
    d.rectangle([0,0,319,31],fill=(9,48,57,255))
    d.line([(1,1),(319,1)],fill=(199,165,110,255))
    d.line([(2,29),(317,29)],fill=(174,139,90,255),width=1)
    d.text((12,9),'FLOT-01  |  INTACT H',fill=(235,225,200,255))
    d.rectangle([209,45,317,233],fill=(9,59,68,230))
    d.rectangle([210,46,315,230],outline=(133,107,74,255))
    d.text((218,61),'R3-A',fill=(230,216,182,255))
    d.text((218,82),'3 CASES',fill=(213,224,213,255))
    d.text((218,100),'54 x 18 px',fill=(188,213,211,255))
    d.text((218,124),'MER CLAIRE' if light else 'MER SOMBRE',fill=(142,204,201,255))
    d.text((218,158),'CANDIDAT',fill=(223,196,138,255))
    d.text((218,177),'REVUE PO',fill=(223,196,138,255))
    d.text((218,197),'NON TESTE',fill=(183,201,202,255))
    # G05 geometry: x=21 y=53, 10x10, pitch 18; C6-E6 starts x=57 y=143.
    gx,gy,pitch=21,53,18
    d.rectangle([gx,gy,gx+180,gy+180],fill=(0,30,40,25))
    for i in range(11):
        x=gx+i*pitch;y=gy+i*pitch
        d.line([(x,gy),(x,gy+180)],fill=(111,179,183,108),width=1)
        d.line([(gx,y),(gx+180,y)],fill=(111,179,183,108),width=1)
    for i in range(10):
        d.text((gx+i*pitch+6,40),chr(65+i),fill=(217,224,195,255))
        d.text((8,gy+i*pitch+4),str(i+1),fill=(217,224,195,255))
    ship_xy=(gx+2*pitch,gy+5*pitch)
    canvas.alpha_composite(wake,(ship_xy[0]-WAKE_ORIGIN[0],ship_xy[1]))
    canvas.alpha_composite(ship,ship_xy)
    return canvas.convert('RGB')

def detail(ship:Image.Image,wake:Image.Image,light:bool)->Image.Image:
    # This proof uses the whole 66x22 wake footprint. It is NOT the ship sprite.
    sea=make_sea(light).crop((45,134,45+66,134+22)).convert('RGBA')
    sea.alpha_composite(wake)
    sea.alpha_composite(ship,WAKE_ORIGIN)
    return sea.resize((264,88),Image.Resampling.NEAREST).convert('RGB')

def serialize(im:Image.Image)->bytes:
    from io import BytesIO
    b=BytesIO();im.save(b,format='PNG',optimize=True);return b.getvalue()

def generate()->dict[str,bytes]:
    ship=paint_ship();wake=paint_wake()
    files={
        'sous_marin_intact_r3a_h_54x18.png':serialize(ship),
        'sous_marin_intact_r3a_h_x4.png':serialize(ship.resize((216,72),Image.Resampling.NEAREST)),
        'sillage_r3a_h_66x22.png':serialize(wake),
        'sillage_r3a_h_x4.png':serialize(wake.resize((264,88),Image.Resampling.NEAREST)),
        'r3a_sur_mer_sombre_320x240.png':serialize(make_screen(ship,wake,False).quantize(colors=24,dither=Image.Dither.NONE)),
        'r3a_sur_mer_claire_320x240.png':serialize(make_screen(ship,wake,True).quantize(colors=24,dither=Image.Dither.NONE)),
        'r3a_detail_mer_sombre_x4.png':serialize(detail(ship,wake,False)),
        'r3a_detail_mer_claire_x4.png':serialize(detail(ship,wake,True)),
    }
    return files

def validate() -> None:
    s=paint_ship();w=paint_wake()
    assert s.size==(54,18) and s.mode=='RGBA'
    assert set(s.getchannel('A').get_flattened_data()).issubset({0,255})
    assert w.size==(66,22) and w.mode=='RGBA'
    assert set(w.getchannel('A').get_flattened_data()).issubset({0,255})
    assert s.getpixel((0,0))[3]==0 and s.getpixel((53,17))[3]==0
    assert s.resize((216,72),Image.Resampling.NEAREST).resize((54,18),Image.Resampling.NEAREST).tobytes()==s.tobytes()

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--check',action='store_true');a=parser.parse_args()
    validate(); outputs=generate()
    for file,data in outputs.items():
        path=ROOT/file
        if a.check:
            if not path.exists() or path.read_bytes()!=data:raise SystemExit('MISMATCH: '+file)
        else: path.write_bytes(data)
        print(file,len(data),hashlib.sha256(data).hexdigest())
    print('R3A_OK: RGB/alpha 0-255, native dimensions, x4 NEAREST, deterministic; ON_DEVICE_UNVERIFIED')
if __name__=='__main__':main()