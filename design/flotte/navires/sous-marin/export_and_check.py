#!/usr/bin/env python3
"""FLOT-01 pilot: SVG->native RGBA and deterministic 320x240 evidence.

Run from any cwd: python export_and_check.py [--check].
--check NEVER writes to disk. No firmware encoding decision is implied.
Requires CairoSVG >=2.7 and Pillow >=11.3.
"""
from __future__ import annotations
import argparse
import hashlib
import io
import json
import re
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import cairosvg

HERE = Path(__file__).resolve().parent
SRC = HERE / 'sous_marin_intact_h.svg'
ORIGIN=(19,39); CELL=18; H_BBOX=(55,129,109,147)

def rgba(svg: bytes, wh:tuple[int,int]) -> Image.Image:
    return Image.open(io.BytesIO(cairosvg.svg2png(bytestring=svg, output_width=wh[0], output_height=wh[1]))).convert('RGBA')

def png(im:Image.Image) -> bytes:
    output=io.BytesIO(); im.save(output,format='PNG',optimize=False,compress_level=9)
    return output.getvalue()

def rotate_source(hsvg:str)->bytes:
    g=re.search(r'(<g id="sous-marin-intact">.*?</g>)',hsvg,re.S)
    assert g, 'Expected editable source group not found'
    return ('''<svg xmlns="http://www.w3.org/2000/svg" width="18" height="54" viewBox="0 0 18 54" shape-rendering="crispEdges">\n'''
            '<title>FLOT-01 sous-marin intact vertical, concept PO</title>\n'
            '<desc>Rotation -90 de la source canonique H, proue vers le haut.</desc>\n'
            '<g transform="translate(0 54) rotate(-90)">\n'+g.group(1)+'\n</g>\n</svg>\n').encode('utf-8')

def text(d, xy, value, fill='#E9D9B2', anchor=None):
    d.text(xy,value,font=ImageFont.load_default(),fill=fill,anchor=anchor)

def sea(draw,box,base,accent):
    xa,ya,xb,yb=box
    draw.rectangle(box,fill=base)
    # Low-amplitude, deterministic sea texture, behind sprite only.
    for y in range(ya+4,yb,9):
        for j in range(9):
            x=xa + 7 + ((j*37 + y*7) % max(1,xb-xa-20))
            draw.line([(x,y),(x+4+(j%3),y)],fill=accent,width=1)

def base_preview(vert=False):
    im=Image.new('RGBA',(320,240),'#081E2C');d=ImageDraw.Draw(im)
    d.rectangle((0,0,319,22),fill='#102C39')
    d.line((0,22,319,22),fill='#B98157',width=1)
    text(d,(7,5),'FLOT-01  /  SOUS-MARIN  /  INTACT',fill='#F3D69E')
    sea(d,(19,39,198,218),'#0B5365','#1F7984')
    for x in range(11):
        xx=19+x*18
        d.line((xx,39,xx,219),fill='#86ACAA',width=1)
    for y in range(11):
        yy=39+y*18
        d.line((19,yy,199,yy),fill='#86ACAA',width=1)
    for i in range(10):
        text(d,(19+18*i+6,26),chr(65+i),fill='#E8C58E')
        text(d,(4,43+18*i),str(i+1),fill='#E8C58E')
    d.rectangle((54,128,73,183) if vert else (54,128,109,147),outline='#E9B66E',width=1)
    text(d,(30,224),'TEST V C6-C8' if vert else 'C6-E6   |  54 x 18',fill='#E8C58E')
    d.rectangle((205,36,316,218),fill='#102B37',outline='#B98157',width=1)
    text(d,(213,44),'VUE GRILLE G02',fill='#F3D69E')
    d.line((212,61,307,61),fill='#986F57')
    text(d,(213,71),'H : 3 CASES',fill='#DCE8D8')
    text(d,(213,105),'V : ROTATION',fill='#DCE8D8')
    d.rectangle((245,121,264,176),outline='#B98157')
    text(d,(212,192),'SPRITES RGBA',fill='#F3D69E')
    text(d,(212,204),'ALPHA REEL',fill='#C3D6CE')
    return im

