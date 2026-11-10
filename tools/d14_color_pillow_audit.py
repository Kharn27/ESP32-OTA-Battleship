#!/usr/bin/env python3
"""D14 Issue #69: independent Pillow oracle for PO-approved G01 PR90.

Audit the ENTIRE 320x240 RGB888 PNG using Pillow (not t03_export.read_png),
independently quantize to RGB565-BE (not t03_export.rgb565_be), compare
with the custom decoder, generated test-only C data AND actual ESP32
application ELF/BIN bytes, then reconstruct PNG from the bytes in the BIN.

No source artwork edits, LCD settings, smoothing, gamma, colorspace LUT or
device flash. Reports state exactly which evidence was available.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import sys
import zlib

from PIL import Image, ImageChops, ImageDraw

W, H = 320, 240
RGB888_BYTES = W * H * 3
RAW565_BYTES = W * H * 2
SOURCE_PNG_SHA256 = "ca73f029e80c95c09b3c7defe1437bba3156280b2f2169465ee3f30014343873"
SOURCE_GIT_BLOB = "7bd321c6ccbfc230802e20010bfa06e71a9099a9"
SOURCE_RAW565_SHA256 = "8a376f7ca4cf83ed31b146905b25c80455b71fdef194eee8c40ac00e2d5248dc"


def sha(data):
    return hashlib.sha256(data).hexdigest()


def png_filter_on_row(png, row):
    """Read PNG filter byte from the compressed stream to explain any mismatch.
    The expected PR90 PNG is RGB8 with 3 IDAT chunks (no interlace).
    This does not decode pixels and is independent of t03_export.read_png.
    """
    chunks = []
    offset = 8
    while offset + 12 <= len(png):
        size = int.from_bytes(png[offset:offset+4], "big")
        typ = png[offset+4:offset+8]
        if typ == b"IDAT":
            chunks.append(png[offset+8:offset+8+size])
        offset += size + 12
        if typ == b"IEND":
            break
    if not chunks:
        return None
    try:
        scanlines = zlib.decompress(b"".join(chunks))
        return scanlines[row * (W * 3 + 1)]
    except (IndexError, zlib.error):
        return None


def compare_rgb888(pillow_bytes, custom_bytes, png_bytes):
    assert len(pillow_bytes) == len(custom_bytes) == RGB888_BYTES
    if pillow_bytes != custom_bytes:
        pos = next(i for i in range(RGB888_BYTES)
                   if pillow_bytes[i] != custom_bytes[i])
        pixel = pos // 3
        y, x = divmod(pixel, W)
        idx = pixel * 3
        raise AssertionError(
            "INDEPENDENT_PNG_DISAGREEMENT pixel=(%d,%d) channel=%s "
            "pillow_rgb=%s custom_rgb=%s png_filter_row=%s first_byte=%d"
            % (x, y, "RGB"[pos % 3],
               list(pillow_bytes[idx:idx+3]), list(custom_bytes[idx:idx+3]),
               png_filter_on_row(png_bytes, y), pos))
    return W * H


def independent_rgb565be(rgb888):
    """Separate implementation: bitmasks instead of rgb565_be() shifts.

    Generate big-endian wire bytes from Pillow RGB data only. No source
    import/use of existing t03_export.rgb565_be.
    """
    assert len(rgb888) == RGB888_BYTES
    out = bytearray(RAW565_BYTES)
    for p in range(W*H):
        i = 3*p
        encoded = ((rgb888[i] & 0xf8) << 8) | \
                  ((rgb888[i+1] & 0xfc) << 3) | \
                  (rgb888[i+2] >> 3)
        out[2*p:2*p+2] = encoded.to_bytes(2, "big")
    return bytes(out)


def require_equal_rgb565(expected, got, label):
    if len(got) != RAW565_BYTES:
        raise AssertionError("%s wrong length %d (want %d)" %
                             (label, len(got), RAW565_BYTES))
    if expected != got:
        pos = next(i for i in range(RAW565_BYTES) if expected[i] != got[i])
        pixel = pos // 2
        y, x = divmod(pixel, W)
        start = pixel*2
        raise AssertionError(
            "%s RGB565 disagreement pixel=(%d,%d) expected=%s got=%s "
            "offset=%d" %
            (label, x, y, expected[start:start+2].hex(),
             got[start:start+2].hex(), pos))


def extract_generated_flash_array(path):
    source = Path(path).read_text(encoding="utf8")
    match = re.search(
        r"static\s+const\s+uint8_t\s+g01_pr90_raw565"
        r"\s*\[\s*(\d+)\s*\]\s*=\s*\{([^}]*)\}\s*;",
        source, re.S)
    if not match or int(match.group(1)) != RAW565_BYTES:
        raise AssertionError("G01 Flash array not found or wrong size in " + str(path))
    body = match.group(2)
    values = re.findall(r"0x([0-9a-fA-F]{2})(?:u|U)?\b", body)
    if len(values) != RAW565_BYTES:
        raise AssertionError("G01 Flash array parsed %d/%d bytes" %
                             (len(values), RAW565_BYTES))
    if "T03_RAW565BE" not in source or "320u, 240u" not in source:
        raise AssertionError("T03Image raw format/dimensions changed")
    return bytes(int(x,16) for x in values)


def find_compiled_bytes(path, expected, label):
    data = Path(path).read_bytes()
    first = data.find(expected)
    if first < 0:
        raise AssertionError(
            "%s: expected 153600 bytes NOT present as a continuous "
            "Flash asset in %s" % (label, path))
    second = data.find(expected, first + 1)
    if second >= 0:
        raise AssertionError("%s: G01 asset appears multiple times" % label)
    embedded = data[first:first+RAW565_BYTES]
    require_equal_rgb565(expected, embedded, label)
    return embedded, dict(file=Path(path).name, file_sha256=sha(data),
                          total_file_bytes=len(data),
                          offset_decimal=first, offset_hex="0x%x" % first,
                          occurrences=1, bytes_compared=RAW565_BYTES)


def expand_rgb565be(raw):
    """PC preview of the EXTRACTED firmware bytes; bit replication only.
    R5->R8: (r5<<3)|(r5>>2); G6->G8: (g6<<2)|(g6>>4).
    No gamma/saturation correction and NO use of source RGB888.
    """
    require_equal_rgb565(raw, raw, "self")
    output = bytearray(RGB888_BYTES)
    for p in range(W*H):
        value = (raw[2*p] << 8) | raw[2*p+1]
        r5 = (value >> 11) & 31
        g6 = (value >> 5) & 63
        b5 = value & 31
        output[3*p] = (r5 << 3) | (r5 >> 2)
        output[3*p+1] = (g6 << 2) | (g6 >> 4)
        output[3*p+2] = (b5 << 3) | (b5 >> 2)
    return bytes(output)


def color_examples(source_rgb, raw, preview_rgb):
    # Coordinates selected in the four named regions of the approved PNG.
    # They are samples, NOT calibrated colorimetric readings.
    samples = {
        "ciel_orange": (275, 75),
        "mer_bleue": (145, 188),
        "bouton_bronze": (286, 220),
        "logo": (159, 45),
        "soleil": (195, 112),
    }
    result = {}
    for label, (x,y) in samples.items():
        pixel = y*W+x
        orig = list(source_rgb[3*pixel:3*pixel+3])
        decoded = list(preview_rgb[3*pixel:3*pixel+3])
        result[label] = {
            "xy": [x,y], "pillow_rgb888": orig,
            "flash_rgb565_be": raw[2*pixel:2*pixel+2].hex().upper(),
            "preview_rgb888": decoded,
            "abs_difference": [abs(a-b) for a,b in zip(orig,decoded)]
        }
    return result


def write_visuals(outdir, src_image, preview_rgb, raw, metrics):
    outdir.mkdir(parents=True, exist_ok=True)
    rebuilt = Image.frombytes("RGB", (W,H), preview_rgb)
    reconstructed_path = outdir / "g01_reconstructed_from_esp32_flash.png"
    rebuilt.save(reconstructed_path, format="PNG")
    canvas = Image.new("RGB", (W*2, H+28), "white")
    canvas.paste(src_image, (0,28))
    canvas.paste(rebuilt, (W,28))
    painter = ImageDraw.Draw(canvas)
    painter.text((8,8),"SOURCE: G01 PR90 PNG / Pillow", fill="black")
    painter.text((W+8,8),"ESP32 BIN RAW565 -> RGB888", fill="black")
    canvas.save(outdir / "g01_original_vs_esp32_flash.png", format="PNG")
    abs_image = ImageChops.difference(src_image, rebuilt)
    abs_image.save(outdir / "g01_quantization_delta_actual.png", format="PNG")
    # Exaggerated ONLY in the diagnostic delta visualization, not preview.
    Image.eval(abs_image, lambda n: min(255,8*n)).save(
        outdir / "g01_quantization_delta_x8_DEBUG_ONLY.png", format="PNG")
    (outdir / "g01_extracted_from_esp32_flash.rgb565be").write_bytes(raw)
    metrics["outputs"] = {
        "reconstructed_png": reconstructed_path.name,
        "reconstructed_sha256": sha(reconstructed_path.read_bytes()),
        "side_by_side_png": "g01_original_vs_esp32_flash.png",
        "exact_delta_png": "g01_quantization_delta_actual.png",
        "amplified_delta_png": "g01_quantization_delta_x8_DEBUG_ONLY.png",
        "flash_extracted_raw565": "g01_extracted_from_esp32_flash.rgb565be",
    }


def main():
    a = argparse.ArgumentParser()
    a.add_argument("--input", required=True, help="PR90 canonical G01 PNG")
    a.add_argument("--generated-c", required=True,
                   help="actual compiled test-only build C array")
    a.add_argument("--generated-raw", help="reference from old generator (optional)")
    a.add_argument("--elf", help="actual diagnostic ESP-IDF ELF (optional)")
    a.add_argument("--bin", help="actual diagnostic ESP-IDF APP BIN (optional)")
    a.add_argument("--out-dir", required=True)
    args = a.parse_args()
    if (args.bin is None) != (args.elf is None):
        a.error("--bin and --elf must be supplied together")

    png = Path(args.input).read_bytes()
    if sha(png) != SOURCE_PNG_SHA256 or len(png) != 144890:
        raise AssertionError("not original PO-approved G01 PR90 PNG")
    src = Image.open(args.input)
    if src.size != (W,H) or src.format != "PNG" or src.mode != "RGB":
        raise AssertionError("Pillow PNG not native RGB8 320x240")
    src_rgb = src.convert("RGB").tobytes()
    if len(src_rgb) != RGB888_BYTES:
        raise AssertionError("Pillow RGB888 geometry mismatch")
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from t03_export import read_png
    custom_rgb, _ = read_png(args.input)
    compare_rgb888(src_rgb,custom_rgb,png)

    pillow_expected = independent_rgb565be(src_rgb)
    if sha(pillow_expected) != SOURCE_RAW565_SHA256:
        raise AssertionError("independent Pillow RGB565 hash differs")
    actual_c = extract_generated_flash_array(args.generated_c)
    require_equal_rgb565(pillow_expected,actual_c,"generated C Flash asset")
    if args.generated_raw:
        require_equal_rgb565(pillow_expected,Path(args.generated_raw).read_bytes(),
                             "existing generator golden")

    firmware_evidence = None
    if args.bin:
        extracted_elf, elf_evidence = find_compiled_bytes(
            args.elf, pillow_expected, "ELF")
        extracted_bin, bin_evidence = find_compiled_bytes(
            args.bin, pillow_expected, "APP_BIN")
        require_equal_rgb565(extracted_elf,extracted_bin,"ELF vs BIN")
        actual_pixels = extracted_bin
        firmware_evidence = {"elf":elf_evidence, "bin":bin_evidence}
        image_origin = "APP_BIN_EXTRACTED"
    else:
        actual_pixels = actual_c
        image_origin = "GENERATED_C_FLASH_ARRAY_HOST_ONLY"

    preview_rgb = expand_rgb565be(actual_pixels)
    differences = [0,0,0]
    sum_diff = [0,0,0]
    count_changed = 0
    for p in range(W*H):
        differs = False
        for channel in range(3):
            at = p*3+channel
            delta = abs(src_rgb[at]-preview_rgb[at])
            differences[channel] = max(differences[channel],delta)
            sum_diff[channel] += delta
            differs |= delta != 0
        count_changed += differs
    if any(d>bound for d,bound in zip(differences,(7,3,7))):
        raise AssertionError("preview delta larger than RGB565 quantization")

    report = {
        "audit": "D14 Issue69 independent Pillow PR90 G01 RGB565 audit",
        "image_origin": image_origin,
        "source": {
            "repo": "Kharn27/ESP32-OTA-Battleship",
            "source": str(args.input), "git_blob_sha":SOURCE_GIT_BLOB,
            "png_sha256":sha(png), "bytes":len(png),
            "dimensions":[W,H], "mode":src.mode,
            "png_info_keys":sorted(src.info.keys())
        },
        "independent_oracle": {
            "reader":"Pillow.Image.open(...).convert('RGB').tobytes()",
            "compared_to_custom_read_png_rgb888_bytes":RGB888_BYTES,
            "compared_rgb888_pixels":W*H, "rgb888_mismatched_pixels":0,
            "pillow_rgb888_sha256":sha(src_rgb),
            "custom_rgb888_sha256":sha(custom_rgb),
            "quantization":"independent bitmask RGB888_to_RGB565BE",
            "rgb565_bytes":RAW565_BYTES,
            "pillow_rgb565_sha256":sha(pillow_expected),
            "generated_c_rgb565_sha256":sha(actual_c),
            "generated_c_mismatched_pixels":0,
            "old_generator_golden_compared":args.generated_raw is not None,
            "compiled_firmware_evidence":firmware_evidence,
        },
        "pc_reconstruction": {
            "algorithm":"565 bit replication only, no gamma/contrast/saturation",
            "max_abs_channel_delta_rgb888":dict(zip("RGB",differences)),
            "mean_abs_channel_delta_rgb888":dict(zip(
                "RGB",[round(n/(W*H),6) for n in sum_diff])),
            "pixels_changed_only_by_quantization":count_changed,
            "pixels_exact_rgb888":W*H-count_changed,
            "pixels_with_delta_exceeding_565_bounds":0,
            "examples":color_examples(src_rgb,actual_pixels,preview_rgb),
        },
        "causal_chain": {
            "SOURCE_PNG":"MATCH_PO_PR90_SHA256",
            "PNG_DECODE":"PILLOW_EQ_CUSTOM_76800_RGB888_PIXELS",
            "GENERATE_RGB565":"PILLOW_EQ_GENERATED_C_76800_PIXELS",
            "COMPILED_FLASH":("ELF_AND_APP_BIN_EXACT" if args.bin
                              else "NOT_CHECKED_HOST_ONLY"),
            "T03_COPY":"HOST_TESTED_BY_C99_TESTS_NOT_HARDWARE",
            "ESP_LCD":"BUILT_NOT_RUNTIME_MEASURED",
            "PANEL_CURVE":"ON_DEVICE_UNVERIFIED_CAUSE_OPEN"
        },
        "limitations":[
            "Proof of bytes present in APP BIN does not capture SPI DMA on device.",
            "No optical instrument or CYD run in this audit.",
            "Correct primary RGB565 swatches do not validate intermediate colors.",
            "PC preview compares 565 bit replication only, no LCD panel curve."
        ]
    }
    outdir = Path(args.out_dir)
    write_visuals(outdir,src,preview_rgb,actual_pixels,report)
    (outdir/"pillow_expected.rgb565be").write_bytes(pillow_expected)
    (outdir/"report.json").write_text(
        json.dumps(report,indent=2,sort_keys=True)+"\n",encoding="utf8")
    (outdir/"README.txt").write_text(
        "D14 G01 PR90 independent Pillow audit.\n"
        "Original source: design/ecrans/accueil/accueil_cyd_warship_320x240.png\n"
        "g01_original_vs_esp32_flash.png: PC source and reconstructed RAW565.\n"
        "g01_reconstructed_from_esp32_flash.png: only bytes extracted from APP BIN\n"
        "when image_origin=APP_BIN_EXTRACTED, otherwise generated C bytes.\n"
        "g01_quantization_delta_actual.png: absolute RGB888 delta, not graded.\n"
        "g01_quantization_delta_x8_DEBUG_ONLY.png: delta x8 for visibility.\n"
        "g01_extracted_from_esp32_flash.rgb565be: extracted raw 565 bytes.\n"
        "report.json: SHA/offset/pixel-statistics & causal proof status.\n",
        encoding="utf8")
    print("D14_PILLOW_AUDIT PASS " +
          json.dumps({"origin":image_origin,
                      "rgb888_pixels_equal":W*H,
                      "rgb565_pixels_equal":W*H,
                      "source_sha256":sha(png),
                      "raw565_sha256":sha(pillow_expected),
                      "max_abs_delta_rgb888":differences,
                      "firmware":firmware_evidence,
                      "report":str(outdir/"report.json")},
                     sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main())
