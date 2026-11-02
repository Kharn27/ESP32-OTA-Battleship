#!/usr/bin/env python3
"""G07 / Issue #53 - editable PC concept, not a firmware asset.

Generates a maritime pixel-art SVG composition, a focused state sheet and
deterministic PNG previews. Python 3 + CairoSVG + Pillow (host only).
"""
from __future__ import annotations
from pathlib import Path
import math
import random
import xml.etree.ElementTree as ET
import cairosvg
from PIL import Image

D = Path(__file__).resolve().parent
W, H = 320, 240
NAVY = "#071f31"
INK = "#082b3b"
DEEP = "#0a4656"
SEA = "#0a7783"
TEAL = "#49a9ad"
FOAM = "#c6e6d5"
CREAM = "#fff2d1"
COPPER = "#c7763d"
GOLD = "#ffbd6a"
RED = "#ed7448"
FONT = "DejaVu Sans, sans-serif"
rng = random.Random(5307)

def a(**attrs):
    return " ".join(f'{k.replace("_","-")}="{str(v)}"' for k, v in attrs.items() if v is not None)

def r(x,y,w,h,fill,**kw):
    return f'<rect {a(x=x,y=y,width=w,height=h,fill=fill,**kw)}/>'
def l(x1,y1,x2,y2,color,width=1,**kw):
    return f'<line {a(x1=x1,y1=y1,x2=x2,y2=y2,stroke=color,stroke_width=width,**kw)}/>'
def p(points,fill,**kw):
    return f'<polygon {a(points=" ".join(f"{x},{y}" for x,y in points),fill=fill,**kw)}/>'
def t(x,y,label,size=10,fill=CREAM,weight=700,anchor="start",**kw):
    import html
    return f'<text {a(x=x,y=y,font_family=FONT,font_size=size,font_weight=weight,text_anchor=anchor,fill=fill,**kw)}>{html.escape(label)}</text>'
def g(id,content,**kw):
    return f'<g {a(id=id,**kw)}>{content}</g>'
def svg(content):
    return f'<svg xmlns="http://www.w3.org/2000/svg" width="320" height="240" viewBox="0 0 320 240" shape-rendering="crispEdges">{content}</svg>\n'

# Five adversary ships are fully hit in the victory EXAMPLE, but no hull/segment
# affiliation is drawn. The shot history contains exactly 17 hits. Each group
# (5+4+3+3+2) is used for test consistency only, never a visual outline.
FLEET = (
    [(c,1) for c in range(1,6)] +
    [(8,row) for row in range(3,7)] +
    [(c,5) for c in range(3,6)] +
    [(0,row) for row in range(7,10)] +
    [(c,8) for c in range(6,8)]
)
HITS = set(FLEET)
MISSES = {(0,0),(4,0),(7,0),(9,0),(6,1),(9,1),(0,2),(3,2),(7,2),
          (2,3),(5,3),(9,3),(1,4),(6,4),(3,4),(7,4),(0,5),(2,6),
          (4,6),(6,6),(9,6),(2,7),(4,7),(7,7),(9,8),(3,9),(9,9)}
assert len(HITS)==17 and len(MISSES)==27 and not (HITS & MISSES)
assert [5,4,3,3,2] == [5,4,3,3,2]

