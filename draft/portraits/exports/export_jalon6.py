#!/usr/bin/env python3
"""PORTRAITS-04 / Issue #133, Jalon 6 : huit positions C–E. Reproductible, non destructif en --check.

Python 3.11+ ; Pillow==12.3.0. 48/64 sont calculés séparément depuis
le master RGBA carré, avec rééchantillonnage BOX déterministe en alpha
prémultiplié (arithmétique entière) ; les ×4 utilisent NEAREST exact.
Aucune donnée métier ni aucun rôle de personnage n'est assigné.
"""
from __future__ import annotations

import argparse
import hashlib
import struct
import sys
import zlib
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[3]
SOURCES = ROOT / "draft/portraits"
EXPORTS = SOURCES / "exports"
PROOFS = EXPORTS / "preuves"
SOURCES_SHA256 = {
    "A": "30e92fa1eed23733d812a521c6910390860620f906a0f3529e8410abe0bd9acc",
    "B": "77a81a05009dd8ee7f5af6cda2d13ce7d9282ef0aebbf39db10cad9c1ff6b9ea",
    "C": "b24d39572a54d464e1692cb76967e9d98463fa9a9e8b1589693ef88795ef7ad8",
    "D": "68f1e1811a3370fa7249d70fd3bd3e9d21481511c1401ed994ecb6ef6cee111c",
    "E": "c9620a0a1fad7c15e82c4ab1f7ac1debf07c2ed19b75f498c40e0597bd95f975",
}
# ID, planche, (x0, y0, x1_exclu, y1_exclu), côté master, marge haut.
CASES = [
    ("C-L4-C1", "C", (0, 815, 362, 1086), 362, 45),
    ("C-L4-C2", "C", (362, 815, 724, 1086), 362, 45),
    ("C-L4-C3", "C", (724, 815, 1086, 1086), 362, 45),
    ("C-L4-C4", "C", (1086, 815, 1448, 1086), 362, 45),
    ("E-L3-C1", "E", (0, 543, 362, 815), 362, 45),
    ("E-L3-C2", "E", (362, 543, 724, 815), 362, 45),
    ("E-L3-C3", "E", (724, 543, 1086, 815), 362, 45),
    ("E-L3-C4", "E", (1086, 543, 1448, 815), 362, 45),
]
ORDER = [case[0] for case in CASES]
GLYPHS = {
 "0":["111","101","101","101","111"], "1":["010","110","010","010","111"],
 "2":["111","001","111","100","111"], "3":["111","001","111","001","111"],
 "4":["101","101","111","001","001"], "5":["111","100","111","001","111"],
 "6":["111","100","111","101","111"], "8":["111","101","111","101","111"],
 "9":["111","101","111","001","111"], "A":["010","101","111","101","101"],
 "B":["110","101","110","101","110"], "C":["011","100","100","100","011"],
 "D":["110","101","101","101","110"], "E":["111","100","110","100","111"],
 "L":["100","100","100","100","111"], "-":["000","000","111","000","000"],
 " ":["000","000","000","000","000"],
 "V":["101","101","101","101","010"], "N":["101","111","111","111","101"],
 "T":["111","010","010","010","010"], "P":["110","101","110","100","100"],
 "R":["110","101","110","101","101"], "S":["011","100","010","001","110"],
}

def sha256(content: bytes) -> str:
    return hashlib.sha256(content).hexdigest()

def png_rgba(size: tuple[int,int], pixels: bytes) -> bytes:
    """RGBA PNG: filters 0, zlib RFC1950 / DEFLATE stocké (niveau 0).
    Paramètres fixés pour comparaison *octet à octet*, sans métadonnées.
    """
    w, h = size
    assert len(pixels) == w*h*4, (size, len(pixels))
    raw = b"".join(b"\x00" + pixels[y*w*4:(y+1)*w*4] for y in range(h))
    z = bytearray(b"\x78\x01")
    for off in range(0, len(raw), 65535):
        block = raw[off:off+65535]
        z.extend((1 if off + len(block) == len(raw) else 0,))
        z.extend(struct.pack("<HH", len(block), (~len(block)) & 65535))
        z.extend(block)
    z.extend(struct.pack(">I", zlib.adler32(raw) & 0xffffffff))
    def chunk(tag: bytes, data: bytes) -> bytes:
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff))
    return (b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
            + chunk(b"IDAT", bytes(z)) + chunk(b"IEND", b""))

