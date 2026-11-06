#!/usr/bin/env python3
"""G07-R2 #53: legal illustrative DEFAITE scenario and non-destructive history marks.
No confidential CPU fleet is drawn before voluntary VOIR SA FLOTTE.
"""
from pathlib import Path
import json
from PIL import Image, ImageDraw

SCENARIO=Path(__file__).with_name("g07_r2_defaite_scenario.json")

def xy(cell):
    assert cell[0] in "ABCDEFGHIJ" and 1<=int(cell[1:])<=10
    return 21+18*(ord(cell[0])-65)+9,53+18*(int(cell[1:])-1)+9

def fleet_cells(fleet):
    assert sorted(map(len,fleet.values()),reverse=True)==[5,4,3,3,2]
    occupied=set()
    for ship in fleet.values():
        assert len(ship)==len(set(ship))
        cols=[ord(s[0])-65 for s in ship]
        rows=[int(s[1:])-1 for s in ship]
        assert (len(set(cols))==1 and sorted(rows)==list(range(min(rows),max(rows)+1))) or (len(set(rows))==1 and sorted(cols)==list(range(min(cols),max(cols)+1)))
        assert not occupied.intersection(ship)
        occupied.update(ship)
    assert len(occupied)==17
    return occupied

def load_and_validate():
    sc=json.loads(SCENARIO.read_text(encoding="utf-8"))
    enemy=fleet_cells(sc["enemy_fleet"])
    human=fleet_cells(sc["human_fleet"])
    turns=sc["turns"]
    assert len(turns)==23
    a=[t["attack"] for t in turns]
    d=[t["defense"] for t in turns]
    assert len(set(a))==len(set(d))==23
    for cell in a+d: xy(cell)
    hits=set(a)&enemy
    misses=set(a)-enemy
    defense_hits=set(d)&human
    defense_misses=set(d)-human
    assert hits==set(sc["visible_fire_smoke_hits"]) and len(hits)==5
    assert misses==set(sc["existing_splash_misses"])|set(sc["new_subtle_misses"]) and len(misses)==18
    assert len(defense_hits)==17 and len(defense_misses)==6
    assert not enemy.issubset(hits) and human.issubset(defense_hits)
    assert turns[-1]==sc["last_simultaneous_pair"]=={"attack":"F6","defense":"G5"}
    assert len(set(d[:-1])&human)==16
    assert sc["result"]=="DEFAITE"
    return sc

def marks_layers(sc):
    """Existing dramatic impacts preserved; foam/ember marks sit on an editable overlay."""
    miss=Image.new("RGBA",(320,240))
    hit=Image.new("RGBA",(320,240))
    dm=ImageDraw.Draw(miss)
    dh=ImageDraw.Draw(hit)
    for cell in sc["new_subtle_misses"]:
        x,y=xy(cell)
        dm.line((x-4,y+2,x-2,y+1),fill=(112,183,199,220),width=1)
        dm.line((x+2,y+1,x+4,y+2),fill=(112,183,199,220),width=1)
        dm.line((x-3,y+3,x+3,y+3),fill=(87,151,176,225),width=1)
        dm.line((x-2,y+1,x+2,y+1),fill=(178,219,226,235),width=1)
        dm.point((x,y),fill=(226,245,244,255))
    for cell in sc["visible_fire_smoke_hits"]:
        x,y=xy(cell)
        dh.point((x,y),fill=(255,201,111,255))
        dh.point((x-1,y+1),fill=(252,132,48,255))
        dh.point((x,y+2),fill=(179,68,34,255))
    return miss,hit

def verify_history_pixels(composite,sc):
    native=composite.convert("RGB")
    for cell in sc["new_subtle_misses"]:
        assert native.getpixel(xy(cell))==(226,245,244),("miss",cell)
    for cell in sc["visible_fire_smoke_hits"]:
        assert native.getpixel(xy(cell))==(255,201,111),("hit",cell)
    return True

if __name__=="__main__":
    load_and_validate()
    print("PASS: DEFAITE 23 simultaneous pairs, 17 human segments hit, 5 enemy hits, 18 player misses")