def sea_and_sky():
    s=[]
    # Seven hand-picked dusk bands: no photographic gradient, contiguous sea.
    for y,h,col in [(0,17,"#162b41"),(17,17,"#26394e"),
                    (34,16,"#4c4b60"),(50,13,"#99666d"),
                    (63,12,"#ce8271"),(75,13,"#f0a26e"),
                    (88,12,"#ad8873")]:
        s.append(r(0,y,W,h,col))
    # Clouds: chipped pixel silhouettes, horizon anchored.
    cloud = [(204,48,44,4),(214,44,22,4),(232,39,18,4),
             (248,50,40,4),(258,46,22,4),(304,55,16,4),
             (0,57,73,4),(21,54,38,3),(87,61,59,3)]
    for x,y,w,h in cloud:
        s.append(r(x,y,w,h,"#53475a",opacity=".65"))
    for x,y,w,h in [(207,52,30,2),(255,53,36,2),(13,59,50,2),(101,63,36,2)]:
        s.append(r(x,y,w,h,"#eaa17c",opacity=".8"))
    # Pixel-stepped sun, drowned partially behind the horizon.
    cx,cy,rad=283,78,21
    for dy in range(-rad,rad+1,3):
        dx=int(math.sqrt(max(0,rad*rad-dy*dy)))
        s.append(r(cx-dx,cy+dy,2*dx+1,3,"#f7bc79"))
    for dy in range(-16,17,3):
        dx=int(math.sqrt(max(0,16*16-dy*dy)))
        s.append(r(cx-dx,cy+dy,2*dx+1,3,"#ffe09c"))
    s.append(r(0,95,320,145,"#085363"))
    for y,h,c in [(95,11,"#177582"),(106,11,"#096c79"),(117,15,"#0a7783"),
                  (132,16,"#086c79"),(148,19,"#116e78"),(167,17,"#0a6876"),
                  (184,19,"#0a6170"),(203,18,"#095864"),(221,19,"#084a5b")]:
        s.append(r(0,y,320,h,c))
    # Sea texture is staggered, with current and shade: purposeful broken crests.
    waves=[]
    rr=random.Random(5307)
    for i in range(345):
        x=rr.randrange(0,317)
        y=rr.randrange(98,239)
        n=rr.choice([3,5,7,9,12,16])
        c=rr.choice(["#2d8790","#3b9c9c","#0b5364","#0d6272","#207780","#6ab4aa"])
        waves.append(r(x,y,n,rr.choice([1,1,1,2]),c,opacity=rr.choice([".48",".62",".78"])))
    s.append(g("mer-vagues-discontinues", "".join(waves)))
    # Sunset in reflective, broken pieces, not a painted vertical beam.
    rr=random.Random(53)
    reflections=[]
    for y in range(101,155,3):
        taper=int((y-101)*0.39)+9
        for _ in range(rr.randint(1,3)):
            x=283+rr.randrange(-taper,taper+1)
            w=rr.randrange(3,10)
            reflections.append(r(x,y,w,1,rr.choice(["#f7bd80","#e49e71","#b8b47e"]),opacity=".75"))
    s.append(g("reflets-coucher-soleil", "".join(reflections)))
    # Rocky shoal on distant horizon, no second hidden warship.
    s.append(p([(203,99),(208,96),(217,97),(224,94),(237,96),(244,101),
                (244,107),(203,107)],"#183e50"))
    s.append(r(304,94,4,13,"#12394a"))
    s.append(r(303,93,6,3,"#2c4b54"))
    s.append(r(305,90,2,3,"#f8cd89"))
    return g("ambiance-maritime-continue","".join(s))

def military_ship():
    s=[]
    # Friendly escort / illustrative silhouette on the edge of the battle map.
    # Perspective: all pieces rest on one horizontal waterline.
    # Smoke above exhaust, never a floating sticker or a leisure sail.
    for x,y,w,h,c in [(271,68,4,3,"#7a7479"),(274,64,5,4,"#81777b"),
                       (277,60,6,3,"#a38784"),(279,56,6,3,"#af9288"),
                       (285,57,5,2,"#c29a86")]:
        s.append(r(x,y,w,h,c,opacity=".78"))
    # Hull's far hull edge and pointed bow (right). Steel, not timber.
    s.append(p([(216,115),(224,110),(309,110),(315,114),(307,125),
                (283,130),(234,127),(222,122)],"#081e2c"))
    s.append(p([(218,115),(235,113),(308,113),(314,115),(305,121),
                (279,125),(237,123),(225,120)],"#436374"))
    s.append(p([(223,118),(241,119),(302,119),(307,117),(300,123),
                (278,125),(239,123)],"#273f51"))
    s.append(l(228,114,307,114,"#d7a979",1))
    s.append(l(237,122,281,125,"#8d9f9c",1))
    # Deck and forward/rear gun mounts. Barrels point over open water.
    s.append(r(230,106,69,5,"#223b4a"))
    s.append(r(233,104,21,3,"#415e6b"))
    s.append(r(239,102,7,3,"#7e9390"))
    s.append(r(213,104,29,2,"#87999c"))
    s.append(r(297,102,7,4,"#506b76"))
    s.append(r(304,104,11,2,"#879c9a"))
    # Bridge stacks, radar mast, chimney and windows.
    s.append(r(262,95,33,11,"#294453"))
    s.append(r(265,91,26,6,"#4d6471"))
    s.append(r(271,85,18,6,"#364d5e"))
    s.append(r(276,79,6,7,"#6a7679"))
    s.append(r(271,73,4,15,"#2a404e"))
    s.append(l(273,73,286,73,"#c6c2aa",1))
    s.append(r(283,71,9,2,"#70878a"))
    s.append(r(290,73,2,4,"#344b54"))
    s.append(r(257,90,7,15,"#34505d"))
    s.append(r(258,88,5,3,"#a37161"))
    for x in [269,275,281,287]:
        s.append(r(x,94,4,3,"#f2c28d"))
        s.append(r(x,98,4,1,"#1e3948"))
    # Mast/antenna scale-reasonable.
    s.append(l(278,66,278,79,"#314554",1))
    s.append(l(273,70,284,70,"#9ba8a0",1))
    s.append(r(277,64,3,3,"#eac18f"))
    # Foam meets hull on exact water surface, repeated uneven strokes.
    for x,y,w,col in [(213,124,15,"#80c2b9"),(221,127,19,"#b2d8c8"),
                      (239,130,33,"#77b7aa"),(266,132,25,"#d2d7ba"),
                      (287,130,21,"#acd3bd"),(304,124,14,"#93c9bb"),
                      (222,132,19,"#3eaca6"),(274,135,36,"#468f8e")]:
        s.append(r(x,y,w,2,col))
    return g("escorte-amie-silhouette-militaire","".join(s),data_scene_role="navire-ami-illustratif")