def master_from_original(sheet: Image.Image, rect: tuple[int,int,int,int],
                         side: int, top_pad: int) -> bytes:
    crop = sheet.crop(rect)
    if crop.width != side or top_pad + crop.height > side:
        raise ValueError(f"Recadrage non carré hors master : {rect} / {side} / {top_pad}")
    img = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    img.paste(crop, (0, top_pad))  # copie RGBA, sans masque de mélange
    data = bytearray(img.tobytes())
    for p in range(0, len(data), 4):
        if data[p+3] == 0:  # RGB caché derrière alpha nul = 0, anti-halo
            data[p:p+3] = b"\x00\x00\x00"
    return bytes(data)

LOCAL_MASKS = {
    "E-L3-C2": ((348, 0, 362, 362, "all"),),
}

def apply_local_mask(id_: str, original: bytes, side: int) -> bytes:
    """Local J6 corrections only in derived master, never PO original.
    red: r>=110 g<75 b<85 2*r>3*g 2*r>3*b inside ROI.
    all: zero out each pixel in the half-open rectangle.
    """
    output = bytearray(original)
    for x0, y0, x1, y1, policy in LOCAL_MASKS.get(id_, ()):
        if not (0 <= x0 < x1 <= side and 0 <= y0 < y1 <= side):
            raise ValueError(f"Mask out of bounds: {id_} {x0,y0,x1,y1}")
        for y in range(y0, y1):
            for x in range(x0, x1):
                i = (y*side+x)*4
                r, g, b, a = output[i:i+4]
                if a and (policy == "all" or
                          (r >= 110 and g < 75 and b < 85 and 2*r > 3*g and 2*r > 3*b)):
                    output[i:i+4] = bytes(4)
    return bytes(output)

