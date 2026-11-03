#!/usr/bin/env python3
"""G02: check the SVG <use> placements, not an unrelated hardcoded list."""
from pathlib import Path
from collections import Counter
from xml.etree import ElementTree as ET
import re

DIR=Path(__file__).resolve().parent
NS="{http://www.w3.org/2000/svg}"
LENGTHS={"patrouilleur":2,"sous-marin":3,"contre-torpilleur":3,
         "croiseur":4,"porte-avions":5}
ORIGIN=(19,39)
STEP=18
TX_H=re.compile(r"translate\((\d+) (\d+)\) scale\(1\)")
TX_V=re.compile(r"translate\((\d+) (\d+)\) rotate\(-90\) scale\(1\)")

def extract(filename):
    root=ET.parse(DIR/filename).getroot()
    assert root.attrib["viewBox"]=="0 0 320 240"
    cells={}
    positions={}
    for use in root.iter(NS+"use"):
        href=use.get("href") or use.get("{http://www.w3.org/1999/xlink}href")
        if not href or not href.startswith("#nav-"):
            continue
        ship=href[5:]
        assert ship in LENGTHS and ship not in positions,ship
        col=int(use.attrib["data-col"])
        row=int(use.attrib["data-row"])
        n=int(use.attrib["data-length"])
        orient=use.attrib["data-orient"]
        assert n==LENGTHS[ship] and orient in ("H","V"),ship
        match=(TX_H if orient=="H" else TX_V).fullmatch(use.attrib["transform"])
        assert match, (ship, use.attrib["transform"])
        tx,ty=map(int,match.groups())
        gx=ORIGIN[0]+col*STEP
        gy=ORIGIN[1]+row*STEP
        wanted=(gx,gy if orient=="H" else gy+n*STEP)
        assert (tx,ty)==wanted,(ship,(tx,ty),wanted)
        # For a -90deg hull, transformed local y=3..14 remains inside its column.
        if orient=="V":
            assert gx+3<=tx+3<tx+14<gx+STEP
        positions[ship]=(col,row,n,orient,tx,ty)
        for i in range(n):
            tile=(col+(i if orient=="H" else 0),
                  row+(i if orient=="V" else 0))
            assert 0<=tile[0]<10 and 0<=tile[1]<10,(ship,tile)
            assert tile not in cells,(ship,cells.get(tile),tile)
            cells[tile]=ship
    assert Counter(p[2] for p in positions.values())==Counter([2,3,3,4,5]),positions
    assert len(positions)==5 and len(cells)==17,(positions,len(cells))
    assert {p[3] for p in positions.values()}=={"H","V"}
    assert positions["contre-torpilleur"][:4]==(5,4,3,"V")
    assert positions["croiseur"][:4]==(8,3,4,"V")
    # Orthogonal and diagonal contacts preserved and visible as separate <use>.
    assert cells[(4,5)]=="sous-marin" and cells[(5,5)]=="contre-torpilleur"
    assert cells[(7,7)]=="patrouilleur" and cells[(8,6)]=="croiseur"
    return positions,cells

def main():
    p1,c1=extract("g02_grille_320x240.svg")
    p4,c4=extract("g02_grille_x4.svg")
    assert p1==p4 and c1==c4
    for name,p in sorted(p1.items()):
        print(f"PASS {name}: {chr(65+p[0])}{p[1]+1}, {p[2]} cases, {p[3]}, use=({p[4]}, {p[5]})")
    print("PASS native et x4 SVG: 17 cases, F5-F7, I4-I7, 2 contacts, aucun recouvrement")

if __name__=="__main__":
    main()
