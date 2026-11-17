#!/usr/bin/env python3
"""BTN-ATK-01 — deterministic candidate exports from the TWO 2048x682 user uploads.

These uploads differ in dimensions/hash from the PO main-branch originals! Do not
claim this script proves lineage to draft/Bouton_Attaquer_{on,off}.png on GitHub.

--check compares pixel by pixel without writing anything, including PNG proofs.
--write regenerates all outputs. Requires pillow and numpy.
"""
from __future__ import annotations
import argparse
import hashlib
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT=Path(__file__).resolve().parent
SOURCES={
 'on': ('source_upload_on_2048x682.png','49646da9d0e6f941e448a53f13cba95f8b486aeeaa5b5487c013e8cc3de9392c'),
 'off': ('source_upload_off_2048x682.png','74806a3576363382e9fd891c212439b82b8bb81e17bd66baf349dc04b927cf68'),
}
# Cropped exactly 4:1 around the plate. Shared crop for both states.
CROP=(24,90,2024,590)
SIZES=((140,35),(112,28))

def digest(data):return hashlib.sha256(data).hexdigest()

def read_sources():
    src={}
    for name,(filename, sha) in SOURCES.items():
        p=ROOT/filename
        assert digest(p.read_bytes())==sha, f"SOURCE_UPLOAD_MISMATCH: {filename}"
        im=Image.open(p)
        assert im.size==(2048,682) and im.mode=='RGBA', f"SOURCE_GEOMETRY_MISMATCH: {filename}"
        src[name]=im
    return src

def clean_alpha(arr):
    a=arr.astype(np.float32)
    # Tiny 1-2/255 alpha components beyond the metal plate are artifacts.
    a[a<=2]=0
    # Upload ON image has alpha capped at ~253 by its exporter. Normalize.
    a=np.clip(np.rint(a*255.0/253.0),0,255).astype(np.uint8)
    return a

def off_mask_from_on(on_image):
    """Use ON's transparent silhouette as soft edge template for opaque-black OFF.

    The two references have the same polygonal plate. OFF outline is 4px wider
    and ~4px higher in the supplied images. This transform was determined from
    the >=20 alpha silhouette (ON) versus >1 RGB silhouette (OFF).
    Returns 2048x682 alpha mask, no color or artwork synthesized.
    """
    import PIL.Image as I
    a=I.fromarray(clean_alpha(np.asarray(on_image.getchannel('A'))), 'L')
    # Align xON=[30,2016] to xOFF=[26,2020], yON=[95,584] to yOFF=[91,581].
    # PIL transform uses inverse input-coordinate map.
    sx=(2016-30)/(2020-26); sy=(584-95)/(581-91)
    cx=30 - sx*26; cy=95 - sy*91
    return a.transform(a.size,I.Transform.AFFINE,(sx,0,cx,0,sy,cy),resample=I.Resampling.BICUBIC)

def matte_black_rgb(off_image, mask):
    """Convert opaque image, composited against almost black, to transparent RGBA.

    At semi-transparent boundary reverse compositing from black (RGB/alpha).
    Mask follows shared border, not thresholded red text or cannon shadows.
    Fully transparent pixel RGB set to zero.
    """
    rgb=np.asarray(off_image.convert('RGB')).astype(np.float32)
    a=np.asarray(mask,dtype=np.uint8)
    aa=a.astype(np.float32)/255.0
    # Semi transparent RGB was flattened onto almost black in OFF source.
    # Avoid amplifying <5 alpha (image black background noise).
    rgb=np.where((a>=5)[...,None],np.clip(rgb/np.maximum(aa[...,None],5/255),0,255),0)
    rgba=np.dstack([np.rint(rgb).astype(np.uint8),a])
    rgba[a<4,:]=0
    return Image.fromarray(rgba,'RGBA')

def premul_resize(rgba,size):
    """Alpha-correct LANCZOS downsample, avoiding dark fringe from transparent RGB."""
    arr=np.asarray(rgba,dtype=np.float32)
    alpha=arr[:,:,3:4]/255.0
    premul=np.concatenate([arr[:,:,:3]*alpha,arr[:,:,3:4]],axis=2)
    out=[]
    for c in range(4):
        channel=Image.fromarray(premul[:,:,c].astype(np.float32),'F')
        # Pillow F mode does not implement LANCZOS; scale via 16-bit integral data
        channel=Image.fromarray(np.rint(premul[:,:,c]).clip(0,255).astype(np.uint8),'L')
        out.append(np.asarray(channel.resize(size,Image.Resampling.LANCZOS),dtype=np.float32))
    a=np.clip(out[3],0,255)
    r=np.zeros((*a.shape,4),dtype=np.uint8)
    for c in range(3):
        r[:,:,c]=np.rint(np.where(a>0,out[c]*255/np.maximum(a,1),0)).clip(0,255).astype(np.uint8)
    r[:,:,3]=np.rint(a).astype(np.uint8)
    r[a<2,:]=0
    return Image.fromarray(r,'RGBA')