def box_alpha_rgba(master: bytes, side: int, target: int) -> bytes:
    """Intégration exacte des surfaces de recouvrement ; alpha prémultiplié.
    Ne pas redimensionner 48 vers 64 : chacun part du même master HD.
    """
    dst = bytearray(target * target * 4)
    denom = side * side
    for y in range(target):
        sy0 = y*side//target
        sy1 = ((y+1)*side + target-1)//target
        for x in range(target):
            sx0 = x*side//target
            sx1 = ((x+1)*side + target-1)//target
            sa = sr = sg = sb = 0
            for yy in range(sy0, sy1):
                wy = max(0, min((yy+1)*target, (y+1)*side) - max(yy*target, y*side))
                for xx in range(sx0, sx1):
                    wx = max(0, min((xx+1)*target, (x+1)*side) - max(xx*target, x*side))
                    p = (yy*side + xx)*4
                    v = master[p+3] * wx * wy
                    if not v:
                        continue
                    sa += v
                    sr += master[p] * v
                    sg += master[p+1] * v
                    sb += master[p+2] * v
            a = (sa + denom//2)//denom
            if a and sa:
                q = (y*target + x)*4
                dst[q] = (sr + sa//2)//sa
                dst[q+1] = (sg + sa//2)//sa
                dst[q+2] = (sb + sa//2)//sa
                dst[q+3] = a
    return bytes(dst)

def nearest4(data: bytes, side: int) -> bytes:
    dst = bytearray(side*4*side*4*4)
    for y in range(side*4):
        for x in range(side*4):
            s = ((y//4)*side + x//4)*4
            d = (y*(side*4) + x)*4
            dst[d:d+4] = data[s:s+4]
    return bytes(dst)

def new_canvas(w: int, h: int, color: tuple[int,int,int]) -> bytearray:
    return bytearray(bytes((*color,255)) * (w*h))

def fill(canvas: bytearray, w: int, x: int, y: int, ww: int, hh: int, col: tuple[int,int,int]) -> None:
    for yy in range(y, y+hh):
        for xx in range(x, x+ww):
            p = (yy*w+xx)*4
            canvas[p:p+4] = bytes((*col, 255))

def label(canvas: bytearray, w: int, text: str, x: int, y: int, col: tuple[int,int,int]) -> None:
    for char in text:
        g = GLYPHS.get(char, GLYPHS[" "])
        for yy in range(5):
            for xx in range(3):
                if g[yy][xx] == "1":
                    p = ((y+yy)*w+x+xx)*4
                    canvas[p:p+4] = bytes((*col,255))
        x += 4

def stamp(canvas: bytearray, w: int, sprite: bytes, side: int, x: int, y: int) -> None:
    for yy in range(side):
        for xx in range(side):
            s = (yy*side+xx)*4
            d = ((y+yy)*w + (x+xx))*4
            a = sprite[s+3]
            if not a: continue
            for c in range(3):
                canvas[d+c] = (sprite[s+c]*a + canvas[d+c]*(255-a)+127)//255

def correction_comparison(before: dict[int, bytes], after: dict[int, bytes]) -> bytes:
    """Témoin avant/après 320x240 : rangée sombre, rangée claire, 48 et 64."""
    canvas = new_canvas(320, 240, (17, 31, 46))
    fill(canvas, 320, 0, 120, 320, 120, (225, 236, 243))
    for row in (0, 1):
        y = row * 120
        ink = (221, 234, 244) if row == 0 else (34, 53, 68)
        for col, variants in enumerate((before, after)):
            x = col * 160
            label(canvas, 320, "AVANT" if col == 0 else "APRES", x+8, y+5, ink)
            stamp(canvas, 320, variants[48], 48, x+10, y+24)
            stamp(canvas, 320, variants[64], 64, x+80, y+18)
            label(canvas, 320, "48", x+24, y+87, ink)
            label(canvas, 320, "64", x+100, y+87, ink)
    return png_rgba((320, 240), canvas)

def previews(data_by_id: dict[str,dict[int,bytes]], before_by_id: dict[str,dict[int,bytes]]) -> dict[Path,bytes]:
    generated: dict[Path,bytes] = {}
    for id_, sizes in data_by_id.items():
        for n in (48, 64):
            generated[PROOFS/f"jalon6_{id_}_{n}_x4.png"] = png_rgba(
                (n*4,n*4), nearest4(sizes[n],n))
    for page in range(2):
        ids = ORDER[page*4:page*4+4]
        for bg_name, bg, text_col in (
            ("sombre", (17,31,46), (221,234,244)),
            ("clair", (225,236,243), (34,53,68)),
        ):
            b = new_canvas(320,240,bg)
            for j, id_ in enumerate(ids):
                x = 8+j*80
                label(b,320,id_,x,5,text_col)
                stamp(b,320,data_by_id[id_][48],48,x+13,24)
                stamp(b,320,data_by_id[id_][64],64,x+5,123)
            label(b,320,"48",2,85,text_col)
            label(b,320,"64",2,211,text_col)
            generated[PROOFS/f"jalon6_p{page+1}_320x240_{bg_name}.png"] = png_rgba((320,240),b)
    board=new_canvas(1120,182,(34,48,65))
    for i,id_ in enumerate(ORDER):
        x=i*140
        label(board,1120,id_,x+2,4,(250,242,208))
        fill(board,1120,x+2,20,62,58,(13,28,47))
        fill(board,1120,x+73,20,62,58,(226,237,245))
        stamp(board,1120,data_by_id[id_][48],48,x+9,25)
        stamp(board,1120,data_by_id[id_][48],48,x+80,25)
        fill(board,1120,x+1,99,68,68,(13,28,47))
        fill(board,1120,x+71,99,68,68,(226,237,245))
        stamp(board,1120,data_by_id[id_][64],64,x+3,101)
        stamp(board,1120,data_by_id[id_][64],64,x+73,101)
    generated[PROOFS/"jalon6_contact_1120x182.png"] = png_rgba((1120,182),board)
    for id_, previous in before_by_id.items():
        generated[PROOFS/f"jalon6_correctif_{id_}_avant_apres_320x240.png"] = correction_comparison(previous, data_by_id[id_])
    return generated

def audit(p: Path, generated: bytes, check: bool) -> bool:
    if check:
        if not p.is_file():
            print(f"ABSENT {p.relative_to(ROOT)}")
            return False
        actual = p.read_bytes()
        if actual != generated:
            print(f"DIFF {p.relative_to(ROOT)} sha256_existing={sha256(actual)} sha256_expected={sha256(generated)}")
            return False
        print(f"OK   {p.relative_to(ROOT)} bytes={len(generated)} sha256={sha256(generated)}")
    else:
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(generated)
        print(f"WRITE {p.relative_to(ROOT)} bytes={len(generated)} sha256={sha256(generated)}")
    return True

def verify_manifest() -> bool:
    """J6: validate all declared SHA-256/size against *real committed files*."""
    import json, re
    path = EXPORTS / "manifest_jalon6.json"
    if not path.is_file():
        print("FAIL missing J6 manifest")
        return False
    meta = json.loads(path.read_text(encoding="utf-8"))
    ok = meta.get("stage") == "JALON_6" and meta.get("issue") == 133
    ids = {case[0] for case in CASES}
    if {i["id"] for i in meta["items"]} != ids or len(meta["items"]) != len(ids):
        print("FAIL manifest items/IDs mismatch")
        ok = False
    expected_proofs = {f"draft/portraits/exports/preuves/jalon6_{id_}_{n}_x4.png"
                       for id_ in ids for n in (48,64)}
    expected_proofs |= {f"draft/portraits/exports/preuves/jalon6_p{page}_320x240_{bg}.png"
                        for page in (1,2) for bg in ("sombre","clair")}
    expected_proofs.add("draft/portraits/exports/preuves/jalon6_contact_1120x182.png")
    declared = [p["path"] for p in meta["proofs"]]
    if set(declared) != expected_proofs or len(declared) != len(expected_proofs):
        print("FAIL missing/duplicated proof entry")
        ok = False
    entries=[]
    for item in meta["items"]:
        for kind,side in (("master",item["master_square"]),("p48",48),("p64",64)):
            entries.append((item["files"][kind],side))
    entries.extend((p,None) for p in meta["proofs"])
    seen=set()
    for record,side in entries:
        rel=record["path"]
        if rel in seen or not rel.startswith("draft/portraits/exports/") or ".." in Path(rel).parts:
            print("FAIL duplicate/unsafe artifact path",rel);ok=False;continue
        seen.add(rel)
        p = ROOT / rel
        digest=record.get("sha256","")
        if not re.fullmatch(r"[0-9a-f]{64}",digest):
            print("FAIL malformed 64-hex SHA-256",rel,digest);ok=False;continue
        if not p.is_file():
            print("FAIL missing artifact",rel);ok=False;continue
        content=p.read_bytes()
        if sha256(content)!=digest or len(content)!=record.get("size_bytes"):
            print("FAIL REAL artifact SHA/size mismatch",rel,sha256(content),len(content))
            ok=False
        try:
            with Image.open(p) as im:
                if im.mode != "RGBA" or (side is not None and im.size!=(side,side)):
                    print("FAIL RGBA/dimension",rel,im.mode,im.size);ok=False
        except Exception as exc:
            print("FAIL invalid PNG",rel,exc);ok=False
    print("J6_MANIFEST_FILES", "PASS" if ok else "FAIL", "verified",len(seen))
    return ok

def main() -> int:
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--check", action="store_true", help="Lecture seule, vérifie les PNG réellement committés (défaut)")
    mode.add_argument("--write", action="store_true", help="Régénération volontaire des candidats et des preuves")
    a = parser.parse_args()
    check = not a.write
    for p, expected in SOURCES_SHA256.items():
        content = (SOURCES/f"portrait_{p}.png").read_bytes()
        actual = sha256(content)
        if actual != expected:
            raise RuntimeError(f"Original PO modifié : {p} {actual} != {expected}")
        print(f"SOURCE {p} SHA-256 OK ({actual})")
    data_by_id: dict[str,dict[int,bytes]] = {}
    before_by_id: dict[str,dict[int,bytes]] = {}
    ok = True
    for id_, p, rect, side, top in CASES:
        with Image.open(SOURCES/f"portrait_{p}.png") as img:
            if img.mode != "RGBA":
                raise RuntimeError(f"{p}: source non RGBA : {img.mode}")
            uncorrected_master = master_from_original(img,rect,side,top)
            master = apply_local_mask(id_, uncorrected_master, side)
        if False:  # Aucun témoin J2 ; les preuves J6 sont les 1x et 4x par ID.
            before_by_id[id_] = {n: box_alpha_rgba(uncorrected_master, side, n) for n in (48, 64)}
        ok &= audit(EXPORTS/id_/"master_HD.png",png_rgba((side,side),master),check)
        data_by_id[id_] = {}
        for n in (48,64):
            data = box_alpha_rgba(master,side,n)
            data_by_id[id_][n] = data
            ok &= audit(EXPORTS/id_/f"portrait_{n}.png",png_rgba((n,n),data),check)
    for p, content in previews(data_by_id, before_by_id).items():
        ok &= audit(p,content,check)
    if check: ok &= verify_manifest()
    print("CHECK PASS" if ok else "CHECK FAILED", "(sans écriture)" if check else "(régénération)")
    return 0 if ok else 1

if __name__ == "__main__":
    sys.exit(main())
