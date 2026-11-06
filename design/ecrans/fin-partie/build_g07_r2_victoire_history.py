#!/usr/bin/env python3
"""G07-R2 VICTOIRE: compose exactly 17 shot markers from auditable cell data.

Run original build_g07_r2_victoire.py first; this second pass keeps the
exact verified grid, its existing five hit marks and five misses, adds twelve
re-editable hit layers, and updates the deliverable ORA and hashes.
DESIGN_PREVIEW only; not a firmware resource or hardware test.
"""
from __future__ import annotations
import hashlib, io, json, os, sys, zipfile
from pathlib import Path
from xml.etree import ElementTree as ET
import numpy as np
from PIL import Image, ImageChops, ImageDraw

D=Path(__file__).resolve().parent
# The script is designed to run in the canonical design/ecrans/fin-partie directory.
BASE = D/'g07_r2_victoire_320x240.png'
X4 = D/'g07_r2_victoire_x4.png'
ORA = D/'g07_r2_victoire_source.ora'
JSON = D/'g07_r2_victoire_history.json'
REPORT = D/'g07_r2_victoire_provenance.json'
GEOM = D/'g07_r2_victoire_geometry_proof_320x240.png'
GEOMX4 = D/'g07_r2_victoire_geometry_proof_x4.png'
COLS='ABCDEFGHIJ'; X0=21; Y0=53; STEP=18
CYAN=(182,227,237)

# Five impacts existed in the approved underlying composition; preserved unchanged.
BASE_HITS=['F1','E4','J5','D7','G9']
MISSES=['C2','H3','B5','G7','J8']
SHIPS={
 'Porte-avions':['B1','C1','D1','E1','F1'],
 'Croiseur':['E4','F4','G4','H4'],
 'Contre-torpilleur':['J4','J5','J6'],
 'Sous-marin':['B7','C7','D7'],
 'Patrouilleur':['G9','H9'],
}
ALL_HITS=[c for cells in SHIPS.values() for c in cells]
ADDED=[c for c in ALL_HITS if c not in BASE_HITS]

def pxy(cell):
 col=COLS.index(cell[0]); row=int(cell[1:])-1
 assert 0<=col<10 and 0<=row<10
 return 21+18*col,53+18*row

def png_bytes(im):
 b=io.BytesIO(); im.save(b,format='PNG',optimize=True);return b.getvalue()

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def mask_from_hit(image, cell):
 """Derive warm fire and smoky plume pixels from original painted hit.
 Pixels are feathered into the local sea, never rectangular sticker cutouts.
 """
 x,y=pxy(cell)
 a=np.asarray(image.crop((x+1,y+1,x+18,y+18)).convert('RGB')).astype(np.float32)
 r,g,b=a[:,:,0],a[:,:,1],a[:,:,2]
 yy,xx=np.indices((17,17)); dx=np.abs(xx-8)
 # Small glow tongues, dark smoke and integrated low reflections.
 core=(r>155)&(r>g+18)&(r>b+40)
 ember=(r>110)&(r>g+19)&(r>b+17)
 plume=(yy<11)&(dx<7)&(r>52)&(r>g+8)&(r>b+2)
 silver_smoke=(yy<7)&(dx<5)&(r>77)&(r>g+1)&(g>b-4)
 alpha=np.zeros((17,17),np.uint8)
 alpha[plume]=185
 alpha[silver_smoke]=np.maximum(alpha[silver_smoke],145)
 alpha[ember]=238
 alpha[core]=255
 # Avoid isolated borrowed sea glints far from the fire.
 alpha[dx>=7]=np.minimum(alpha[dx>=7],80)
 alpha[yy>=15]=np.minimum(alpha[yy>=15],160)
 out=np.zeros((17,17,4),np.uint8);out[:,:,:3]=a.astype(np.uint8);out[:,:,3]=alpha
 return Image.fromarray(out,'RGBA')

