#!/usr/bin/env python3
"""BTN-TYPO-01 #157: native-CYD HD-lettering study; no originals modified.
Run from repo root: python draft/etudes/boutons_typographie/generate.py --repo . [--check]
Needs Pillow 12.3.0, numpy 2.2.6, opencv-python-headless 4.12.0.88.
"""
import argparse,hashlib,io,json,subprocess,urllib.request
from pathlib import Path
import cv2,numpy as np
from PIL import Image,ImageDraw,ImageFont

OUT='draft/etudes/boutons_typographie'
P={
 'jouer':dict(hd='assets/boutons/bouton_jouer_original_1107x252_RGBA.png',orig='assets/boutons/bouton_jouer_CYD_140x32_RGBA.png',
 sha='f81c510060af5aeee231bea50df420d04db7b320',oldsha='ddd7f95eee37ba3759a108b92f536c515d8d78e8',dim=(1107,252),cyddim=(140,32),
 roi=(390,35,865,214),center=(625,125),levels=((1.13,1.15),(1.23,1.29)),safe=(320,918,24,228)),
 'palmares':dict(hd='assets/boutons/bouton_palmares_original_1163x250_RGBA.png',orig='assets/boutons/bouton_palmares_CYD_140x30_RGBA.png',
 sha='06a53829d508ae7366338b14a187a655edd6505c',oldsha='0c3ac936ebc0c9cbe20d3e9e0c5fe29be289985d',dim=(1163,250),cyddim=(140,30),
 roi=(356,23,1022,211),center=(685,123),levels=((1.075,1.13),(1.135,1.24)),safe=(325,1035,20,232)),
}
ATK='assets/boutons/attaquer_on_140x35_RGBA.png';ATK_SHA='7ac6667492a96e4a2ad6d7445ab68a504576324b'
ABD_REF='efdc669ef15425c5db342c30de9e07b82ba93dd8';ABD_FILE='draft/etudes/bouton_abandon/exports/abandon_140x35_RGBA.png';ABD_SHA='d3c3d3e0da3700cf5d6bf863990e0e460d7d76a9'
def sh(b):return hashlib.sha256(b).hexdigest()
def gitsha(b):return hashlib.sha1(b'blob '+str(len(b)).encode()+b'\0'+b).hexdigest()
def load(data,sha,wh,desc):
 if gitsha(data)!=sha:raise RuntimeError(f'Wrong actual Git blob {desc}: {gitsha(data)}')
 im=Image.open(io.BytesIO(data));im.load()
 if im.size!=wh or im.mode!='RGBA':raise RuntimeError(f'Unexpected source {desc}: {im.size} {im.mode}')
 return im

def retouch(src,spec,magnitude):
 l,t,r,b=spec['roi']; arr=np.asarray(src);rgb=arr[t:b,l:r,:3].astype(np.int16)
 red,green,blue=rgb[:,:,0],rgb[:,:,1],rgb[:,:,2]
 k=((red>=155)&(green>=127)&(blue>=78)&(red>=blue+16)&(green>=blue+7)).astype(np.uint8)*255
 nc,labels,stats,_=cv2.connectedComponentsWithStats(k,8)
 face=np.zeros_like(k)
 for i in range(1,nc):
  if stats[i,cv2.CC_STAT_AREA]>=32:face[labels==i]=255
 xy=cv2.findNonZero(face)
 if xy is None:raise RuntimeError('Cannot isolate original gold lettering')
 fx,fy,fw,fh=cv2.boundingRect(xy)
 if fw<(240 if src.width<1110 else 390) or fh<65:raise RuntimeError(f'Letter segmentation suspicious {fw}x{fh}')
 radius=8 if src.width<1110 else 7
 hard=cv2.dilate(face,cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(2*radius+1,2*radius+1)))
 if max(hard[0].max(),hard[-1].max(),hard[:,0].max(),hard[:,-1].max())>6:raise RuntimeError('Mask touches ROI edge in '+spec['hd']+' border='+str([int(hard[0].max()),int(hard[-1].max()),int(hard[:,0].max()),int(hard[:,-1].max())]))
 full=np.zeros(arr.shape[:2],np.uint8);full[t:b,l:r]=hard
 soft=cv2.GaussianBlur(full,(0,0),1.1)
 ys,xs=np.where(soft>2);box=(xs.min(),ys.min(),xs.max()+1,ys.max()+1)
 plate=np.array(src);plate[:,:,:3]=cv2.inpaint(arr[:,:,:3],full,5,cv2.INPAINT_TELEA)
 glyph=src.crop(tuple(map(int,box)));glyph.putalpha(Image.fromarray(soft).crop(tuple(map(int,box))))
 sx,sy=spec['levels'][magnitude];w=round(glyph.width*sx);h=round(glyph.height*sy)
 glyph=glyph.resize((w,h),Image.Resampling.LANCZOS)
 cx,cy=spec['center'];px=round(cx-w/2);py=round(cy-h/2)
 sl,sr,st,sb=spec['safe']
 if not (px>sl and px+w<sr and py>st and py+h<sb):raise RuntimeError(f'Expanded text collides with embellishments: {px,py,w,h}')
 result=Image.fromarray(plate,'RGBA');result.alpha_composite(glyph,(px,py))
 output=np.asarray(result);protect=np.zeros(arr.shape[:2],bool)
 protect[:,:sl]=True;protect[:,sr:]=True;protect[:st,:]=True;protect[sb:,:]=True
 if not np.array_equal(output[protect],arr[protect]):raise RuntimeError('A fixed ornament/frame changed')
 return result,dict(gold_bbox=[int(fx+l),int(fy+t),int(fx+l+fw),int(fy+t+fh)],crop=list(map(int,box)),
     scales=[sx,sy],new_size=[w,h],xy=[px,py],changed_in_protected_regions=0)

