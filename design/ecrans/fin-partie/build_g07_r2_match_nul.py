#!/usr/bin/env python3
"""G07-R2 MATCH NUL: scenario-driven pixel-precise SVG composition and PC proofs."""
from __future__ import annotations
import base64,hashlib,json,random,zipfile,io
from pathlib import Path
import cairosvg
from PIL import Image,ImageChops,ImageDraw,ImageFont
from xml.etree import ElementTree as ET

D=Path(__file__).resolve().parent
SOURCE=D/'g07_r2_match_nul_original_320x240.png'
SVG=D/'g07_r2_match_nul_correctif_source.svg'
NATIVE=D/'g07_r2_match_nul_320x240.png'
ZOOM=D/'g07_r2_match_nul_x4.png'
PROOF=D/'g07_r2_match_nul_geometry_proof_320x240.png'
PROOF4=D/'g07_r2_match_nul_geometry_proof_x4.png'
ORA=D/'g07_r2_match_nul_source.ora'
SCENARIO=D/'g07_r2_match_nul_scenario.json'
REPORT=D/'g07_r2_match_nul_provenance.json'

# Underlying layouts: private DOCUMENTARY evidence only; no boats on the ATTAQUE image.
FLEETS={
 'enemy':{'porte_avions':['C2','C3','C4','C5','C6'], 'croiseur':['F2','G2','H2','I2'], 'sous_marin':['H5','H6','H7'], 'contre_torpilleur':['B8','C8','D8'], 'patrouilleur':['I9','J9']},
 'human':{'porte_avions':['B2','C2','D2','E2','F2'], 'croiseur':['I4','I5','I6','I7'], 'sous_marin':['C6','D6','E6'], 'contre_torpilleur':['F5','F6','F7'], 'patrouilleur':['G8','H8']}
}
ATTACK_MISSES=['A1','E1','J2','G4','A5','F6','J7','D9','G10']
DEFENSE_MISSES=['A1','J1','A4','D4','G4','A7','B9','J8','E10']
GRID='#b6e3ed'

def cell(s):
    c=ord(s[0])-65;r=int(s[1:])-1
    assert 0<=c<10 and 0<=r<10
    return c,r

def flat(d): return [pos for coords in d.values() for pos in coords]

def verify_fleet(d):
    occupied=[]
    lengths={'porte_avions':5,'croiseur':4,'sous_marin':3,'contre_torpilleur':3,'patrouilleur':2}
    for ship,cells in d.items():
        assert len(cells)==lengths[ship],ship
        xy=[cell(s) for s in cells]
        assert len(set(xy))==len(xy)
        xs={p[0] for p in xy};ys={p[1] for p in xy}
        assert (len(xs)==1 and sorted(ys)==list(range(min(ys),max(ys)+1))) or (len(ys)==1 and sorted(xs)==list(range(min(xs),max(xs)+1))),ship
        occupied+=cells
    assert len(occupied)==17 and len(set(occupied))==17


def make_scenario():
    for v in FLEETS.values():verify_fleet(v)
    a=flat(FLEETS['enemy']); d=flat(FLEETS['human'])
    assert set(a).isdisjoint(ATTACK_MISSES) and set(d).isdisjoint(DEFENSE_MISSES)
    assert a[-1]=='J9' and d[-1]=='H8'
    # Document all 26 paired rounds. Each side has 16 hits + 9 misses before final hit.
    ar=a[:-1]+ATTACK_MISSES; dr=d[:-1]+DEFENSE_MISSES
    # Interleave the misses to make a believable history; last turn retained for paired hit.
    order=[0,1,17,2,18,3,4,19,5,6,20,7,8,21,9,10,22,11,23,12,13,24,14,15,16]
    assert sorted(order)==list(range(25))
    turns=[]
    for n,j in enumerate(order,1):
        pos_a=ar[j];pos_d=dr[j]
        turns.append({'round':n,'attack':{'cell':pos_a,'result':'TOUCHÉ' if pos_a in a else 'RATÉ'},'defense':{'cell':pos_d,'result':'TOUCHÉ' if pos_d in d else 'RATÉ'},'simultaneous_reveal':True})
    turns.append({'round':26,'attack':{'cell':'J9','result':'TOUCHÉ','sinks':'patrouilleur'},'defense':{'cell':'H8','result':'TOUCHÉ','sinks':'patrouilleur'},'simultaneous_reveal':True,'outcome_after_both':'MATCH NUL'})
    assert len(turns)==26
    for side,all_hits in [('attack',a),('defense',d)]:
        hits=[t[side]['cell'] for t in turns if t[side]['result']=='TOUCHÉ']; misses=[t[side]['cell'] for t in turns if t[side]['result']=='RATÉ']
        assert len(hits)==17 and len(set(hits))==17 and set(hits)==set(all_hits)
        assert len(misses)==9 and len(set(misses))==9 and not (set(hits)&set(misses))
    return {'schema':'G07-R2/LOT1-MATCH-NUL-documentaire-v1','status':'SCENARIO_ILLUSTRATIF_NON_MOTEUR','board':'10x10','fleet_geometry':'5+4+3+3+2 = 17 cases',
     'fleets_private_not_drawn_on_attack':FLEETS, 'attack_hits':a, 'attack_misses':ATTACK_MISSES,'defense_hits':d,'defense_misses':DEFENSE_MISSES,
     'rounds':turns,'last_simultaneous_pair':{'round':26,'attack':'J9 TOUCHÉ','defense':'H8 TOUCHÉ','result':'MATCH NUL'},
     'render_policy':'ATTAQUE only; 17 fiery compact distinct touch markers; 9 blue rings misses; ships concealed. Defense history is documentary only, no extra panel rendered.'}


