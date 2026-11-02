#!/usr/bin/env python3
"""G03: validate the actual SVG placement and export canonical native/nearest PNGs."""
from pathlib import Path
import re
import cairosvg
from PIL import Image

HERE = Path(__file__).resolve().parent
SVG = HERE / "g03_deploiement_320x240.svg"
G02 = HERE.parent / "issue-30-navires-flotte" / "g02_grille_320x240.svg"
NATIVE = HERE / "g03_deploiement_320x240.png"
ZOOM = HERE / "g03_deploiement_x4.png"
svg = SVG.read_text(encoding="utf-8")
g02 = G02.read_text(encoding="utf-8")
def definitions(text):
    m = re.search(r"<defs>[\s\S]*?</defs>", text)
    assert m, "missing defs"
    return m.group()
assert definitions(svg) == definitions(g02), "G02 ship symbols changed"
assert 'viewBox="0 0 320 240"' in svg
pattern = r'<use\s+href="#nav-([\w-]+)"\s+data-col="(\d+)"\s+data-row="(\d+)"\s+data-length="(\d+)"\s+data-orient="([HV])"\s+transform="([^"]+)"/>'
ships = re.findall(pattern, svg)
expected = {"porte-avions": 5, "sous-marin": 3, "contre-torpilleur": 3, "croiseur": 4, "patrouilleur": 2}
assert len(ships) == 5 and {name: int(n) for name,_,_,n,_,_ in ships} == expected, ships
occupied, placements = {}, {}
for name, sc, sr, sn, orientation, transform in ships:
    col, row, n = map(int, (sc, sr, sn))
    m = re.fullmatch(r"translate\((\d+) (\d+)\) (rotate\(-90\) )?scale\(1\)", transform)
    assert m, transform
    x,y = map(int, m.group(1,2))
    assert 0 <= col < 10 and 0 <= row < 10
    assert (col+n <= 10 if orientation=="H" else row+n <= 10)
    assert x == 19+18*col
    assert y == (42+18*row if orientation=="H" else 42+18*(row+n))
    assert bool(m.group(3)) == (orientation=="V")
    cells = {(col+i,row) if orientation=="H" else (col,row+i) for i in range(n)}
    assert not any(cell in occupied for cell in cells)
    occupied.update({cell:name for cell in cells})
    placements[name] = cells
assert len(occupied)==17
assert any(abs(a-c)+abs(b-d)==1 for a,b in placements["sous-marin"] for c,d in placements["contre-torpilleur"])
assert any(abs(a-c)==1 and abs(b-d)==1 for a,b in placements["patrouilleur"] for c,d in placements["croiseur"])
assert all(label in svg for label in ("AUTO","PRÊT","RETOUR","VS CAPITAINE","Nouveau placement","Confirmer et jouer","Revenir à l'accueil"))
cairosvg.svg2png(bytestring=svg.encode("utf-8"),write_to=str(NATIVE),output_width=320,output_height=240)
with Image.open(NATIVE) as raw:
    assert raw.size==(320,240)
    pal=raw.convert("RGB").quantize(colors=256,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
    pal.save(NATIVE,optimize=True)
    pal.resize((1280,960),Image.Resampling.NEAREST).save(ZOOM,optimize=True)
with Image.open(NATIVE) as native,Image.open(ZOOM) as zoom:
    assert zoom.size==(1280,960)
    expected_up=native.convert("RGBA").resize((1280,960),Image.Resampling.NEAREST)
    assert zoom.convert("RGBA").tobytes()==expected_up.tobytes(), "NEAREST RGBA mismatch"
    corrupted=zoom.convert("RGBA").copy()
    px=corrupted.getpixel((1,0))
    corrupted.putpixel((1,0),(px[0]^1,*px[1:]))
    assert corrupted.tobytes()!=expected_up.tobytes(), "negative RGB control ineffective"
print("PASS G03 5 ships / 17 cells / side+diagonal contacts / G02 intact / 320x240 + exact NEAREST x4")
