#!/usr/bin/env python3
"""D15-A G01 TEST ONLY: four approved CYD PNGs -> independent Flash layers.

The legacy maquette is NEVER an input for firmware source generation.
An optional --reference is used solely to report PC image differences.
No graphics files, asset manifest or PO source are modified.
"""
import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image

W, H = 320, 240
LAYERS = (
    ("wallpaper.accueil.retenu.cyd", "assets/wallpaper/accueil_retenu_CYD_320x240.png",
     (320, 240), (0, 0), 2),
    ("logo.titre.cyd", "assets/logo/logo_titre_CYD_145x66_RGBA.png",
     (145, 66), (88, 8), 3),
    ("boutons.jouer.cyd", "assets/boutons/bouton_jouer_CYD_140x32_RGBA.png",
     (140, 32), (14, 198), 3),
    ("boutons.palmares.cyd", "assets/boutons/bouton_palmares_CYD_140x30_RGBA.png",
     (140, 30), (166, 199), 3),
)


def sha256(b):
    return hashlib.sha256(b).hexdigest()


def load_assets(root):
    manifest_path = root / "assets/manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf8"))
    if manifest.get("schema_version") != 1:
        raise ValueError("Invalid assets manifest schema")
    catalog = manifest.get("assets")
    if not isinstance(catalog, list) or len({x["id"] for x in catalog}) != len(catalog):
        raise ValueError("Duplicate or invalid asset IDs")
    index = {item["id"]: item for item in catalog}
    imgs, report = [], []
    for aid, path, dim, xy, stride in LAYERS:
        if aid not in index:
            raise ValueError("Missing approved asset: " + aid)
        record = index[aid]
        if (record.get("path") != path or record.get("dimensions") != list(dim)
                or not record.get("po_validation_ref")
                or record.get("device_status") not in
                ("ON_DEVICE_UNVERIFIED", "ON_DEVICE_VERIFIED")):
            raise ValueError("Asset contract mismatch for: " + aid)
        source = root / path
        if not source.is_file() or source.is_symlink():
            raise ValueError("Approved asset is absent or link: " + path)
        raw = source.read_bytes()
        if sha256(raw) != record.get("sha256"):
            raise ValueError("SHA256 mismatch: " + path)
        with Image.open(source) as opened:
            if opened.size != dim or opened.format != "PNG":
                raise ValueError("Invalid native CYD PNG dimensions: " + path)
            if stride == 3 and opened.mode != "RGBA":
                raise ValueError("Expected straight RGBA sprite: " + path)
            if stride == 2 and opened.mode not in ("RGB", "RGBA"):
                raise ValueError("Expected RGB wallpaper: " + path)
            img = opened.convert("RGBA")
        if stride == 2 and img.getextrema()[3] != (255, 255):
            raise ValueError("Wallpaper has transparency: " + path)
        if xy[0] + dim[0] > W or xy[1] + dim[1] > H:
            raise ValueError("Asset outside LCD: " + aid)
        imgs.append(img)
        report.append({"id": aid, "path": path, "source_sha256": sha256(raw),
                       "dimensions": list(dim), "x": xy[0], "y": xy[1],
                       "stride_flash": stride})
    return imgs, report


def to565(r, g, b):
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def flash_bytes(img, stride):
    raw = img.tobytes()
    out = bytearray()
    for i in range(0, len(raw), 4):
        r, g, b, a = raw[i:i + 4]
        v = to565(r, g, b)
        out.extend((v >> 8, v & 255))
        if stride == 3:
            out.append(a)
    return bytes(out)