def h(x):return str(x)

def build_svg(scenario):
    img64=base64.b64encode(SOURCE.read_bytes()).decode('ascii')
    out=['<?xml version="1.0" encoding="UTF-8"?>',
    '<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="320" height="240" viewBox="0 0 320 240" shape-rendering="crispEdges">',
    '<title>G07-R2 — MATCH NUL compact, correction de 17 touches ennemies</title>',
    '<desc>Maquette CYD WARSHIP 320x240. Décor original raster conservé; surcouches impacts, verdict et texte modifiables.</desc>',
    '<defs>',
    '<image id="decor-original" width="320" height="240" xlink:href="data:image/png;base64,'+img64+'"/>',
    '</defs>',
    '<g id="decor-original-aplati"><use xlink:href="#decor-original"/></g>']
    # Replace old floating splash & miss decals before repainting the history.
    old=['F1','C2','I2','G3','H4','C5','B6','J6','G7','D8','G9','B10']
    out.append('<g id="nettoyage-anciens-marqueurs" aria-label="Remplacement de marques incohérentes par mer de la même image">')
    for i,p in enumerate(old):
        c,r=cell(p);x=21+c*18+1;y=53+r*18+1
        # A nearby, untouched donor tile of the same maritime source.
        occupied_old={tuple(cell(q)) for q in old}
        picks=sorted([v for v in range(10) if (c,v) not in occupied_old],key=lambda v:(abs(v-r),v))
        sr=picks[0];dx=0;dy=(r-sr)*18
        out.append(f'<clipPath id="restaure_{p}"><rect x="{x}" y="{y}" width="17" height="17"/></clipPath>')
        out.append(f'<g clip-path="url(#restaure_{p})"><use xlink:href="#decor-original" transform="translate({dx} {dy})"/></g>')
    out.append('</g>')
    # Undo the beige raised button/scroll and surrounding gold petal silhouette.
    out.append('<g id="verdict-support-bois-plat" aria-label="Verdict non cliquable sur le bois">')
    out+=['<rect x="207" y="98" width="106" height="27" fill="#512213"/>',
          '<rect x="208" y="98" width="104" height="2" fill="#6a2e19"/>',
          '<rect x="209" y="101" width="102" height="4" fill="#5b2617"/>',
          '<rect x="208" y="105" width="104" height="5" fill="#6b2d18"/>',
          '<rect x="209" y="110" width="102" height="6" fill="#592418"/>',
          '<rect x="209" y="116" width="102" height="6" fill="#632918"/>',
          '<rect x="208" y="123" width="104" height="2" fill="#3c160e"/>']
    for y, segments, col in [(101,[(211,240),(260,302)],'#803b1c'),(106,[(212,229),(244,286),(293,309)],'#79341c'),(111,[(208,244),(253,279),(291,310)],'#3f180d'),(118,[(214,247),(258,301)],'#81391d'),(121,[(209,226),(272,311)],'#4b1c10')]:
        for a,b in segments:out.append(f'<path d="M{a},{y}H{b}" stroke="{col}" stroke-width="1"/>')
    out.append('</g>')
    # Small physically-flat serif lettering; NO rectangle/capsule/hover treatment.
    out.append('<g id="verdict-et-legende" text-anchor="middle">')
    out.append('<text x="260" y="111" font-family="DejaVu Serif" font-weight="bold" font-size="12" fill="#2a120b" stroke="#2a120b" stroke-width="1">MATCH NUL</text>')
    out.append('<text x="260" y="110" font-family="DejaVu Serif" font-weight="bold" font-size="12" fill="#f6d29a">MATCH NUL</text>')
    out.append('<text x="260" y="121" font-family="DejaVu Sans" font-weight="bold" font-size="7.5" fill="#f9d68e">17 TOUCHÉS</text>')
    out.append('</g>')
    # Hit vocabulary: tiny stylized ember / copper-orange splinter in dark water with foam interference.
    # Markers are deliberately separate and centered in each 18px cell; no ship silhouettes.
    out.append('<g id="historique-attaque-touches" aria-label="17 TOUCHÉ ennemis sur ATTAQUE">')
    for p in scenario['attack_hits']:
        c,r=cell(p);cx=21+c*18+9;cy=53+r*18+9
        out.append(f'<g id="touche_{p}" data-cell="{p}" transform="translate({cx},{cy})">')
        out.extend([
        '<path d="M-6,2 l3,-2 2,1 3,-2 4,2 -2,2 -3,-1 -4,2 -3,-1Z" fill="#051b29" opacity=".9"/>',
        '<path d="M-6,-1 l2,-2 2,1 3,-3 2,2 2,-1 1,2 -2,2 -3,0 -3,3 -3,-1Z" fill="#531e18"/>',
        '<path d="M-4,-3 l2,1 2,2 2,-2 2,0 -2,2 -1,1 2,2 -2,1 -2,-2 -2,2 -2,-1 2,-2 -1,-2Z" fill="#f17a22"/>',
        '<path d="M-2,-2h2v2h2v2h-2v1h-2v-1h-1v-2h1Z" fill="#ffe5a1"/>',
        '<path d="M-6,4 h2 m6,0 h3" stroke="#8dd4d6" stroke-width="1" opacity=".9"/>',
        '<path d="M-5,-4 h1 m8,1 h1" stroke="#ffd174" stroke-width="1"/>'])
        out.append('</g>')
    out.append('</g>')
    # Miss vocabulary: pale-cyan ripple, hollow dark center.
    out.append('<g id="historique-attaque-rates" aria-label="9 RATÉ sur ATTAQUE">')
    for p in scenario['attack_misses']:
        c,r=cell(p);cx=21+c*18+9;cy=53+r*18+9
        out.append(f'<g id="rate_{p}" data-cell="{p}" transform="translate({cx},{cy})">')
        out.append('<ellipse cx="0" cy="0" rx="5" ry="3" fill="#061f33" stroke="#9be9ee" stroke-width="1.5"/>')
        out.append('<path d="M-3,-1 h2 m3,2 h2" stroke="#e9ffff" stroke-width="1"/>')
        out.append('</g>')
    out.append('</g>')
    out.append('</svg>')
    return '\n'.join(out)+'\n'