def hit_mark(cx,cy,last=False):
    s=[]
    # Impact belongs *inside* a gridded sea cell; orange core and four foam lobes.
    c="#ffcd80" if last else "#ee9760"
    s += [r(cx-5,cy-2,10,5,"#173f4e"),
          r(cx-4,cy-4,8,2,"#7caead"),
          r(cx-6,cy+1,3,3,"#cbdfcd"),
          r(cx+4,cy+1,3,3,"#b4d8c3"),
          r(cx-3,cy-5,2,3,"#cce5d4"),
          r(cx+2,cy-5,2,3,"#a4d3c7"),
          r(cx-3,cy-1,6,4,RED),
          r(cx-1,cy-3,3,5,c)]
    if last: s += [r(cx-1,cy-7,3,2,"#fff3d8"),r(cx-7,cy-2,2,3,"#ffe0a4")]
    return "".join(s)

def miss_mark(cx,cy):
    return "".join([r(cx-5,cy+3,9,1,"#83bdb9"),
       r(cx-2,cy+1,5,1,"#d6eee0"),r(cx+1,cy-5,2,2,"#cde8df"),
       r(cx+3,cy-2,1,2,"#8ebfc3"),r(cx-4,cy,2,1,"#b6dce0"),
       r(cx-5,cy+1,2,1,"#b1d5cd"),r(cx+5,cy+3,2,1,"#9fcfc2")])

def battlefield():
    s=[r(20,52,181,181,"#093d50",opacity=".78"),
       r(20,52,181,181,"none",stroke="#c38b61",stroke_width=1.5)]
    # A-J / 1-10 stay outside the tappable cells, proper native scale.
    for col,char in enumerate("ABCDEFGHIJ"):
        s.append(t(29+18*col,49,char,9.0,CREAM,800,"middle"))
    for row in range(10):
        s.append(t(17,63+18*row,str(row+1),8.7,CREAM,800,"end"))
    cells=[]
    for row in range(10):
        for col in range(10):
            x=21+col*18; y=53+row*18
            cells.append(r(x,y,18,18,"#126578",opacity=".13",data_cell=f"{chr(65+col)}{row+1}"))
            # Fine division, deliberately less contrast than impacts.
            cells.append(l(x,y,x+18,y,"#5da9a7",1,opacity=".86"))
            cells.append(l(x,y,x,y+18,"#5da9a7",1,opacity=".86"))
    s.append(g("grille-10x10-100-cases","".join(cells)))
    s.append(l(201,53,201,233,"#b28f69",1))
    s.append(l(21,233,201,233,"#b28f69",1))
    # Broken sea glints inside empty cells, horizontal only.
    rr=random.Random(155)
    ambient=[]
    for _ in range(85):
        col=rr.randrange(10);row=rr.randrange(10)
        if (col,row) in HITS or (col,row) in MISSES: continue
        xx=22+18*col+rr.randrange(1,11)
        yy=53+18*row+rr.randrange(3,14)
        ambient.append(r(xx,yy,rr.choice([2,3,4]),1,rr.choice(["#75b9b0","#286b7f"]),opacity=".72"))
    s.append(g("mouvements-mer-en-cases","".join(ambient)))
    marks=[]
    for col,row in sorted(MISSES):
        marks.append(g(f"rate-{chr(65+col)}{row+1}",miss_mark(30+col*18,62+row*18)))
    for col,row in sorted(HITS):
        marks.append(g(f"touche-{chr(65+col)}{row+1}",hit_mark(30+col*18,62+row*18,last=(col,row)==(8,6))))
    s.append(g("historique-coups-reveles-sans-flotte-adverse","".join(marks)))
    # No target highlight is drawn after result.
    return g("zone-attaque-lecture-seule","".join(s),data_interaction="aucun-tir-post-verdict")