def png(im):
 f=io.BytesIO();im.save(f,format='PNG',optimize=True,compress_level=9);return f.getvalue()
def native(img,wh):return img.convert('RGBa').resize(wh,Image.Resampling.LANCZOS).convert('RGBA')
def preview(j,p,atk,abd,label):
 out=Image.new('RGBA',(320,240),(16,31,44,255));d=ImageDraw.Draw(out)
 d.rectangle((160,0,319,239),fill=(214,224,220,255));d.line((159,0,159,239),fill=(155,170,172,255))
 ft=ImageFont.load_default();d.text((10,10),'FOND SOMBRE',font=ft,fill=(239,234,207));d.text((170,10),'FOND CLAIR',font=ft,fill=(27,40,49))
 for img,y in ((j,34),(p,80),(atk,128),(abd,175)):
  out.alpha_composite(img,(10,y));out.alpha_composite(img,(170,y))
 d.text((10,224),label,font=ft,fill=(239,234,207));d.text((170,224),'ETUDE / NON VALIDE',font=ft,fill=(27,40,49))
 return out

def main():
 p=argparse.ArgumentParser();p.add_argument('--repo',default='.',type=Path);p.add_argument('--check',action='store_true');a=p.parse_args()
 repo=a.repo.resolve();out=repo/OUT;inputhash={};originals={};cyd={};result={};audits={}
 for name,s in P.items():
  hd=(repo/s['hd']).read_bytes();sm=(repo/s['orig']).read_bytes()
  originals[name]=load(hd,s['sha'],s['dim'],s['hd']);cyd[name]=load(sm,s['oldsha'],s['cyddim'],s['orig'])
  for path,raw in ((s['hd'],hd),(s['orig'],sm)):inputhash[path]={'gitblob':gitsha(raw),'sha256':sh(raw),'bytes':len(raw)}
  for idx,label in enumerate(('a_modere','b_affirme')):
   hd_new,info=retouch(originals[name],s,idx);sm_new=native(hd_new,s['cyddim'])
   result[f'maitres/{name}_{label}_HD_RGBA.png']=hd_new
   result[f'exports/{name}_{label}_{s["cyddim"][0]}x{s["cyddim"][1]}_RGBA.png']=sm_new
   cyd[(name,label)]=sm_new;audits[name+'_'+label]=info
 atkb=(repo/ATK).read_bytes();atk=load(atkb,ATK_SHA,(140,35),ATK)
 try:
  abdb=subprocess.run(['git','show',ABD_REF+':'+ABD_FILE],cwd=repo,capture_output=True,check=True).stdout
 except (subprocess.CalledProcessError,FileNotFoundError):
  with urllib.request.urlopen('https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/'+ABD_REF+'/'+ABD_FILE,timeout=30) as h:abdb=h.read()
 abd=load(abdb,ABD_SHA,(140,35),ABD_FILE)
 inputhash[ATK]={'gitblob':gitsha(atkb),'sha256':sh(atkb),'bytes':len(atkb)}
 inputhash[ABD_REF+':'+ABD_FILE]={'gitblob':gitsha(abdb),'sha256':sh(abdb),'bytes':len(abdb)}
 screens={}
 screens['actuel']=preview(cyd['jouer'],cyd['palmares'],atk,abd,'ACTUEL')
 for level in ('a_modere','b_affirme'):screens[level]=preview(cyd[('jouer',level)],cyd[('palmares',level)],atk,abd,label=level.upper())
 contact=Image.new('RGBA',(960,240))
 for i,(label,im) in enumerate(screens.items()):
  result[f'preuves/{label}_320x240.png']=im
  result[f'preuves/{label}_1280x960_NEAREST.png']=im.resize((1280,960),Image.Resampling.NEAREST)
  contact.alpha_composite(im,(320*i,0))
 result['preuves/contact_3_etats_960x240.png']=contact
 encoded={name:png(im) for name,im in result.items()}
 report={'mission':'BTN-TYPO-01 #157','status':'CANDIDATE / ART_NOT_REVIEWED / ON_DEVICE_UNVERIFIED',
 'method':'Exact approved HD lettering masked and enlarged, original gold and relief retained; only text area inpainted',
 'dependencies':inputhash,'audit':audits,
 'files':{name:{'size':list(result[name].size),'bytes':len(raw),'sha256':sh(raw),'mode':result[name].mode} for name,raw in encoded.items()}}
 rb=(json.dumps(report,indent=2,sort_keys=True,ensure_ascii=False)+'\n').encode()
 if a.check:
  for name,raw in encoded.items():
   f=out/name
   if not f.exists() or f.read_bytes()!=raw:raise RuntimeError('CHECK FAIL: '+str(f))
  if (out/'rapport.json').read_bytes()!=rb:raise RuntimeError('CHECK FAIL: rapport.json')
  print(f'CHECK PASS: {len(encoded)} PNG + rapport.json; no files written; protected decorations pixel-identical')
 else:
  for name,raw in encoded.items():
   f=out/name;f.parent.mkdir(parents=True,exist_ok=True);f.write_bytes(raw)
  (out/'rapport.json').write_bytes(rb)
  print(f'GENERATED {len(encoded)} PNG + rapport.json from approved HD artwork')
 print('masks:',audits)
if __name__=='__main__':main()