def grid_check(png):
    rgb=png.convert('RGB');pix=rgb.load();C=(182,227,237)
    for x in range(21,202,18):
        assert all(pix[x,y]==C for y in range(53,234)),f'grid x={x} broken'
    for y in range(53,234,18):
        assert all(pix[x,y]==C for x in range(21,202)),f'grid y={y} broken'
    return {'x_lines':list(range(21,202,18)), 'y_lines':list(range(53,234,18)),'color':list(C),'all_22_lines_full_1px':'PASS'}

def checksum(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def proof(img,sc):
    x=img.copy().convert('RGBA');lay=Image.new('RGBA',img.size);p=ImageDraw.Draw(lay)
    for coord in sc['attack_hits']:
        c,r=cell(coord);x0=21+c*18;y0=53+r*18
        p.rectangle((x0+2,y0+2,x0+16,y0+16),outline=(248,211,38,255),width=1)
    for coord in sc['attack_misses']:
        c,r=cell(coord);x0=21+c*18;y0=53+r*18
        p.rectangle((x0+2,y0+2,x0+16,y0+16),outline=(41,235,252,255),width=1)
    x.alpha_composite(lay);return x.convert('RGB')

def write_ora(svg,png):
    # Detailed authoritative composition is SVG. ORA offers raster-separated groups via masks.
    bg=Image.open(SOURCE).convert('RGBA'); final=png.convert('RGBA')
    # The ORA full base layer is the original image and its editability is LIMITED.
    # For per-object edits, SVG is the editable master.
    ET.register_namespace('','')
    root=ET.Element('image',{'w':'320','h':'240','name':'G07-R2 Match nul — editable SVG is master','version':'0.0.3'})
    s=ET.SubElement(root,'stack')
    ET.SubElement(s,'layer',{'name':'Composite final (voir source SVG pour édition par objet)','src':'data/final.png','opacity':'1.0','visibility':'visible','composite-op':'svg:src-over','x':'0','y':'0'})
    ET.SubElement(s,'layer',{'name':'Décor original — référence conservée','src':'data/decor.png','opacity':'1.0','visibility':'hidden','composite-op':'svg:src-over','x':'0','y':'0'})
    def b(img):
        buf=io.BytesIO();img.save(buf,format='PNG');return buf.getvalue()
    with zipfile.ZipFile(ORA,'w') as z:
        z.writestr('mimetype','image/openraster',compress_type=zipfile.ZIP_STORED)
        z.writestr('stack.xml',ET.tostring(root,encoding='utf-8',xml_declaration=True))
        z.writestr('data/final.png',b(final));z.writestr('data/decor.png',b(bg))
        z.writestr('mergedimage.png',b(final));z.writestr('Thumbnails/thumbnail.png',b(final.resize((160,120),Image.Resampling.NEAREST)))


def main():
    scenario=make_scenario()
    SCENARIO.write_text(json.dumps(scenario,ensure_ascii=False,indent=2)+'\n')
    SVG.write_text(build_svg(scenario),encoding='utf-8')
    cairosvg.svg2png(url=str(SVG),write_to=str(NATIVE),output_width=320,output_height=240)
    im=Image.open(NATIVE).convert('RGB')
    assert im.size==(320,240)
    g=grid_check(im)
    # Independent PNG-content checks, not merely a count of SVG tags.
    pix=im.load()
    for address in scenario['attack_hits']:
        c,r=cell(address); assert pix[21+c*18+9,53+r*18+9]==(255,229,161), f'Hit marker missing at {address}'
    for address in scenario['attack_misses']:
        c,r=cell(address); assert pix[21+c*18+9,53+r*18+9]==(6,31,51), f'Miss marker missing at {address}'
    assert SVG.read_text().count('id="touche_')==17
    assert SVG.read_text().count('id="rate_')==9
    im.resize((1280,960),Image.Resampling.NEAREST).save(ZOOM)
    assert not ImageChops.difference(im.resize((1280,960),Image.Resampling.NEAREST),Image.open(ZOOM).convert('RGB')).getbbox()
    pf=proof(im,scenario);pf.save(PROOF)
    pf.resize((1280,960),Image.Resampling.NEAREST).save(PROOF4)
    write_ora(SVG,im)
    # Regression test: a one-pixel grid translation is rejected.
    shifted=Image.new('RGB',(320,240));shifted.paste(im.crop((0,0,319,240)),(1,0))
    try:grid_check(shifted)
    except AssertionError:pass
    else:raise AssertionError('Negative translation test wrongly passed')
    report={
      'mission':'G07-R2 MATCH NUL — correction Pilotage 06/11/2026','issue':53,'pr':74,
      'status':'DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED; PO artistic acceptance pending',
      'geometry':g,'attack_hit_count':len(scenario['attack_hits']),'attack_miss_count':len(scenario['attack_misses']),
      'human_defense_hit_count_documented':len(scenario['defense_hits']),
      'last_round':scenario['last_simultaneous_pair'],'scenario_assertions':'PASS 5+4+3+3+2, 17 hits each side, no repeats/overlap, round 26 double hit',
      'visual_marks_generated_from_source_svg':'17 editable hit groups and 9 editable miss groups',
      'png_marker_centers_17_hits_9_misses':'PASS', 'zoom_4x_strict_nearest':'PASS', 'negative_grid_shift_1px':'PASS (rejected)',
      'runtime_integration':'NOT_TESTED','TFT_stylus':'NOT_TESTED','firmware_asset':'NO',
      'artwork_editability':'SVG with separately addressable verdict, touch markers, miss markers and raster reference background; additional ORA flattening is NOT per-object editable.',
      'rights':'Original artwork provided by user / project; all overlay icon geometry authored for this correction; no stock imagery.',
      'files':{p.name:{'bytes':p.stat().st_size,'sha256':checksum(p)} for p in [SOURCE,SVG,NATIVE,ZOOM,PROOF,PROOF4,ORA,SCENARIO]}}
    REPORT.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print('PASS geometry & scenario & native & x4; sizes',NATIVE.stat().st_size,ZOOM.stat().st_size)
if __name__=='__main__': main()
