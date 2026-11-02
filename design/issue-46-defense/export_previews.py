#!/usr/bin/env python3
"""G05 DÉFENSE — audit du SVG effectivement exporté (preview PC, jamais firmware)."""
from pathlib import Path
import re
import xml.etree.ElementTree as ET
import cairosvg
from PIL import Image

HERE = Path(__file__).resolve().parent
MAIN = HERE / "g05_defense_320x240.svg"
G02 = HERE.parent / "issue-30-navires-flotte" / "g02_grille_320x240.svg"
OUT = HERE / "g05_defense_320x240.png"
ZOOM = HERE / "g05_defense_x4.png"
NS = {"svg": "http://www.w3.org/2000/svg"}
svg = MAIN.read_text(encoding="utf-8")
g02 = G02.read_text(encoding="utf-8")
def require(test, message):
    if not test:
        raise AssertionError(message)
def definitions(s):
    m = re.search(r"<defs>[\s\S]*?</defs>", s)
    require(m is not None, "Missing ship definitions")
    return m.group()
require(definitions(svg) == definitions(g02), "G02 symbol definitions changed")
root = ET.fromstring(svg)
require(root.get("width") == "320" and root.get("height") == "240" and root.get("viewBox") == "0 0 320 240", "Native dimensions")
ids = {x.get("id"): x for x in root.findall(".//svg:g", NS) if x.get("id")}
must = [
    "ocean-continu-crepuscule-g01",
    "grille-defense-10x10-axes",
    "cinq-navires-humains-g02-inchanges",
    "historique-impacts-capitaine-conserves",
    "signaux-maritimes-et-legende",
    "commande-retour-volontaire-attaque",
]
require(all(x in ids for x in must), "Missing editable functional layers")
grid = ids["grille-defense-10x10-axes"]
paths = [x.get("d", "") for x in grid.findall("svg:path", NS)]
require(len([p for p in paths if re.fullmatch(r"M\d+ 42V222",p)]) == 9, "Grid must have 9 vertical dividers")
require(len([p for p in paths if re.fullmatch(r"M18 \d+H198",p)]) == 9, "Grid must have 9 horizontal dividers")
texts = [x.text or "" for x in root.findall(".//svg:text", NS)]
require(all(str(i) in texts for i in range(1,11)), "Missing columns 1-10")
require(all(chr(65+i) in texts for i in range(10)), "Missing rows A-J")
require("DÉFENSE" in texts and "ATTAQUE" in texts and "REVENIR" in texts, "Defense active and voluntary attack action")
require("PATROUILLEUR" in texts and "COULÉ" in texts and "RATÉ" in texts and "TOUCHÉ" in texts, "Miss/hit/sunk labels")
require(not any(x.text=="FEU" for x in root.findall(".//svg:text",NS)), "No firing command in DEFENSE")
require(not any(x.get("id", "").startswith("action-principale-feu") for x in root.findall(".//svg:g",NS)), "No FEU UI layer")
ships = ids["cinq-navires-humains-g02-inchanges"].findall("svg:use", NS)
expected = {"porte-avions":5,"sous-marin":3,"contre-torpilleur":3,"croiseur":4,"patrouilleur":2}
require(len(ships)==5, "Exactly 5 player ships")
occupied = {}
for ship in ships:
    ship_type = ship.get("href", "").removeprefix("#nav-")
    col,row,n = (int(ship.get(key)) for key in ("data-col","data-row","data-length"))
    o=ship.get("data-orient")
    require(ship_type in expected and n==expected[ship_type], "G02 type/length")
    require(o in ("H","V"), "invalid orientation")
    require(col>=0 and row>=0 and (col+n<=10 if o=="H" else row+n<=10), "Out-of-bounds ship")
    transform=ship.get("transform")
    require(transform == f"translate({19+18*col} {42+18*(row if o=='H' else row+n)}) "+("rotate(-90) " if o=="V" else "")+"scale(1)", f"bad ship transform: {ship_type}")
    for i in range(n):
        c,r=(col+i,row) if o=="H" else (col,row+i)
        require((c,r) not in occupied, "Ship overlap")
        occupied[c,r]=ship_type
require(len(occupied)==17 and len(set(occupied.values()))==5, "17 distinct ship cells")
require(occupied[4,5]=="sous-marin" and occupied[5,5]=="contre-torpilleur", "Required legal side contact F5/F6")
require(occupied[7,7]=="patrouilleur" and occupied[8,6]=="croiseur", "Required legal diagonal contact H8/G9")
impacts = ids["historique-impacts-capitaine-conserves"]
actual={}
for node in impacts:
    name=node.get("data-cell")
    require(name not in actual, "Repeated impact")
    require(re.fullmatch(r"[A-J](10|[1-9])", name or "") is not None, "Invalid impact coordinate")
    c,r=int(name[1:])-1,ord(name[0])-65
    state=node.get("data-type")
    require(state in ("RATE","TOUCHE","COULE"),"Unknown impact")
    require((c,r) in occupied if state!="RATE" else (c,r) not in occupied, "Miss/hit on wrong occupancy")
    actual[name]=state
require(actual=={"A7":"RATE","I2":"RATE","F6":"TOUCHE","H7":"TOUCHE","H8":"COULE"}, "Impact fixture changed")
require(occupied[6,7]==occupied[7,7]=="patrouilleur", "Sunk patrol is exactly H7/H8")
require(any(x.get("data-stage")=="dernier" and x.get("data-cell")=="A7" for x in impacts), "Last riposte must be a miss")
require("ALERTE NON LUE" not in texts and "DERNIER TIR" not in texts, "Consulted defense must not repeat attack-side alert/summary")
# Image rendering and proof at exact native and nearest 4x.
cairosvg.svg2png(bytestring=svg.encode("utf-8"),write_to=str(OUT),output_width=320,output_height=240)
with Image.open(OUT) as png:
    require(png.size==(320,240),"Native PNG size")
    zoom=png.convert("RGBA").resize((1280,960),Image.Resampling.NEAREST)
    zoom.save(ZOOM,optimize=True)
with Image.open(ZOOM) as check:
    require(check.size==(1280,960),"Zoom PNG size")
    require(check.convert("RGBA").tobytes()==zoom.tobytes(),"NEAREST 4x RGB/RGBA pixel equivalence")
    changed=check.convert("RGBA").copy()
    px=changed.getpixel((5,3))
    changed.putpixel((5,3),(px[0]^1,px[1],px[2],px[3]))
    require(changed.tobytes()!=zoom.tobytes(),"Negative RGB control")
print("PASS G05: unchanged G02 defs; 5 ships / 17 legal cells / touching fleets")
print("PASS G05: 10x10, full labels, A7/I2 MISS, F6/H7 HIT, H8 sunk, no FEU, no enemy hull")
print("PASS G05: 320x240 native PNG; 1280x960 NEAREST RGBA exact; negative RGB")
