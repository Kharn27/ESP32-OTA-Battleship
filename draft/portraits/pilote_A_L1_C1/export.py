#!/usr/bin/env python3
"""PORTRAITS-01 #124 : génération d'un candidat, à lancer à la racine du repo."""
from pathlib import Path
import hashlib
from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = Path(__file__).resolve().parents[3]
SRC = ROOT / "draft/portraits/portrait_A.png"
SHA = "30e92fa1eed23733d812a521c6910390860620f906a0f3529e8410abe0bd9acc"
RECT = (0, 49, 362, 411)
FONT = {
"0":["111","101","101","101","111"],"1":["010","110","010","010","111"],
"2":["110","001","010","100","111"],"3":["110","001","010","001","110"],
"4":["101","101","111","001","001"],"6":["011","100","111","101","111"],
"8":["111","101","111","101","111"],"A":["010","101","111","101","101"],
"B":["110","101","110","101","110"],"C":["011","100","100","100","011"],
"D":["110","101","101","101","110"],"E":["111","100","110","100","111"],
"F":["111","100","110","100","100"],"I":["111","010","010","010","111"],
"L":["100","100","100","100","111"],"M":["101","111","111","101","101"],
"N":["101","111","111","111","101"],"O":["111","101","101","101","111"],
"P":["110","101","110","100","100"],"R":["110","101","110","101","101"],
"S":["011","100","010","001","110"],"T":["111","010","010","010","010"],
"U":["101","101","101","101","111"],"V":["101","101","101","101","010"],
"X":["101","101","010","101","101"]," ":["000","000","000","000","000"],
"/":["001","001","010","100","100"],"-":["000","000","111","000","000"],
}

def hash_file(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def save(im, name):
    path = HERE / name
    im.save(path, "PNG", optimize=True)
    print(name, im.size, len(path.read_bytes()), hash_file(path))

def put_label(im, text, x, y, color):
    for c in text.upper():
        for j, row in enumerate(FONT.get(c, FONT[" "])):
            for i, on in enumerate(row):
                if on == "1":
                    im.putpixel((x+i,y+j),color)
        x += 4

def rect(im,x,y,w,h,color):
    im.paste(color, (x,y,x+w,y+h))

def main():
    assert hash_file(SRC) == SHA, "SHA-256 original PO différent : arrêt"
    with Image.open(SRC) as original:
        assert original.mode=="RGBA" and original.size==(1448,1086)
        hd = original.crop(RECT).copy()
    assert hd.size==(362,362) and hd.getchannel("A").getextrema()[0]<255
    save(hd,"source_HD_coupe.png")
    data=[]; removed=0
    for r,g,b,a in hd.getdata():
        if g>140 and g>r*1.14 and g>b*1.8:
            data.append((0,0,0,0)); removed+=1
        elif a==0: data.append((0,0,0,0))
        else: data.append((r,g,b,a))
    clean=Image.new("RGBA",hd.size)
    clean.putdata(data)
    save(clean,"source_HD_nettoyee.png")
    print("Pixels de frange lime retirés :",removed)
    sizes=(32,48,64)
    sprites={n:clean.resize((n,n),Image.Resampling.BOX) for n in sizes}
    for n in sizes: save(sprites[n],f"pilote_{n}.png")
    board=Image.new("RGBA",(320,240),(12,21,31,255))
    rect(board,4,16,312,100,(29,42,59,255))
    rect(board,4,128,312,100,(232,224,207,255))
    put_label(board,"A1 / FOND SOMBRE",8,6,(237,216,166,255))
    put_label(board,"A1 / FOND CLAIR",8,119,(62,74,76,255))
    for index,n in enumerate(sizes):
        x=9+103*index
        rect(board,x,26,94,87,(12,30,47,255))
        rect(board,x,138,94,85,(246,237,220,255))
        put_label(board,f"{n} PX",x+34,30,(235,215,169,255))
        put_label(board,f"{n} PX",x+34,142,(58,63,69,255))
        board.alpha_composite(sprites[n],(x+(94-n)//2,110-n))
        board.alpha_composite(sprites[n],(x+(94-n)//2,220-n))
    save(board,"comparatif_320x240.png")
    zoom=board.resize((1280,960),Image.Resampling.NEAREST)
    for x,y in [(0,0),(31,13),(117,50),(210,118),(319,239)]:
        assert zoom.getpixel((4*x,4*y))==board.getpixel((x,y))
    save(zoom,"comparatif_320x240_x4_nearest.png")

if __name__=="__main__":
    main()
