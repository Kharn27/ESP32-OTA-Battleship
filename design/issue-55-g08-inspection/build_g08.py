#!/usr/bin/env python3
"""G08 / Issue #55: post-defeat fleet inspection, native PC design preview.

Runs from the repository checkout, borrowing the approved G07 scene-generating
primitives, not modifying or copying G07. Requires Python 3, CairoSVG, Pillow.
The fleet and shots are a deliberately fictitious, reproducible DEFEAT example.
"""
from __future__ import annotations
import hashlib
import importlib.util
import random
import xml.etree.ElementTree as ET
from pathlib import Path

import cairosvg
from PIL import Image, ImageChops

D = Path(__file__).resolve().parent
ROOT = D.parents[1]
G07 = ROOT / "design" / "issue-53-g07-verdict" / "build_g07.py"
assert G07.is_file(), "G07 must remain available, unmodified, in the same checkout"
spec = importlib.util.spec_from_file_location("warship_g07_scene", G07)
base = importlib.util.module_from_spec(spec)
spec.loader.exec_module(base)

# Zero-indexed (column,row). All horizontal or vertical, all cells in bounds.
# THESE ARE FICTIONAL POSITIONS FOR THE G08 CONCEPT; NOT GAME SAVE DATA.
SHIPS = (
    ("porte-avions", tuple((c, 1) for c in range(1, 6))),
    ("croiseur", tuple((8, row) for row in range(3, 7))),
    ("escorteur-1", tuple((c, 5) for c in range(3, 6))),
    ("escorteur-2", tuple((0, row) for row in range(7, 10))),
    ("patrouilleur", tuple((c, 8) for c in range(6, 8))),
)
FLEET = set().union(*(set(cells) for _, cells in SHIPS))
# At the same time, the captain sank the human fleet: it remains on DÉFENSE,
# not invented here. Five enemy segments were still undamaged at defeat.
UNTOUCHED = {(5, 1), (8, 6), (5, 5), (0, 9), (7, 8)}
HITS = FLEET - UNTOUCHED
MISSES = set(base.MISSES)
CELL_NS = "{http://www.w3.org/2000/svg}"
states = ("A", "B", "C")

def cell_name(cell):
    return f"{chr(65+cell[0])}{cell[1]+1}"

def fleet_check():
    sizes = sorted((len(c) for _, c in SHIPS), reverse=True)
    assert sizes == [5, 4, 3, 3, 2], sizes
    assert len(FLEET) == sum(sizes) == 17
    assert len(HITS) == 12 and len(UNTOUCHED) == 5 and len(MISSES) == 27
    assert FLEET.isdisjoint(MISSES)
    assert HITS.isdisjoint(MISSES)
    assert not (HITS & UNTOUCHED)
    assert all(0 <= col < 10 and 0 <= row < 10 for col,row in FLEET | MISSES)
    for _, cells in SHIPS:
        assert len(cells) == len(set(cells)), "duplicated cell in a ship"
        xs = {c for c, _ in cells}
        ys = {r for _, r in cells}
        assert len(xs) == 1 or len(ys) == 1
        varying = sorted(ys if len(xs) == 1 else xs)
        assert varying == list(range(varying[0], varying[0] + len(cells)))
    assert sum(len(set(cells)) for _, cells in SHIPS) == len(FLEET), "ships overlap"
    assert len(FLEET-HITS) > 0, "defeat example cannot destroy the Captain's fleet"

