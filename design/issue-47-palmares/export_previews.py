#!/usr/bin/env python3
"""G06 — verify the actual editable SVG sources and export 320×240 + exact nearest x4.

PC preview only. No embedded-format or CYD runtime claims.
"""
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image
import cairosvg

HERE = Path(__file__).resolve().parent
MAIN = HERE / "g06_palmares_320x240.svg"
ZERO = HERE / "g06_zero_partie.svg"
NATIVE = HERE / "g06_palmares_320x240.png"
ZOOM = HERE / "g06_palmares_x4.png"
ZERO_PNG = HERE / "g06_zero_partie.png"
SVG_NS = "{http://www.w3.org/2000/svg}"

def check(ok, reason):
    if not ok:
        raise AssertionError(reason)

def load(path):
    root = ET.fromstring(path.read_bytes())
    return root

main = load(MAIN)
zero = load(ZERO)
check(main.get("width") == "320" and main.get("height") == "240", "native 320x240 required")
check(main.get("viewBox") == "0 0 320 240", "native viewBox must not be changed")
check(zero.get("viewBox") == "0 0 252 108", "zero-state is small vignette, not second screen")
texts = [n.text or "" for n in main.findall(f".//{SVG_NS}text")]
ztexts = [n.text or "" for n in zero.findall(f".//{SVG_NS}text")]
must = ["PALMARÈS","EXEMPLE","VS CAPITAINE","PARTIES JOUÉES",
        "VICTOIRES","DÉFAITES","MATCHS NULS","FORFAITS VOLONTAIRES",
        "GLOBAL","ACCUEIL","RETOUR"]
check(all(t in texts for t in must), "required label absent")
check(texts.count("FORFAITS VOLONTAIRES") == 1 and texts.count("GLOBAL") == 1, "single global forfeit")
for forbidden in ("MOUSSE","AMIRAL","VS HUMAIN","JOUEUR 1","CLASSEMENT","EN LIGNE","FEU","JOUER"):
    check(forbidden not in texts, "unavailable mode / invented feature: "+forbidden)
groups = {e.get("id"): e for e in main.findall(f".//{SVG_NS}g") if e.get("id")}
expected_groups = [
    "scene-maritime-originale-g01-ocean-continu",
    "navire-decoratif-ancre-dans-la-mer",
    "journal-de-bord-stats-capitaine",
    "forfaits-volontaires-globaux-distincts",
    "retour-volontaire-vers-accueil",
]
check(all(g in groups for g in expected_groups), "source must preserve meaningful editable groups")
stats={}
for e in main.findall(f".//{SVG_NS}g"):
    key=e.get("data-stat")
    if key:
        check(key not in stats, "duplicate counter "+key)
        value=e.get("data-value")
        check(value is not None and len(value)==2 and value.isdecimal(), "fixture must be two digits")
        check(value in [n.text for n in e.findall(f"{SVG_NS}text")], "rendered number does not match fixture "+key)
        stats[key]=int(value)
check(stats == {
    "parties":12,"victoires":7,"defaites":3,"nuls":2,"forfaits_globaux":1
}, "fictional fixture changed without review")
check(stats["parties"] == stats["victoires"] + stats["defaites"] + stats["nuls"],
      "fictional fixture must balance final results")
check(len(stats)==5, "exactly 5 counters")

check("ÉTAT INITIAL — AUCUNE PARTIE" in ztexts,"zero state explicitly identified")
check(ztexts.count("00")==5, "zero state five zeros without imaginary results")
check(all(t in ztexts for t in ("PARTIES JOUÉES","VICTOIRES","DÉFAITES","MATCHS NULS","FORFAITS VOLONTAIRES — GLOBAL")),
      "zero state labels incomplete")
check(len([e for e in main.findall(f".//{SVG_NS}g") if e.get("id") == "retour-volontaire-vers-accueil"])==1,
      "only one return action")

cairosvg.svg2png(url=str(MAIN),write_to=str(NATIVE),output_width=320,output_height=240)
cairosvg.svg2png(url=str(ZERO),write_to=str(ZERO_PNG),output_width=252,output_height=108)
with Image.open(NATIVE) as native:
    check(native.size==(320,240),"native png size")
    exact = native.convert("RGBA").resize((1280,960),Image.Resampling.NEAREST)
    exact.save(ZOOM,optimize=True)
with Image.open(ZOOM) as zoom:
    check(zoom.size==(1280,960),"zoom dimensions")
    check(zoom.convert("RGBA").tobytes()==exact.tobytes(),"NEAREST RGBA pixel-perfect")
    broken=zoom.convert("RGBA").copy()
    p=broken.getpixel((5,7))
    broken.putpixel((5,7),(p[0]^1,p[1],p[2],p[3]))
    check(broken.tobytes()!=exact.tobytes(),"negative RGB corruption detection")
with Image.open(ZERO_PNG) as small:
    check(small.size==(252,108),"small zero state dimensions")
print("PASS G06: editable SVG 320x240 + five labeled counters + separate GLOBAL + return ACCUEIL")
print("PASS G06: illustrated fiction 12=7+3+2; forfait=01 excluded from parties/results; zero state 5×00")
print("PASS G06: exported native 320x240 + exact RGBA NEAREST 1280x960; negative RGB control")