def mix565(bg, ink, alpha):
    if alpha == 0:
        return bg
    if alpha == 255:
        return ink
    inv = 255 - alpha
    br, bg6, bb = (bg >> 11) & 31, (bg >> 5) & 63, bg & 31
    fr, fg6, fb = (ink >> 11) & 31, (ink >> 5) & 63, ink & 31
    return (((br * inv + fr * alpha + 127) // 255) << 11
            | ((bg6 * inv + fg6 * alpha + 127) // 255) << 5
            | ((bb * inv + fb * alpha + 127) // 255))


def compose_565(blobs):
    bg = blobs[0]
    values = [((bg[i] << 8) | bg[i + 1]) for i in range(0, len(bg), 2)]
    for i in range(1, len(LAYERS)):
        _, _, (w, h), (x, y), _ = LAYERS[i]
        pix = blobs[i]
        for row in range(h):
            for col in range(w):
                p = (row * w + col) * 3
                ink = (pix[p] << 8) | pix[p + 1]
                dst = (y + row) * W + x + col
                values[dst] = mix565(values[dst], ink, pix[p + 2])
    return values


def from565(vals):
    data = bytearray()
    for val in vals:
        r = (val >> 11) & 31
        g = (val >> 5) & 63
        b = val & 31
        data.extend(((r << 3) | (r >> 2), (g << 2) | (g >> 4),
                     (b << 3) | (b >> 2)))
    return Image.frombytes("RGB", (W, H), bytes(data))


def difference(left, right):
    a, b = left.convert("RGB").tobytes(), right.convert("RGB").tobytes()
    assert len(a) == len(b) == W * H * 3
    delta = [abs(x - y) for x, y in zip(a, b)]
    return {"different_pixels": sum(any(delta[i:i + 3]) for i in range(0, len(delta), 3)),
            "mean_abs_rgb_channel": round(sum(delta) / len(delta), 6),
            "max_abs_channel": max(delta)}


def array_source(name, data):
    lines = [f"static const uint8_t {name}[{len(data)}] = {{"]
    for i in range(0, len(data), 24):
        lines.append("    " + ",".join(str(v) for v in data[i:i + 24]) + ",")
    lines.append("};")
    return "\n".join(lines)


def build_c(blobs):
    s = ['/* D15-A TEST ONLY; generated strictly from 4 manifest-approved CYD assets. */',
         '#include "d15a_g01.h"', '#include <stdint.h>']
    for i, b in enumerate(blobs):
        s.append(array_source(f"d15a_layer_{i}", b))
    s.append("const D15AAsset d15a_assets[D15A_IMAGE_COUNT] = {")
    for i, (_, _, (w, h), (x, y), stride) in enumerate(LAYERS):
        s.append(f"    {{d15a_layer_{i},sizeof d15a_layer_{i},{w}u,{h}u,{x}u,{y}u,{stride}u}},")
    s += ["};", ""]
    return "\n".join(s).encode("ascii")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--repo", type=Path, default=Path("."))
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--report", type=Path)
    p.add_argument("--preview", type=Path)
    p.add_argument("--reference", type=Path,
                   help="Comparison-only maquette; NEVER used by build data generation")
    args = p.parse_args()
    root = args.repo.resolve()
    imgs, sources = load_assets(root)
    blobs = [flash_bytes(img, LAYERS[i][4]) for i, img in enumerate(imgs)]
    src = build_c(blobs)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_bytes() != src:
        args.output.write_bytes(src)
    report = {"format": "D15A_RGB565BE_RAW_wallpaper_plus_3_RGB565BE_alpha8_sprites",
              "output_sha256": sha256(src),
              "flash_pixel_bytes": sum(map(len, blobs)),
              "flash_layers": [{**row, "flash_bytes": len(data), "flash_sha256": sha256(data)}
                               for row, data in zip(sources, blobs)],
              "source": "assets/manifest.json (4 native CYD IDs; no legacy inputs)"}
    if args.report or args.preview or args.reference:
        ref_rgb = imgs[0].copy()
        for layer, (_, _, _, (x, y), _) in zip(imgs[1:], LAYERS[1:]):
            ref_rgb.alpha_composite(layer, dest=(x, y))
        rgb888 = ref_rgb.convert("RGB")
        actual565 = from565(compose_565(blobs))
        report["rgb888_source_vs_firmware_rgb565"] = difference(rgb888, actual565)
        report["recomposed_rgb888_sha256"] = sha256(rgb888.tobytes())
        report["firmware_rgb888_sha256"] = sha256(actual565.tobytes())
        if args.reference:
            if not str(args.reference) == "maquettes/accueil/accueil_cyd_warship_320x240.png":
                raise ValueError("Current accueil composition is oracle-only under maquettes/accueil/")
            oracle = root / args.reference
            with Image.open(oracle) as im:
                if im.size != (W, H):
                    raise ValueError("Oracle is not 320x240")
                target = im.convert("RGB")
            report["maquette_oracle_only"] = str(args.reference)
            report["maquette_vs_assembled_rgb888"] = difference(target, rgb888)
            report["maquette_vs_firmware_rgb565"] = difference(target, actual565)
        if args.preview:
            args.preview.parent.mkdir(parents=True, exist_ok=True)
            actual565.save(args.preview, format="PNG")
            report["preview_png"] = str(args.preview)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, sort_keys=True, indent=2) + "\n", encoding="utf8")
    print(json.dumps(report, sort_keys=True))


if __name__ == "__main__":
    main()