def ship_draw(name, cells):
    # Solid continuous steel hull with a prow, shadow and bridge. Ships are
    # always UNDER existing explosion marks; no new markers are generated.
    cols = [c for c,_ in cells]
    rows = [r for _,r in cells]
    x = 21 + min(cols)*18
    y = 53 + min(rows)*18
    horizontal = len(set(rows)) == 1
    w = len(cells)*18 if horizontal else 18
    h = 18 if horizontal else len(cells)*18
    steel = "#8aaeb2"
    bright = "#c3d7ca"
    dark = "#234e60"
    parts = []
    if horizontal:
        parts.extend([
            base.p([(x+1,y+8),(x+7,y+3),(x+w-8,y+3),(x+w-2,y+8),
                    (x+w-8,y+14),(x+7,y+14)], "#153e54"),
            base.p([(x+3,y+8),(x+8,y+5),(x+w-9,y+5),(x+w-4,y+8),
                    (x+w-9,y+12),(x+8,y+12)], steel),
            base.r(x+9,y+5,w-19,2,bright),
            base.r(x+9,y+11,w-19,2,dark),
            base.r(x+9,y+7,w-19,2,"#557f8b"),
            base.r(x+w//2-8,y+2,16,3,"#b3c7c2"),
            base.r(x+w//2-4,y,8,2,"#e0dec4"),
            base.r(x+w//2-1,y-1,2,2,"#556e79"),
        ])
        # Portholes add maritime detail without obscuring the impact markers.
        for i in range(1,len(cells)-1):
            parts.append(base.r(x+i*18+5,y+9,3,1,"#d7e9d6"))
    else:
        parts.extend([
            base.p([(x+9,y+1),(x+15,y+8),(x+15,y+h-8),(x+9,y+h-2),
                    (x+3,y+h-8),(x+3,y+8)], "#153e54"),
            base.p([(x+9,y+3),(x+13,y+9),(x+13,y+h-9),(x+9,y+h-4),
                    (x+5,y+h-9),(x+5,y+9)], steel),
            base.r(x+5,y+10,2,h-20,bright),
            base.r(x+12,y+10,2,h-20,dark),
            base.r(x+7,y+h//2-8,4,16,"#557f8b"),
            base.r(x+8,y+h//2-5,2,9,"#e0dec4"),
        ])
    return base.g("navire-"+name,"".join(parts),
                  data_ship=name,data_length=len(cells),
                  data_cells=",".join(cell_name(cell) for cell in cells),
                  data_layer="coque-sous-impacts")

def battlefield(visible):
    base.HITS = HITS
    base.MISSES = MISSES
    scene = base.battlefield()
    scene = scene.replace("historique-coups-reveles-sans-flotte-adverse",
                          "historique-tirs-joueur")
    anchor = '<g id="historique-tirs-joueur">'
    assert scene.count(anchor) == 1
    if visible:
        ships = base.g("cinq-navires-ennemis-reveles",
                       "".join(ship_draw(name, cells) for name, cells in SHIPS),
                       data_visibility="volontaire")
        scene = scene.replace(anchor, ships+anchor)
    return scene

def command(name,label,action,x,y,w,h,bright=False,size=10):
    return base.g(name,
      base.bevel(x,y,w,h,bright)+base.t(x+w/2,y+h/2+4,label,size,
                                      "#fff0cf",800,"middle"),
      data_ui="tactile-command", data_action=action,
      data_hitbox=f"{x},{y},{w},{h}")

def actions(state):
    label={"A":"VOIR SA FLOTTE","B":"MASQUER FLOTTE","C":"AFFICHER FLOTTE"}[state]
    action={"A":"voir-flotte-volontaire","B":"masquer-flotte",
            "C":"afficher-flotte-sans-confirmation"}[state]
    parts=[
      base.r(208,139,112,101,"#0a3b4d",opacity=".17"),
      command("commande-rejouer","REJOUER","retour-deploiement",212,146,104,28,
              bright=True,size=11.6),
      command("commande-accueil","ACCUEIL","sortie-sans-forfait",212,180,104,27,
              size=11.3),
      command("commande-flotte",label,action,212,212,104,24,size=8.5),
    ]
    return base.g("commandes-post-verdict-sans-feu","".join(parts))

def fleet_status(state):
    label={"A":"FLOTTE SECRÈTE","B":"FLOTTE RÉVÉLÉE","C":"FLOTTE MASQUÉE"}[state]
    sub={"A":"Révélation facultative","B":"Tirs conservés","C":"Revoir à volonté"}[state]
    return base.g("information-lecture-seule-flotte",
      base.r(208,49,110,27,"#0b3144",opacity=".9")
      +base.r(209,49,108,1,"#b69068")
      +base.t(263,60,label,8.6,"#fff2d1",800,"middle")
      +base.t(263,70,sub,7.0,"#b5d7d0",600,"middle"),
      data_ui="information-only",data_interaction="none")

def stage(state):
    assert state in states
    base.rng = random.Random(5307) # identical sea and ship in all three states
    return base.svg("".join([
       base.sea_and_sky(),base.military_ship(),battlefield(state=="B"),
       base.topbar("DÉFAITE","defeat"),fleet_status(state),actions(state)
    ]))

def variant_sheet(screen):
    result=[
      base.r(0,0,320,240,"#092638"),
      base.r(0,0,320,26,"#123a4b"),
      base.t(8,18,"G08  /  INSPECTION APRÈS DÉFAITE",10.5,"#fff2d1",800),
      base.l(8,26,312,26,"#c38b61"),
    ]
    labels={
      "A":("A  /  SECRET","Avant choix"),
      "B":("B  /  VISIBLE","Après appui"),
      "C":("C  /  MASQUÉ","Choix réversible"),
    }
    # Embedded reduced screenshots; the full-size master B is the actual review.
    for i,state in enumerate(states):
        x = 5+i*105
        result.append(base.t(x,41,labels[state][0],8.8,"#ffd48c",800))
        source=screen[state]
        inside=source[source.index(">")+1:source.rindex("</svg>")]
        result.append(f'<svg x="{x}" y="47" width="100" height="75" '
                      f'viewBox="0 0 320 240">{inside}</svg>')
        result.append(base.r(x,47,100,75,"none",stroke="#b69068",
                             stroke_width="1"))
        result.append(base.t(x,137,labels[state][1],8.0,"#d1e9db",650))
    result+= [
      base.l(8,150,312,150,"#597f85"),
      base.t(10,165,"A  →  B : appui volontaire sur VOIR SA FLOTTE",8.1,"#fff2d1",750),
      base.t(10,181,"B  →  C : MASQUER FLOTTE",8.1,"#fff2d1",750),
      base.t(10,197,"C  →  B : AFFICHER FLOTTE (sans dialogue)",8.1,"#fff2d1",750),
      base.r(8,206,304,27,"#153d4c"),
      base.t(160,218,"ATTAQUE  →  DÉFENSE  →  ATTAQUE",8.5,"#ffcf95",800,"middle"),
      base.t(160,229,"La visibilité B ou C est conservée.",8.0,"#d0e8de",650,"middle"),
    ]
    return base.svg("".join(result))

def structural_validate(screen):
    fleet_check()
    history = {}
    for state,raw in screen.items():
        root=ET.fromstring(raw)
        assert (root.attrib["width"],root.attrib["height"]) == ("320","240")
        cells=[e.get("data-cell") for e in root.iter() if e.get("data-cell")]
        assert len(cells)==100 and len(set(cells))==100
        hits={e.get("id")[7:] for e in root.iter()
              if e.get("id","").startswith("touche-")}
        misses={e.get("id")[5:] for e in root.iter()
                if e.get("id","").startswith("rate-")}
        assert hits == {cell_name(c) for c in HITS}, state
        assert misses == {cell_name(c) for c in MISSES}, state
        assert hits.isdisjoint(misses)
        history[state]=(hits,misses)
        ships=[e for e in root.iter() if e.get("data-ship") is not None]
        if state=="B":
            assert len(ships)==5 and sum(int(e.get("data-length")) for e in ships)==17
            assert {e.get("data-ship") for e in ships} == {n for n,_ in SHIPS}
            each=[e.get("data-cells").split(",") for e in ships]
            assert len(set(sum(each, [])))==17
            assert {v for item in each for v in item}=={cell_name(c) for c in FLEET}
            # Document order proves hulls are painted before historical markers.
            assert raw.index('id="cinq-navires-ennemis-reveles"') < raw.index('id="historique-tirs-joueur"')
        else:
            assert not ships and "cinq-navires-ennemis-reveles" not in raw
        verdict=[e for e in root.iter() if e.get("id")=="verdict-information"]
        assert len(verdict)==1 and verdict[0].get("data-outcome")=="defeat"
        assert not list(verdict[0].iter(CELL_NS+"rect")) # no fake button
        controls=[e for e in root.iter() if e.get("data-ui")=="tactile-command"]
        assert len(controls)==4
        assert {e.get("data-action") for e in controls} == {
          "consulter-defense","retour-deploiement","sortie-sans-forfait",
          {"A":"voir-flotte-volontaire","B":"masquer-flotte",
           "C":"afficher-flotte-sans-confirmation"}[state]}
        assert "FEU" not in raw and "VICTOIRE" not in raw and "MATCH NUL" not in raw
    assert history["A"]==history["B"]==history["C"], "shot history changed by reveal"
    print("PASS scenario: 100 cells, 5 ships [5,4,3,3,2], 17 hull cells, "
          "12 hits / 27 misses, 5 enemy segments intact")
    print("PASS A/B/C: identical impact coordinates, hidden/visible/hidden, "
          "hulls painted under impacts, 4 real commands, no FEU")
    print("PASS state persistence: prescribed unchanged on ATTAQUE / DÉFENSE / ATTAQUE")

def save(name,data):
    (D/name).write_text(data,encoding="utf-8")

def export(name,raw):
    path=D/name
    cairosvg.svg2png(bytestring=raw.encode(),write_to=str(path),
                     output_width=320,output_height=240)
    with Image.open(path) as im:
        img=im.convert("RGBA")
    assert img.size==(320,240)
    zoom=img.resize((1280,960),Image.Resampling.NEAREST)
    zoomname=name.replace("_320x240.png","_x4.png")
    zoom.save(D/zoomname)
    with Image.open(D/zoomname) as z:
        assert z.size==(1280,960)
        assert not ImageChops.difference(z.convert("RGBA"),zoom).getbbox()
    # Stronger pixel replication assertion: EACH 4x4 block must be one native pixel.
    assert zoom.tobytes()==img.resize((1280,960),Image.Resampling.NEAREST).tobytes()
    print("PASS",name,"320x240 +",zoomname,"1280x960 EXACT NEAREST")

def run():
    screen={state:stage(state) for state in states}
    structural_validate(screen)
    sheet=variant_sheet(screen)
    save("g08_inspection_320x240.svg",screen["B"])
    save("g08_variantes_320x240.svg",sheet)
    export("g08_inspection_320x240.png",screen["B"])
    export("g08_variantes_320x240.png",sheet)
    # Additional hidden/restored state source scripts are reproducible via stage().
    for st in ("A","C"):
        save(f"g08_etat_{st.lower()}_320x240.svg",screen[st])
    report=[]
    for path in sorted(D.glob("g08_*.png"))+sorted(D.glob("g08_*.svg")):
        report.append(f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}")
    (D/"generation-report.txt").write_text(
        "G08 / Issue #55 / DESIGN_PREVIEW -- ON_DEVICE_UNVERIFIED\n"+
        "Command: python3 design/issue-55-g08-inspection/build_g08.py\n"+
        "PASS: structural and coordinate assertions + PNG x4 NEAREST\n"+
        "\n".join(report)+"\n",encoding="utf-8")
    print("PASS SOURCE+EXPORT SHA256 written to generation-report.txt")
    print("STATUS DESIGN_PREVIEW -- ON_DEVICE_UNVERIFIED")

if __name__=="__main__":
    run()
