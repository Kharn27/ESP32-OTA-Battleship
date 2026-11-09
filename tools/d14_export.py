#!/usr/bin/env python3
"""D14 G03: extract canonical opaque BACKGROUND and G02 ship RGBA masks.

The approved native PNG is the sole base for ALL pixels outside the five
original ship footprints. Inside those footprints, render the SVG with its
five <use> placements removed. Do not edit design/. Never fill-by-hand.
CairoSVG 2.7.1 + Pillow 11.3.0 are pinned for repeatable source rendering.

Output generated C in build directory only; never changes approved artwork.
"""
import argparse
from io import BytesIO
import hashlib
import json
from pathlib import Path
import re
from xml.etree import ElementTree as ET

import cairosvg
from PIL import Image
from t03_export import encode_rle, rgb565_be

W, H, CELL, GX, GY = 320, 240, 18, 18, 42
TYPE_NAMES = ("patrouilleur", "sous-marin", "contre-torpilleur",
              "croiseur", "porte-avions")
LENGTHS = (2, 3, 3, 4, 5)
USE = re.compile(r'<use\s+href="#nav-([\w-]+)"\s+data-col="(\d+)"\s+'
                 r'data-row="(\d+)"\s+data-length="(\d+)"\s+'
                 r'data-orient="([HV])"\s+transform="([^"]+)"/>')
ROOT = 'http://www.w3.org/2000/svg'


def bytes_for_rgb565(image):
    return rgb565_be(image.convert("RGB").tobytes())


def parse_svg(svg):
    uses = list(USE.finditer(svg))
    if len(uses) != 5:
        raise ValueError("Expected exactly 5 canonical G03 ship uses")
    bounds = []
    identities = set()
    for m in uses:
        name, col, row, length, orientation, transform = m.groups()
        col, row, length = int(col), int(row), int(length)
        if name not in TYPE_NAMES or name in identities:
            raise ValueError("Wrong G02 ship identity")
        identities.add(name)
        if length != LENGTHS[TYPE_NAMES.index(name)] or min(col, row) < 0:
            raise ValueError("Unexpected ship length or coordinate")
        if (col + (length if orientation == 'H' else 1) > 10 or
                row + (length if orientation == 'V' else 1) > 10):
            raise ValueError("Ship outside grid")
        expected_transform = (
            f"translate({GX + CELL * col + 1} "
            f"{GY + CELL * (row if orientation == 'H' else row + length)}) "
            f"{'rotate(-90) ' if orientation == 'V' else ''}scale(1)"
        )
        if transform != expected_transform:
            raise ValueError(f"Ship anchor not contract: {name}: {transform}")
        bounds.append((GX + CELL * col, GY + CELL * row,
                       GX + CELL * (col + (length if orientation == 'H' else 1)),
                       GY + CELL * (row + (length if orientation == 'V' else 1))))
    if identities != set(TYPE_NAMES):
        raise ValueError("Missing G02 identity")
    empty_svg = USE.sub('', svg)
    if '<use' in empty_svg or '</defs>' not in empty_svg:
        raise ValueError("Unexpected SVG use structure")
    return bounds, empty_svg


def render_svg(svg, size):
    return Image.open(BytesIO(cairosvg.svg2png(
        bytestring=svg.encode('utf8'), output_width=size[0],
        output_height=size[1]))).convert('RGBA')


def fmt_u8(name, values):
    return ("static const uint8_t " + name + "[" + str(len(values)) + "] = {\n" +
            "\n".join("  " + ",".join(str(b) for b in values[i:i + 24]) + ","
                      for i in range(0, len(values), 24)) + "\n};\n")


