#!/usr/bin/env python3
"""Read-only checks for Issue #162 PR1; never touches repository files."""
import hashlib
import io
import json
from pathlib import Path

import cairosvg
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parent
IDS = [f'grade_{gender}_{grade}' for gender in ('homme', 'femme')
       for grade in ('mousse', 'capitaine', 'amiral')]

def assert_equal(label, actual, expected):
    if actual != expected:
        raise AssertionError(f'{label}: non reproductible')


def main():
    info = json.loads((ROOT / 'manifest_concepts.json').read_text())
    for entry in info['files']:
        p = ROOT / Path(entry['path']).name
        data = p.read_bytes()
        assert_equal(str(p.name) + ' size', len(data), entry['size_bytes'])
        assert_equal(str(p.name) + ' sha256', hashlib.sha256(data).hexdigest(), entry['sha256'])
    for svg in sorted(ROOT.glob('*.svg')):
        png = ROOT / (svg.stem + '.png')
        generated = cairosvg.svg2png(bytestring=svg.read_bytes())
        assert_equal(png.name + ' CairoSVG bytes', generated, png.read_bytes())
    for gender in ('homme', 'femme'):
        imgs = [Image.open(ROOT / f'grade_{gender}_{grade}_etude.png').convert('RGBA')
                for grade in ('mousse', 'capitaine', 'amiral')]
        bbox = (30, 38, 66, 73)  # exact rectangle in concept source 96×96
        baseline = imgs[0].crop(bbox)
        for grade, im in zip(('mousse', 'capitaine', 'amiral'), imgs):
            assert_equal(grade + ' dimensions', im.size, (96, 96))
            assert_equal(grade + ' alpha transparent', im.getpixel((0, 0))[3], 0)
            assert ImageChops.difference(baseline, im.crop(bbox)).getbbox() is None, (
                f'{gender} facial identity drift ({grade})')
        print(f'PASS visage {gender}: 3/3 exact, zone 36×35 = 1260 RGBA pixels')
    for backdrop in ('sombre', 'clair'):
        original = Image.open(ROOT / f'apercu_{backdrop}_320x240.png').convert('RGBA')
        zoom = Image.open(ROOT / f'apercu_{backdrop}_320x240_x4_NEAREST.png').convert('RGBA')
        assert_equal('native dimensions', original.size, (320, 240))
        assert_equal('zoom dimensions', zoom.size, (1280, 960))
        assert ImageChops.difference(
            zoom, original.resize((1280, 960), Image.Resampling.NEAREST)).getbbox() is None
        print(f'PASS {backdrop}: 320×240 + x4 NEAREST exact')
    print('PASS integrity all manifest files / 10 SVG raster / face invariance / previews')

if __name__ == '__main__':
    main()