def bevel(x,y,w,h,bright=True):
    if bright:
        dark="#4b322e"; fill="#b76137"; trim="#f7c783"; shade="#7e412d"
    else:
        dark="#102a3b"; fill="#153d4c"; trim="#b69068"; shade="#0a2a3d"
    return "".join([r(x,y,w,h,dark),r(x+2,y+2,w-4,h-4,fill),
                    r(x+3,y+3,w-6,2,trim),
                    r(x+3,y+h-5,w-6,2,shade),
                    r(x,y,w,h,"none",stroke=trim,stroke_width="1")])

def topbar(verdict="VICTOIRE",outcome="victory"):
    s=[r(0,0,320,42,"#091f31"),r(0,39,320,3,"#996c52"),
       r(0,42,320,3,"#1a5b6b")]
    s.append(t(13,25,"ATTAQUE",12,CREAM,800))
    s.append(bevel(101,4,103,34,False))
    s.append(t(152,18,"DÉFENSE",10.5,CREAM,800,"middle"))
    s.append(t(152,31,"RIPOSTE : RATÉ",7.1,"#aee0cc",600,"middle"))
    # Identifiable result chip is part of the surrounding copper sea UI.
    s.append(bevel(208,4,108,34,False))
    accent={"victory":"#ffd48c","defeat":"#ffc0a4","draw":"#cfdfd2"}[outcome]
    s.append(r(211,7,3,28,accent))
    s.append(t(263,20,verdict,10.8,accent,900,"middle"))
    s.append(t(263,31,"VS CAPITAINE",7.1,CREAM,700,"middle"))
    return g("barre-resultat-navigation","".join(s))

def actions(defeat=False):
    s=[r(208,139,112,101,"#0a3b4d",opacity=".17")]
    s.append(g("commande-rejouer",bevel(212,146,104,28,True)
        +t(264,164,"REJOUER",11.6,"#fff0cf",800,"middle"),data_action="retour-deploiement"))
    s.append(g("commande-accueil",bevel(212,180,104,27,False)
        +t(264,197,"ACCUEIL",11.3,CREAM,800,"middle"),data_action="sortie-sans-forfait"))
    if defeat:
        s.append(g("commande-revelation-reserve-g08",bevel(212,212,104,24,False)
            +t(264,227,"VOIR SA FLOTTE",9.1,"#f5dba7",800,"middle"),
            data_action="demande-volontaire-aucun-revelation-auto"))
    else:
        # Empty sea remains actual illustrated sea, not a disabled faux button.
        s.append(g("eau-libre-sans-commande-revelation",
            r(216,216,39,1,"#67ada4")+r(269,220,35,1,"#3a8a89")
            +r(232,230,48,1,"#428684")))
    return g("commandes-post-verdict-sans-feu","".join(s))

def main(verdict="VICTOIRE",outcome="victory"):
    return svg("".join([sea_and_sky(),military_ship(),battlefield(),
                        topbar(verdict,outcome),actions(outcome=="defeat")]))

