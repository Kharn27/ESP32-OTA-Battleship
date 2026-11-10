#!/usr/bin/env python3
"""Static PC checks of actual binary exports, layout, and negative mutations."""
from pathlib import Path
from PIL import Image
import json, hashlib, io, zipfile
ROOT=Path(__file__).resolve().parent

def error(cond,msg):
 if not cond:raise AssertionError(msg)
def digest(data):return hashlib.sha256(data).hexdigest()
def verify():
 report=json.loads((ROOT/'generation-report.json').read_text(encoding='utf-8'))
 layout=json.loads((ROOT/'layout.json').read_text(encoding='utf-8'))
 error(report['status']=='DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED','wrong status')
 error(not layout['approved_by_po'] and not layout['on_device_verified'],'false PO/hardware claims')
 error(layout['native_size']==[320,240] and layout['grid_origin']==[21,53] and layout['cell_size']==[18,18] and layout['grid_size']==[180,180],'invalid grid registration')
 # Independent G02 truth, not inferred from the layout under test.
 canonical={
   'porte_avions':['B2','C2','D2','E2','F2'],
   'sous_marin':['C6','D6','E6'],
   'contre_torpilleur':['F5','F6','F7'],
   'croiseur':['I4','I5','I6','I7'],
   'patrouilleur':['G8','H8']}
 error(layout['ships']==canonical,'G02 ships differ')
 flat=[v for a in canonical.values() for v in a]
 error(len(flat)==len(set(flat))==17,'fleet overlap')
 error(set(layout['hits'])=={'F6'} and set(layout['sunk_cells'])=={'G8','H8'},'impact scenario inconsistent')
 error(set(layout['misses'])=={'A7','I2'},'miss scenario inconsistent')
 error(set(layout['hits']+layout['sunk_cells']).issubset(flat),'hit on water')
 error(set(layout['misses']).isdisjoint(flat),'miss on ship')
 error(layout['last_shot']==['A7','RATÉ'] and layout['fleet_message']=='PATROUILLEUR COULÉ','last-shot versus fleet status')
 error(layout['navigation']=='ATTAQUE' and layout['fire_button'] is False,'FEU prohibited in DÉFENSE')
 def check_digest(path,expected):
  error(digest(path.read_bytes())==expected,'integrity mismatch for '+path.name)
 for n,entry in report['outputs'].items():
  p=ROOT/n;error(p.exists(),'missing '+n)
  check_digest(p,entry['sha256']);error(p.stat().st_size==entry['bytes'],'size mismatch '+n)
 n=Image.open(ROOT/'g05_r2_defense_320x240.png').convert('RGB')
 z=Image.open(ROOT/'g05_r2_defense_x4.png').convert('RGB')
 error(n.size==(320,240) and z.size==(1280,960),'native/zoom size mismatch')
 error(n.resize((1280,960),Image.Resampling.NEAREST).tobytes()==z.tobytes(),'zoom not nearest exact')
 sn=Image.open(ROOT/'g05_r2_etats_320x240.png').convert('RGB')
 sz=Image.open(ROOT/'g05_r2_etats_x4.png').convert('RGB')
 error(sn.size==(320,240) and sz.size==(1280,960),'state sizes')
 error(sn.resize((1280,960),Image.Resampling.NEAREST).tobytes()==sz.tobytes(),'state zoom not nearest exact')
 # Validate painted grid presence in native pixel region. Ships not counted from baked pixels.
 for x in [21,39,57,75,93,111,129,147,165,183,199]:
  r,g,b=n.getpixel((x,104))
  error(max(r,g,b)>85,f'missing vertical grid cue at x={x}: {(r,g,b)}')
 for y in [53,71,89,107,125,143,161,179,197,215,233]:
  r,g,b=n.getpixel((88,y))
  error(max(r,g,b)>85,f'missing horizontal grid cue at y={y}: {(r,g,b)}')
 with zipfile.ZipFile(ROOT/'g05_r2_defense_layers.ora') as f:
  error('stack.xml' in f.namelist() and 'mergedimage.png' in f.namelist(),'missing ORA layers')
  error(all('data/layer'+str(i)+'.png' in f.namelist() for i in range(3)),'ORA 3 layers required')
  image=Image.open(io.BytesIO(f.read('mergedimage.png'))).convert('RGB')
  error(image.tobytes()==n.tobytes(),'ORA is not faithful to native export')
 # Negative tests: mutations in layout & pixel output must be rejected.
 wrong=dict(layout);wrong['grid_origin']=[22,53]
 try:
  error(wrong['grid_origin']==[21,53],'negative registration')
  raise AssertionError('bad origin not detected')
 except AssertionError as ex:
  error('negative registration' in str(ex),'false negative validation')
 bad=bytearray((ROOT/'g05_r2_defense_320x240.png').read_bytes());bad[-30]^=1
 try:
  error(digest(bytes(bad))==report['outputs']['g05_r2_defense_320x240.png']['sha256'],'negative pixel mutation')
  raise AssertionError('bad output not detected')
 except AssertionError as ex:
  error('negative pixel mutation' in str(ex),'false pixel validation')
 badzoom=bytearray(z.tobytes());badzoom[0]^=1
 error(badzoom!=n.resize((1280,960),Image.Resampling.NEAREST).tobytes(),'negative zoom pixel mutation')
 print('PASS · native board/zoom exact · ORA faithful · G02 17 ships · state truth · 3 negative cases')

if __name__=='__main__':verify()