def encode(base, svg):
    root = ET.fromstring(svg)
    defs = root.find(f"{{{ROOT}}}defs")
    if defs is None:
        raise ValueError("Missing canonical SVG G02 definitions")
    definitions = ET.tostring(defs, encoding='unicode')
    bg = bytearray()
    data = []
    sizes = []
    for i, name in enumerate(TYPE_NAMES):
        orientations = []
        for v in (0, 1):
            length = LENGTHS[i]
            w, h = ((length * CELL, CELL) if v == 0
                    else (CELL, length * CELL))
            transform = (f"translate(1 {0 if v == 0 else h}) " +
                         ("rotate(-90)" if v == 1 else "scale(1)"))
            piece = (
                f'<svg xmlns="{ROOT}" width="{w}" height="{h}" '
                f'viewBox="0 0 {w} {h}">{definitions}'
                f'<use href="#nav-{name}" transform="{transform}"/></svg>')
            rgba = render_svg(piece, (w, h))
            raw = bytearray()
            active = 0
            for r, g, b, a in rgba.getdata():
                val = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                raw.extend((val >> 8, val & 255, a))
                active += int(a != 0)
            if not active or active > w * h:
                raise ValueError("Empty or broken sprite")
            data.append(fmt_u8(f"d14_ship_{i}_{v}", raw))
            orientations.append((w, h, active))
        sizes.append(orientations)
    return data, sizes


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--svg', required=True)
    parser.add_argument('--png', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    svg = Path(args.svg).read_text(encoding='utf8')
    source = Path(args.png).read_bytes()
    original = Image.open(BytesIO(source)).convert('RGB')
    if original.size != (W, H):
        raise ValueError("Not native G03")
    boxes, empty = parse_svg(svg)
    clear = render_svg(empty, (W, H)).convert('RGB')
    clean = original.copy()
    for box in boxes:
        clean.paste(clear.crop(box), box)
    # This guard ensures that ALL approved pixels outside the five original
    # ship rectangles are *exactly* retained from the canonical PNG.
    mask = bytearray(W * H)
    for x0, y0, x1, y1 in boxes:
        for y in range(y0, y1):
            mask[y * W + x0:y * W + x1] = bytes([1]) * (x1 - x0)
    oldpix = original.tobytes()
    newpix = clean.tobytes()
    if any(oldpix[3*i:3*i+3] != newpix[3*i:3*i+3]
           for i in range(W*H) if not mask[i]):
        raise ValueError("Approved non-ship pixel changed")

    raw = bytes_for_rgb565(clean)
    rle, offsets = encode_rle(raw)
    sprites, sizes = encode(clean, svg)
    lines = ["/* D14 generated; canonical G03 PNG + G02 symbols in canonical SVG. */",
             '#include "d14_g03.h"', '#include <stddef.h>',
             '#include <stdint.h>']
    lines.append(fmt_u8('d14_g03_rle', rle))
    lines.append('const uint32_t d14_g03_offsets[241] = {')
    for i in range(0, len(offsets), 12):
        lines.append('  ' + ','.join(str(v) + 'u' for v in offsets[i:i+12]) + ',')
    lines.append('};')
    lines.append('const T03Image d14_g03_image = {d14_g03_rle, sizeof(d14_g03_rle), d14_g03_offsets, 241u, 320u, 240u, T03_ROW_RLE565BE};')
    lines.extend(sprites)
    lines.append('const D14Sprite d14_g03_sprites[5][2] = {')
    for i in range(5):
        entries = []
        for v in range(2):
            w, h, active = sizes[i][v]
            entries.append(f'{{d14_ship_{i}_{v},{w}u,{h}u}}')
        lines.append('  {' + ','.join(entries) + '},')
    lines.append('};')
    lines.append('')
    generated = '\n'.join(lines).encode('ascii')
    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    if not out.is_file() or out.read_bytes() != generated:
        out.write_bytes(generated)
    print(json.dumps({
        "format": "D14-G03-ROW-RLE565BE-v1+RGBA-mask-v1",
        "source_png_sha256": hashlib.sha256(source).hexdigest(),
        "source_svg_sha256": hashlib.sha256(svg.encode()).hexdigest(),
        "background_rgb565_sha256": hashlib.sha256(raw).hexdigest(),
        "background_rle_bytes": len(rle) + 4 * len(offsets),
        "sprite_rgba565_alpha_bytes": sum(
            sizes[i][v][0] * sizes[i][v][1] * 3
            for i in range(5) for v in (0, 1)),
        "source_ship_boxes": boxes,
        "unchanged_pixels_outside_source_boxes": W*H - sum(mask),
        "generated_sha256": hashlib.sha256(generated).hexdigest()
    }, sort_keys=True))


if __name__ == '__main__':
    main()