def states_sheet():
    ss=[r(0,0,320,240,"#0a2639"),
        r(0,0,320,24,"#133d4e"),
        t(8,16,"G07  /  ÉTATS DU VERDICT",11,CREAM,800),
        l(8,23,311,23,"#c58a56",1)]
    # Initial announcement: illustrative example OVER battle, not a new page.
    ss.append(r(7,31,182,96,"#075364"))
    # miniature field beneath a single on-scene visual shout
    for ix in range(0,11):
        ss.append(l(11+16*ix,36,11+16*ix,122,"#6eabad",.7,opacity=".55"))
    for iy in range(0,6):
        ss.append(l(11,36+16*iy,183,36+16*iy,"#6eabad",.7,opacity=".55"))
    for cx,cy in [(25,55),(60,73),(108,62),(155,99),(44,110)]:
        ss.append(hit_mark(cx,cy))
    ss.append(r(15,57,168,45,"#0a2637",opacity=".91"))
    ss.append(r(15,57,168,2,"#e1a365"))
    ss.append(r(15,100,168,2,"#8c6145"))
    ss.append(t(99,86,"VICTOIRE",20,"#ffde9d",900,"middle"))
    ss.append(t(198,47,"ANNONCE INITIALE",9.6,GOLD,800))
    ss.append(t(198,63,"Sur la grille",8.9,CREAM,600))
    ss.append(t(198,77,"après les deux",8.9,CREAM,600))
    ss.append(t(198,91,"impacts révélés",8.9,CREAM,600))
    ss.append(t(198,113,"Sans bouton OK",8.5,"#aee0cb",700))
    # all possible chip messages - not three whole scenes.
    ss.append(t(8,144,"FORMAT COMPACT PERSISTANT",9.1,CREAM,800))
    for x,name,color in [(7,"VICTOIRE","#ffd48c"),(111,"DÉFAITE","#ffc0a4"),(215,"MATCH NUL","#cfdfd2")]:
        ss.append(bevel(x,150,98,27,False))
        ss.append(r(x+2,153,3,21,color))
        ss.append(t(x+51,167,name,10.1,color,900,"middle"))
    ss.append(l(8,184,312,184,"#537780",1))
    ss.append(t(8,197,"DÉFAITE SEULEMENT  ·  APRÈS FORMAT COMPACT",8.1,CREAM,800))
    ss.append(bevel(8,205,140,27,False))
    ss.append(t(78,222,"VOIR SA FLOTTE",10.5,"#f5dba7",800,"middle"))
    ss.append(t(157,213,"Sur ATTAQUE uniquement",8.0,"#c5e6d5",650))
    ss.append(t(157,224,"Aucune révélation auto",8.0,"#c5e6d5",650))
    ss.append(t(157,235,"Détail / masque : G08",8.0,"#c5e6d5",650))
    return svg("".join(ss))

def save_svg(file,text):
    (D/file).write_text(text,encoding="utf-8")

def export(file,svg_text):
    out = D/file
    cairosvg.svg2png(bytestring=svg_text.encode("utf-8"),write_to=str(out),
                     output_width=320,output_height=240)
    with Image.open(out) as src:
        rgba=src.convert("RGBA")
        assert rgba.size==(320,240),file
        zoom=rgba.resize((1280,960),Image.Resampling.NEAREST)
        zoom.save(D/file.replace("_320x240.png","_x4.png"))
        with Image.open(D/file.replace("_320x240.png","_x4.png")) as check:
            assert check.size==(1280,960)
            assert check.convert("RGBA").tobytes()==zoom.tobytes()
            assert check.convert("RGBA").resize((320,240),Image.Resampling.NEAREST).tobytes()==rgba.tobytes()
    print("PASS", file,"320x240, 1280x960, x4 EXACT NEAREST")

def validate():
    root=ET.parse(D/"g07_verdict_320x240.svg").getroot()
    cells=[e for e in root.iter() if e.get("data-cell") is not None]
    assert len(cells)==100 and len({c.get("data-cell") for c in cells})==100
    whole=(D/"g07_verdict_320x240.svg").read_text(encoding="utf-8")
    assert "touche-I7" in whole and "navire-ami-illustratif" in whole
    assert "FEU" not in whole and "VOIR SA FLOTTE" not in whole
    assert "REJOUER" in whole and "ACCUEIL" in whole and "DÉFENSE" in whole
    assert "aucun-tir-post-verdict" in whole
    variants=(D/"g07_variantes_320x240.svg").read_text(encoding="utf-8")
    for phrase in ("VICTOIRE","DÉFAITE","MATCH NUL","VOIR SA FLOTTE","APRÈS FORMAT COMPACT"):
        assert phrase in variants,phrase
    # test the defeat rendering independently without introducing a third full-size file
    d=main("DÉFAITE","defeat")
    assert "VOIR SA FLOTTE" in d and "aucun-revelation-auto" in d
    assert "VOIR SA FLOTTE" not in main("MATCH NUL","draw")
    assert "<svg" in d
    print("PASS grid=100, hits=17, misses=27, secret=non-revealed, 3 results, post-result actions")

def run():
    one=main()
    sheet=states_sheet()
    save_svg("g07_verdict_320x240.svg",one)
    save_svg("g07_variantes_320x240.svg",sheet)
    validate()
    export("g07_verdict_320x240.png",one)
    export("g07_variantes_320x240.png",sheet)
    print("STATUS DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED")

if __name__=="__main__":
    run()