def preview_grid(h,v):
    im=base_preview(); im.alpha_composite(h,(55,129)); im.alpha_composite(h,(238,85)); im.alpha_composite(v,(246,122))
    return im

def preview_grid_v(h,v):
    im=base_preview(vert=True)
    im.alpha_composite(v,(55,129))
    im.alpha_composite(h,(238,85))
    im.alpha_composite(v,(246,122))
    return im

def preview_alpha(h,v):
    im=Image.new('RGBA',(320,240),'#0B1D2D');d=ImageDraw.Draw(im)
    d.rectangle((0,0,319,22),fill='#102C39');d.line((0,22,319,22),fill='#B98157')
    text(d,(8,5),'FLOT-01  /  PREUVE ALPHA SUR FONDS',fill='#F3D69E')
    # Exactly same RGBA PNG composited onto different backgrounds.
    sea(d,(8,38,155,226),'#79C8C5','#D2F0DB')
    sea(d,(164,38,311,226),'#072B47','#11566C')
    d.rectangle((8,38,155,226),outline='#B98157');d.rectangle((164,38,311,226),outline='#B98157')
    text(d,(20,48),'OCEAN CLAIR',fill='#093448');text(d,(177,48),'OCEAN SOMBRE',fill='#F8D9A0')
    # pixel-perfect 1x sprite placements both H and V
    im.alpha_composite(h,(56,91));im.alpha_composite(h,(212,91))
    im.alpha_composite(v,(72,130));im.alpha_composite(v,(228,130))
    d=ImageDraw.Draw(im)
    text(d,(34,73),'INTACT H 54x18',fill='#093448');text(d,(187,73),'INTACT H 54x18',fill='#F3D69E')
    text(d,(29,201),'V 18x54',fill='#093448');text(d,(186,201),'V 18x54',fill='#F3D69E')
    return im

def preview_g02(h):
    im=Image.new('RGBA',(320,240),'#0B2230');d=ImageDraw.Draw(im)
    d.rectangle((0,0,319,22),fill='#102C39');d.line((0,22,319,22),fill='#B98157')
    text(d,(8,5),'FLOT-01  /  FILIATION SILHOUETTE G02',fill='#F3D69E')
    sea(d,(14,41,305,106),'#0B5365','#20808A');sea(d,(14,126,305,191),'#0B5365','#20808A')
    text(d,(20,46),'G02 APPROUVE (CONCEPT)',fill='#F3D69E');text(d,(20,131),'FLOT-01 INTACT (PROPOSITION)',fill='#F3D69E')
    # Reprise de la silhouette #nav-sous-marin de G02, sans modifier la source.
    g02='''<svg xmlns="http://www.w3.org/2000/svg" width="54" height="18" viewBox="0 0 54 18" shape-rendering="crispEdges"><ellipse cx="27" cy="9" rx="25" ry="5" fill="#0A1C32"/><ellipse cx="27" cy="9" rx="23" ry="4" fill="#314E60"/><path d="M7 7L47 7" fill="none" stroke="#647F86" stroke-width="1"/><path d="M9 11L45 11" fill="none" stroke="#07354E" stroke-width="1"/><rect x="22" y="5" width="11" height="7" fill="#0A1C32"/><rect x="23" y="6" width="8" height="5" fill="#94A59B"/><rect x="27" y="3" width="2" height="4" fill="#0A1C32"/><rect x="26" y="7" width="5" height="2" fill="#07354E"/><rect x="28" y="3" width="1" height="1" fill="#F8CB72"/></svg>'''
    im.alpha_composite(rgba(g02.encode(),(54,18)),(130,76))
    im.alpha_composite(h,(130,161))
    d=ImageDraw.Draw(im)
    text(d,(16,208),'FUSEAU + KIOSQUE : identite conservee',fill='#D3E6DC')
    text(d,(16,221),'Reference PO A : metallisation / mats',fill='#E8C58E')
    return im

