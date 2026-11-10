#!/usr/bin/env python3
"""G05-R2 DÉFENSE — editable PC-only native compositor.

HD source is a user-supplied FLATTENED painting. The script is genuinely editable
for board registration, composition, G05-state cards and exports, and an ORA
has separate raster layers. It does NOT extract firmware-ready ship sprites,
and cannot edit baked ship/text artwork as independent vector objects.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import hashlib, json, io, zipfile, xml.etree.ElementTree as ET
import PIL

ROOT=Path(__file__).resolve().parent
REF=ROOT/'reference_defense_1448x1086_lossless.webp'
WIDTH,HEIGHT=320,240
GX,GY,CELL=21,53,18
N=10
# Registration from exact painted master grid, not G04's original pixels.
HD_GRID=(117,269,897,1009)
GUTTER_Y=41
SEAM_X=207
VESSELS={
 'porte_avions':('horizontal','B2',5),
 'sous_marin':('horizontal','C6',3),
 'contre_torpilleur':('vertical','F5',3),
 'croiseur':('vertical','I4',4),
 'patrouilleur':('horizontal','G8',2)
}
MISSES=('I2','A7')
HITS=('F6',)
SUNK=('G8','H8')
LAST_SHOT=('A7','RATÉ')
LAST_SUNK='PATROUILLEUR'


def sha256(path):
 return hashlib.sha256(path.read_bytes()).hexdigest()

def occupied_cells():
 out={}
 for name,(orient,start,length) in VESSELS.items():
  col=ord(start[0])-ord('A');row=int(start[1:])-1
  cells=[f'{chr(ord("A")+col+(i if orient=="horizontal" else 0))}{row+1+(i if orient=="vertical" else 0)}' for i in range(length)]
  out[name]=cells
 return out

def make_native():
 src=Image.open(REF).convert('RGB')
 assert src.size==(1448,1086)
 # Whole painting for top and right: no random filters or disjoint textures.
 backdrop=src.resize((WIDTH,HEIGHT),Image.Resampling.LANCZOS)
 # Mathematical pixel registration of the painted 10x10 grid to G04 bounds.
 hx0,hy0,hx1,hy1=HD_GRID
 sx=(hx1-hx0)/(CELL*N); sy=(hy1-hy0)/(CELL*N)
 left=round(hx0-GX*sx)
 upper=round(hy0+(GUTTER_Y-GY)*sy)
 right=round(left+SEAM_X*sx)
 bottom=round(hy0+(HEIGHT-GY)*sy)
 port=src.crop((left,upper,right,bottom)).resize((SEAM_X,HEIGHT-GUTTER_Y),Image.Resampling.LANCZOS)
 layer=Image.new('RGBA',(WIDTH,HEIGHT),(0,0,0,0))
 layer.paste(port,(0,GUTTER_Y))
 trim=Image.new('RGBA',(WIDTH,HEIGHT),(0,0,0,0))
 d=ImageDraw.Draw(trim)
 # Board/controls separation only; no gameplay stroke painted across a hull.
 d.line((205,45,205,239),fill='#4e271f',width=2)
 d.line((206,45,206,239),fill='#bb8544',width=1)
 native=Image.alpha_composite(Image.alpha_composite(backdrop.convert('RGBA'),layer),trim).convert('RGB')
 return native,(backdrop,layer,trim)

def font(size):
 path='/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf'
 if Path(path).exists():return ImageFont.truetype(path,size)
 return ImageFont.load_default()

def make_states(native):
 # Targeted UX vignettes, deliberately NOT alternative screen references.
 s=Image.new('RGB',(320,240),'#221B22'); d=ImageDraw.Draw(s)
 field=native.crop((204,50,318,222)).resize((320,240),Image.Resampling.LANCZOS)
 s=Image.blend(s,field,0.34);d=ImageDraw.Draw(s)
 d.rectangle((0,0,319,26),fill='#3C2730',outline='#E7BC79',width=1)
 d.text((9,7),'DÉFENSE · ÉTATS DE CONSULTATION',font=font(11),fill='#FFF1C8')
 items=[
  ('A  RIPOSTE RÉVÉLÉE',['ATTAQUE : A7 RATÉ','Impacts conservés']),
  ('B  ALERTE NON LUE',['ATTAQUE : ! COULÉ','Résumé non acquitté']),
  ('C  OUVERTURE DÉFENSE',['Lecture volontaire','Alerte acquittée']),
  ('D  RETOUR ATTAQUE',['Sélection G5 conservée','Sans nouveau FEU'])
 ]
 for k,(name,lines) in enumerate(items):
  x=4+(k%2)*159; y=31+(k//2)*103
  d.rounded_rectangle((x,y,x+153,y+96),radius=4,fill='#4D2D25',outline='#D8A555',width=2)
  d.rounded_rectangle((x+4,y+4,x+149,y+92),radius=3,fill='#14465B',outline='#4EC0C4',width=1)
  d.rectangle((x+5,y+5,x+148,y+25),fill='#603927')
  d.text((x+8,y+11),name,font=font(9),fill='#FFF1C8')
  d.line((x+12,y+34,x+143,y+34),fill='#E6B773',width=1)
  for j,t in enumerate(lines):
   d.text((x+13,y+44+j*15),t,font=font(9),fill='#F7E2C8')
 d.line((10,236,310,236),fill='#D6AB71',width=1)
 return s

def save_ora(layers):
 # OpenRaster is editable raster stack; imported painted text and ships stay baked.
 (backdrop,port,trim)=layers
 out=ROOT/'g05_r2_defense_layers.ora'
 merged=Image.alpha_composite(Image.alpha_composite(backdrop.convert('RGBA'),port),trim)
 stack=ET.Element('image',{'w':'320','h':'240','name':'CYD WARSHIP G05-R2','version':'0.0.3'})
 st=ET.SubElement(stack,'stack')
 sources=[('Séparation laiton (raster éditable)',trim),('Plateau recalé 180x180 (raster éditable)',port),('Fond et panneau peints (raster éditable)',backdrop.convert('RGBA'))]
 with zipfile.ZipFile(out,'w') as z:
  def put(name,data,method=zipfile.ZIP_DEFLATED):
   entry=zipfile.ZipInfo(name,date_time=(2026,11,4,0,0,0))
   entry.compress_type=method
   z.writestr(entry,data)
  put('mimetype','image/openraster',zipfile.ZIP_STORED)
  for i,(label,img) in enumerate(sources):
   srcpath=f'data/layer{i}.png'; temp=io.BytesIO();img.save(temp,format='PNG',optimize=True)
   put(srcpath,temp.getvalue())
   ET.SubElement(st,'layer',{'name':label,'src':srcpath,'x':'0','y':'0','opacity':'1.0','visibility':'visible','composite-op':'svg:src-over'})
  buf=io.BytesIO();merged.save(buf,format='PNG');put('mergedimage.png',buf.getvalue())
  put('stack.xml',ET.tostring(stack,encoding='utf-8',xml_declaration=True))
 return out

def main():
 assert set(MISSES).isdisjoint(HITS) and set(HITS).isdisjoint(SUNK)
 cells=occupied_cells()
 flat=[c for a in cells.values() for c in a]
 assert len(flat)==17 and len(set(flat))==17
 assert set(HITS+SUNK).issubset(flat) and all(v not in flat for v in MISSES)
 assert set(SUNK)==set(cells['patrouilleur'])
 native,layers=make_native()
 p=ROOT/'g05_r2_defense_320x240.png';native.save(p,optimize=True)
 z=ROOT/'g05_r2_defense_x4.png';native.resize((1280,960),Image.Resampling.NEAREST).save(z,optimize=True)
 states=make_states(native)
 s=ROOT/'g05_r2_etats_320x240.png';states.save(s,optimize=True)
 sz=ROOT/'g05_r2_etats_x4.png';states.resize((1280,960),Image.Resampling.NEAREST).save(sz,optimize=True)
 ora=save_ora(layers)
 layout={'native_size':[320,240],'grid_origin':[GX,GY],'grid_cells':[10,10], 'cell_size':[CELL,CELL], 'grid_size':[180,180], 'hd_grid_reference_pixels':list(HD_GRID), 'ships':cells, 'misses':MISSES, 'hits':HITS,'sunk_cells':SUNK,'last_shot':LAST_SHOT,'fleet_message':LAST_SUNK+' COULÉ','navigation':'ATTAQUE','fire_button':False,'approved_by_po':False,'on_device_verified':False,'board_layer_y':GUTTER_Y,'board_panel_seam_x':SEAM_X}
 (ROOT/'layout.json').write_text(json.dumps(layout,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
 src_files=['reference_defense_1448x1086_lossless.webp','build_g05_r2.py','verify_g05_r2.py','layout.json',p.name,z.name,s.name,sz.name,ora.name]
 report={'status':'DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED','artistic_po_approval':'PENDING','pillow_version':PIL.__version__,'render':'Lanczos HD-to-native; integer x4 exactly NEAREST','outputs':{n:{'sha256':sha256(ROOT/n),'bytes':(ROOT/n).stat().st_size} for n in src_files},'source_original_png':{'sha256':'c9123b2363366aa6e478e6612e55e5844a5ce0fb405077f42427bcac0297f494','bytes':3944009,'alpha':'255 everywhere','conversion':'lossless WebP RGB pixels checked identical to supplied PNG'},'external_resources':['User-provided original raster, CC asset stored separately; no stock asset / font redistributed']}
 (ROOT/'generation-report.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
 print('PASS 320×240 / 1280×960 strict NEAREST / ORA layers / 17 fleet cells')
 for name,meta in report['outputs'].items():print(name,meta['bytes'],meta['sha256'][:16])

if __name__=='__main__':main()