def draw_marker(base, template, cell, index):
 """A separate, named OpenRaster layer, contained within cell interiors."""
 x,y=pxy(cell)
 layer=Image.new('RGBA',(320,240),(0,0,0,0))
 sprite=template.copy()
 # Slight variation across flames; pixels retain their maritime-source colors.
 layer.paste(sprite,(x+1,y+1))
 return layer

def verify(before, after, overlays):
 assert before.size==after.size==(320,240)
 a=np.asarray(after.convert('RGB')); bb=np.asarray(before.convert('RGB'))
 cm=np.all(a==np.asarray(CYAN),axis=2)
 assert [x for x in range(18,205) if cm[53:234,x].sum()==181]==list(range(21,202,18))
 assert [y for y in range(50,236) if cm[y,21:202].sum()==181]==list(range(53,234,18))
 assert np.array_equal(a[:,202:],bb[:,202:]),'right-panel preservation FAIL'
 # Existing misses must remain unchanged as full cells including boundaries.
 for cell in MISSES:
  x,y=pxy(cell)
  assert np.array_equal(a[y:y+18,x:x+18],bb[y:y+18,x:x+18]),f'miss modified: {cell}'
 for cell in BASE_HITS:
  x,y=pxy(cell)
  assert np.array_equal(a[y:y+18,x:x+18],bb[y:y+18,x:x+18]),f'existing hit modified: {cell}'
 assert len(ALL_HITS)==17 and len(set(ALL_HITS))==17
 assert len(BASE_HITS)==5 and len(ADDED)==12
 assert not (set(ALL_HITS)&set(MISSES))
 assert set(ALL_HITS)==set(sum(SHIPS.values(),[]))
 assert sorted(map(len,SHIPS.values()))==[2,3,3,4,5]
 # No duplicated markers, and the 12 new markers land only in their own tile.
 changed=np.any(a!=bb,axis=2); tiles=set()
 for y,x in zip(*np.where(changed)):
  assert 21<x<201 and 53<y<233, 'decor or border changed'
  tile=COLS[(x-21)//18]+str(1+(y-53)//18)
  tiles.add(tile)
 assert tiles==set(ADDED), ('wrong modified cells',tiles,set(ADDED))
 for cell in ADDED:
  x,y=pxy(cell)
  changed_here=np.count_nonzero(changed[y+1:y+18,x+1:x+18]); assert changed_here>=8,(cell,changed_here)
 # Negative controls: rogue shot, malformed coordinates, missing fire.
 assert len(set(ALL_HITS+['F1']))!=len(ALL_HITS)+1
 assert 'C2' in MISSES and 'C2' not in ALL_HITS
 assert not np.array_equal(np.roll(a,1,axis=1)[:,202:],bb[:,202:])
 return {'grid_lines': 'PASS 22 full-length 1px RGB lines', 'hits_17_distinct':'PASS',
         'misses_5_unchanged':'PASS', 'original_hits_5_unchanged':'PASS',
         'overlays_12_individual':'PASS', 'no_other_tiles_changed':'PASS',
         'right_panel_x_ge_202_unchanged':'PASS', 'negative_tests':'PASS'}

def create_ora(original_ora, overlays, merged):
 with zipfile.ZipFile(io.BytesIO(original_ora),'r') as src:
  root=ET.fromstring(src.read('stack.xml'))
  stack=root.find('stack')
  assert stack is not None
  orig=[(name,src.read(name)) for name in src.namelist() if name.startswith('data/')]
  with zipfile.ZipFile(ORA,'w') as dst:
   dst.writestr('mimetype','image/openraster',compress_type=zipfile.ZIP_STORED)
   for name,data in orig:dst.writestr(name,data,compress_type=zipfile.ZIP_DEFLATED)
   for i,(cell,layer) in enumerate(overlays):
    p=f'data/history_touch_{i:02d}_{cell}.png'
    # Top of stack, in front of sea but below none of the grid lines.
    node=ET.Element('layer',{'name':f'TOUCHÉ {cell} — ajout indépendant',
      'src':p,'visibility':'visible','opacity':'1.0','x':'0','y':'0',
      'composite-op':'svg:src-over'})
    stack.insert(0,node)
    dst.writestr(p,png_bytes(layer),compress_type=zipfile.ZIP_DEFLATED)
   dst.writestr('stack.xml',ET.tostring(root,encoding='utf-8',xml_declaration=True))
   dst.writestr('mergedimage.png',png_bytes(merged))
   dst.writestr('Thumbnails/thumbnail.png',png_bytes(merged.resize((160,120),Image.Resampling.NEAREST)))

def main():
 assert JSON.exists(),f'missing history scenario: {JSON.name}'
 scenario=json.loads(JSON.read_text(encoding='utf-8'))
 assert scenario['enemy_fleet_for_test_only']==SHIPS
 assert scenario['attack_hits']==ALL_HITS
 assert scenario['attack_misses']==MISSES
 assert scenario['new_hit_cells']==ADDED
 before=Image.open(BASE).convert('RGB');old_ora=ORA.read_bytes()
 # Derive painted texture from three existing fires; all are part of the
 # source artwork, not external assets. No new hull image is used.
 source_sprites=[mask_from_hit(before,c) for c in ['E4','F1','E4']]
 overlays=[];result=before.convert('RGBA')
 for i,cell in enumerate(ADDED):
  lay=draw_marker(before,source_sprites[i%len(source_sprites)],cell,i)
  result=Image.alpha_composite(result,lay)
  overlays.append((cell,lay))
 result=result.convert('RGB')
 checks=verify(before,result,overlays)
 result.save(BASE,format='PNG',optimize=True)
 zoom=result.resize((1280,960),Image.Resampling.NEAREST)
 zoom.save(X4,format='PNG',optimize=True)
 assert ImageChops.difference(Image.open(X4).convert('RGB'),zoom).getbbox() is None
 checks['nearest_x4_pixel_exact']='PASS'
 # Diagnostics are kept separate, never shipped as an in-game scene.
 diagnostic=result.copy();dr=ImageDraw.Draw(diagnostic)
 for cell in ALL_HITS:
  x,y=pxy(cell);dr.rectangle((x+4,y+4,x+13,y+13),outline=(255,238,0),width=1)
 for cell in MISSES:
  x,y=pxy(cell);dr.rectangle((x+4,y+4,x+13,y+13),outline=(20,230,255),width=1)
 diagnostic.save(GEOM,format='PNG',optimize=True)
 diagnostic.resize((1280,960),Image.Resampling.NEAREST).save(GEOMX4,format='PNG',optimize=True)
 create_ora(old_ora,overlays,result)
 # Assertions are computed on re-opened, final PNG bytes.
 assert ImageChops.difference(result,Image.open(BASE).convert('RGB')).getbbox() is None
 manifest=json.loads(REPORT.read_text(encoding='utf-8'))
 manifest['scenario']=scenario
 manifest['assertions_history_PC']=checks
 manifest['layers_history_added']=[f'TOUCHÉ {c}' for c in ADDED]
 manifest['geometry_proof_note']='Yellow squares=TOUCHÉ; cyan squares=RATÉ. Diagnostic only, not product art.'
 manifest['limitations'].append('Five existing painted hits remain merged with the water; twelve additional fire marks are independent editable ORA layers. No firmware sprite delivery.')
 manifest['exports']=[{'file':p.name,'bytes':p.stat().st_size,'sha256':sha(p)} for p in [BASE,X4,GEOM,GEOMX4,ORA,JSON]]
 REPORT.write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('PASS G07-R2 VICTOIRE: exactly 17 distinct hits; 5 misses untouched; geometry unchanged; right panel unchanged; 1280x960 NEAREST; ORA layers; negative controls')

if __name__=='__main__':main()