def build():
    h_svg=SRC.read_bytes();v_svg=rotate_source(h_svg.decode('utf-8'))
    h=rgba(h_svg,(54,18));v=h.transpose(Image.Transpose.ROTATE_90)
    # Positive alpha validation, strictly within fixed size.
    assert h.size==(54,18) and v.size==(18,54)
    assert h.getpixel((0,0))[3]==0 and h.getpixel((53,17))[3]==0
    alpha=list(h.getchannel('A').get_flattened_data());assert 0 in alpha and 255 in alpha and 0 < sum(a!=0 for a in alpha) < 54*18
    assert set(alpha)=={0,255},'No partial-alpha fringe on pixel edges'
    assert h.getbbox()==(1,1,53,16)
    assert v.tobytes()==h.transpose(Image.Transpose.ROTATE_90).tobytes()
    # H on G02 C6-E6; 54x18 inside true grid x=19..199, y=39..219.
    assert H_BBOX==(ORIGIN[0]+2*CELL,ORIGIN[1]+5*CELL,ORIGIN[0]+5*CELL,ORIGIN[1]+6*CELL)
    files={'sous_marin_intact_v.svg':v_svg,'sous_marin_intact_h.png':png(h),'sous_marin_intact_v.png':png(v)}
    for name,img in [('preuve_grille_320x240.png',preview_grid(h,v)),('preuve_grille_v_320x240.png',preview_grid_v(h,v)),('preuve_alpha_clair_sombre_320x240.png',preview_alpha(h,v)),('preuve_comparaison_g02_320x240.png',preview_g02(h))]:
        files[name]=png(img.convert('RGBA'))
        files[name.replace('320x240','x4_nearest')]=png(img.resize((1280,960),Image.Resampling.NEAREST))
    hashes={name:{'sha256':hashlib.sha256(data).hexdigest(),'octets':len(data)} for name,data in sorted(files.items())}
    manifest={'mission':'FLOT-01 / #92','statut':'DESIGN_PREVIEW - ON_DEVICE_UNVERIFIED / ATTENTE_STOP_PO',
              'source':'sous_marin_intact_h.svg',
              'references':['design/flotte/g02_grille_320x240.svg','draft/sous_marin_PO_A.jpeg'],
              'dimensions':{'H':[54,18],'V':[18,54],'grille':[320,240],'zoom':[1280,960]},
              'placement_G02':{'cellules':'C6-E6','origine_grille':[19,39],'pas':18,'ancre_H':[55,129], 'demonstration_V_hors_G02':'C6-C8, uniquement banc essai, pas changement regle'},
              'orientation':{'H':'proue vers la droite','V':'rotation anti-horaire 90 deg de H; proue vers le haut'},
              'format':'PNG RGBA (alpha 0/255), aucun pixel d ocean integre',
              'raw_rgba_estimate_bytes':54*18*4,'files':hashes}
    files['manifest.json']=(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n').encode('utf-8')
    return files,manifest

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--check',action='store_true',help='Non-destructif: compare sans ecrire');args=ap.parse_args()
    files,manifest=build()
    if args.check:
        for name,expected in files.items():
            path=HERE/name
            assert path.is_file(),f'MISSING {name}'
            assert path.read_bytes()==expected,f'CHANGED {name}'
            print(f'PASS {name}: {len(expected)} bytes / {hashlib.sha256(expected).hexdigest()[:12]}')
        for n in ('preuve_grille','preuve_grille_v','preuve_alpha_clair_sombre','preuve_comparaison_g02'):
            native=Image.open(io.BytesIO(files[n+'_320x240.png'])).convert('RGBA')
            x4=Image.open(io.BytesIO(files[n+'_x4_nearest.png'])).convert('RGBA')
            assert native.size==(320,240) and x4.size==(1280,960)
            assert x4.tobytes()==native.resize((1280,960),Image.Resampling.NEAREST).tobytes()
            print('PASS exact NEAREST ×4 : '+n)
        print('PASS --check non destructif : alpha, dimensions, orientation, placement G02, hashes')
    else:
        for name,data in files.items():
            (HERE/name).write_bytes(data)
            print('GENERATED '+name)

if __name__=='__main__':main()