def theme_preview(sprites, wh):
    """Same-sized ON and OFF over BOTH background colors in a native CYD proof."""
    w,h=wh
    cv=Image.new('RGBA',(320,240),'#11212d')
    d=ImageDraw.Draw(cv)
    d.rectangle((0,120,319,239),fill='#ebe2d3')
    d.line((0,119,319,119),fill='#e7b86e')
    fg=['#e9d9bd','#5e3c27']
    coords=([(10,48),(170,48)] if w==140 else [(20,51),(184,51)])
    y2=171 if w==140 else 174
    coords2=[(x,y2) for x,y in coords]
    for row,placements in enumerate([coords,coords2]):
        d.text((10,8 if row==0 else 129),f'{w} x {h} - ' + ('SOMBRE' if row==0 else 'CLAIR'),fill=fg[row])
        for state,(x,y) in zip(['on','off'],placements):
            d.text((x,y-15),state.upper(),fill=fg[row])
            cv.alpha_composite(sprites[f'attaquer_{state}_{w}x{h}_RGBA.png'],(x,y))
    return cv.convert('RGB')

def make():
    src=read_sources()
    on=src['on'].convert('RGBA')
    on_a=clean_alpha(np.asarray(on.getchannel('A')))
    on_arr=np.asarray(on).copy();on_arr[:,:,3]=on_a;on_arr[on_a==0,:3]=0
    on=Image.fromarray(on_arr,'RGBA')
    off_mask=off_mask_from_on(on)
    off=matte_black_rgb(src['off'],off_mask)
    images={'mask_off_2000x500_L.png': off_mask.crop(CROP)}
    for k,full in [('on',on),('off',off)]:
        cut=full.crop(CROP)
        images[f'source_{k}_cutout_2000x500_RGBA.png']=cut
        for wh in SIZES:
            images[f'attaquer_{k}_{wh[0]}x{wh[1]}_RGBA.png']=premul_resize(cut,wh)
    native=Image.new('RGBA',(320,240),'#11212d')
    d=ImageDraw.Draw(native)
    d.rectangle((0,120,319,239),fill='#ebe2d3')
    d.line((0,119,320,119),fill='#e7b86e',width=1)
    d.text((12,8),'140 x 35  -  fond sombre',fill='#e9d9bd')
    d.text((12,129),'112 x 28  -  fond clair',fill='#5e3c27')
    placements=[('on',140,35,10,50),('off',140,35,170,50),('on',112,28,21,174),('off',112,28,184,174)]
    for name,w,h,x,y in placements:
        d.text((x+3,y-14),name.upper(),fill=('#f3d7b4' if y<120 else '#4c3830'))
        native.alpha_composite(images[f'attaquer_{name}_{w}x{h}_RGBA.png'],(x,y))
    for wh in SIZES:
        preview=theme_preview(images,wh)
        w,h=wh
        images[f'preuve_{w}x{h}_320x240.png']=preview
        images[f'preuve_{w}x{h}_x4_1280x960_NEAREST.png']=preview.resize((1280,960),Image.Resampling.NEAREST)
    images['comparatif_320x240.png']=native.convert('RGB')
    images['comparatif_x4_1280x960_NEAREST.png']=native.convert('RGB').resize((1280,960),Image.Resampling.NEAREST)
    return images

def check_image(name,actual):
    p=ROOT/name
    if not p.exists():raise AssertionError(f'MISSING: {name}')
    with Image.open(p) as recorded:
        if recorded.size!=actual.size or recorded.mode!=actual.mode:raise AssertionError(f'MODE/SIZE_DIFFERENT: {name}')
        left,right=np.asarray(recorded),np.asarray(actual)
        if not np.array_equal(left,right):raise AssertionError(f'PIXELS_DIFFERENT: {name} ({np.count_nonzero(left!=right)} channel mismatches)')
    return p

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    g=ap.add_mutually_exclusive_group(required=True);g.add_argument('--write',action='store_true');g.add_argument('--check',action='store_true')
    args=ap.parse_args()
    imgs=make()
    for name,img in imgs.items():
        if args.write:img.save(ROOT/name,optimize=False,compress_level=9)
        p=check_image(name,img)
        print(f'{"WRITE" if args.write else "CHECK"}_OK {name} {img.size} {img.mode} {len(p.read_bytes())} bytes SHA256 {digest(p.read_bytes())}')
        if name.startswith('attaquer_'):
            a=np.asarray(img.getchannel('A'))
            assert a.min()==0 and a.max()==255
            assert a[0,0]==a[0,-1]==a[-1,0]==a[-1,-1]==0
    for name in ['comparatif_320x240.png']+[f'preuve_{w}x{h}_320x240.png' for w,h in SIZES]:
        x4_name=name.replace('320x240.png','x4_1280x960_NEAREST.png')
        assert np.array_equal(np.asarray(imgs[x4_name]),np.asarray(imgs[name]).repeat(4,0).repeat(4,1)),x4_name
    print('PIXEL_IDENTITY_OK; x4_NEAREST_OK; originals on GitHub NOT verified')
if __name__=='__main__':main()
