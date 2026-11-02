#!/usr/bin/env python3
"""G04 — validate screen invariants and export true 320x240 + 4x NEAREST."""
from pathlib import Path
from PIL import Image
import cairosvg
import xml.etree.ElementTree as ET
import re

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "g04_attaque_320x240.svg"
PNG = ROOT / "g04_attaque_320x240.png"
ZOOM = ROOT / "g04_attaque_x4.png"
STATES = ROOT / "g04_etats_annexes.svg"
STATES_PNG = ROOT / "g04_etats_annexes.png"
NS = {"svg": "http://www.w3.org/2000/svg"}

def check(condition, msg):
    if not condition:
        raise AssertionError(msg)

svg = MAIN.read_text(encoding="utf-8")
doc = ET.fromstring(svg)
check(doc.attrib.get("viewBox") == "0 0 320 240", "native viewBox")
check(doc.attrib.get("width") == "320" and doc.attrib.get("height") == "240", "native dimensions")
ids = {group.attrib["id"]: group for group in doc.findall(".//svg:g", NS)}
expected_ids = (
    "grille-attaque-10x10-sans-flotte-adverse",
    "marqueurs-historiques-et-selection-reversible",
    "commande-defense-dernier-tir-et-alerte-separes",
    "action-principale-feu-avec-cible",
    "bandeau-vue-et-abandon-propose",
)
check(all(x in ids for x in expected_ids), "required named editable layers")
grid = ids[expected_ids[0]]
paths = [v.attrib.get("d", "") for v in grid.findall("svg:path", NS)]
cols = [d for d in paths if re.fullmatch(r"M\d+ 42V222", d)]
rows = [d for d in paths if re.fullmatch(r"M18 \d+H198", d)]
check(len(cols) == 9 and len(rows) == 9, "10x10 grid lines")
texts = [v.text or "" for v in doc.findall(".//svg:text", NS)]
check(texts.count("FEU") == 1, "single FEU action")
check("DÉFENSE" in texts and "ATTAQUE" in texts, "view labels")
check(all(x in texts for x in ("QUITTER", "DERNIER TIR", "RATÉ", "ALERTE NON LUE", "COULÉ", "CIBLE", "G5", "TOUCHÉ")), "summary & markers")
check(all(str(k) in [t for t in texts if t.isdigit()] for k in range(1,11)), "ten column numbers")
check(all(chr(ord("A")+k) in texts for k in range(10)), "ten row letters")
check(svg.count('<!-- RATÉ historique C2') == 1, "miss history is distinct")
check(svg.count('<!-- TOUCHÉ historique E7') == 1, "hit history is distinct")
check(svg.count('<!-- CIBLE préparée G5') == 1, "reversible target distinct")
check("M91 155V150" in svg or "M91 155V150" in svg.replace(" ", ""), "selection corners")
check(not doc.findall(".//svg:use", NS), "enemy silhouettes are forbidden on ATTAQUE")
check("nav-porte-avions" not in svg and "nav-croiseur" not in svg, "enemy fleet secret")
check("DERNIER TIR" in [e.text for e in ids[expected_ids[2]].findall("svg:text",NS)], "defense last summary")
check("ALERTE NON LUE" in [e.text for e in ids[expected_ids[2]].findall("svg:text",NS)], "defense persistent alert")
check(STATES.exists(), "state annotations source missing")
state_xml=ET.fromstring(STATES.read_text(encoding="utf-8"))
check(state_xml.attrib.get("viewBox")=="0 0 720 160", "state annotation dimensions")

cairosvg.svg2png(url=str(MAIN), write_to=str(PNG), output_width=320, output_height=240)
cairosvg.svg2png(url=str(STATES), write_to=str(STATES_PNG), output_width=720, output_height=160)
with Image.open(PNG) as native:
    check(native.size == (320,240), "native PNG size")
    nearest = native.convert("RGBA").resize((1280,960), Image.Resampling.NEAREST)
    nearest.save(ZOOM,optimize=True)
    with Image.open(ZOOM) as zoom:
        check(zoom.size==(1280,960), "zoom size")
        check(zoom.convert("RGBA").tobytes()==nearest.tobytes(), "RGBA NEAREST equivalence")
        # Negative control corrupt one RGB channel at a non-origin pixel.
        broken=zoom.convert("RGBA").copy()
        p=broken.getpixel((1,0))
        broken.putpixel((1,0),(p[0]^1,p[1],p[2],p[3]))
        check(broken.tobytes()!=nearest.tobytes(), "negative control must detect RGB corruption")
with Image.open(STATES_PNG) as states_png:
    check(states_png.size==(720,160), "state annotations PNG size")
print("PASS G04 XML + 10x10 + 20 axis labels + separated selection/miss/hit + DÉFENSE last/alert + no enemy hull")
print("PASS G04 PNG 320x240, PNG x4 1280x960 exact RGBA NEAREST, negative RGB control")
print("PASS G04 three-state annotation 720x160 preview")
