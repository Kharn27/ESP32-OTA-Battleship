#!/usr/bin/env python3
"""T03: deterministic stdlib-only PNG RGB8 -> exact RGB565 BE + row-RLE C.
G03 only; deliberately rejects alpha, indexed, interlaced, malformed images.
No image post-processing and no changes to design/ source files.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import zlib

PNG_SIG = b"\x89PNG\r\n\x1a\n"
W, H = 320, 240


def read_png(path):
    data = Path(path).read_bytes()
    if not data.startswith(PNG_SIG):
        raise ValueError("bad PNG signature")
    at = 8
    ihdr = None
    compressed = []
    seen_end = False
    seen_idat = False
    while at < len(data):
        if len(data) - at < 12:
            raise ValueError("truncated chunk header")
        n = int.from_bytes(data[at:at + 4], "big")
        typ = data[at + 4:at + 8]
        end = at + 12 + n
        if end > len(data):
            raise ValueError("truncated chunk")
        body = data[at + 8:at + 8 + n]
        checksum = int.from_bytes(data[at + 8 + n:end], "big")
        if zlib.crc32(typ + body) != checksum:
            raise ValueError("PNG CRC mismatch")
        if ihdr is None and typ != b"IHDR":
            raise ValueError("missing initial IHDR")
        if typ == b"IHDR":
            if ihdr is not None or n != 13:
                raise ValueError("invalid IHDR")
            ihdr = struct.unpack(">IIBBBBB", body)
        elif typ == b"IDAT":
            seen_idat = True
            compressed.append(body)
        elif typ == b"IEND":
            if n or not seen_idat or end != len(data):
                raise ValueError("invalid IEND")
            seen_end = True
            break
        elif (typ[0] & 32) == 0:
            raise ValueError("unsupported critical PNG chunk")
        at = end
    if not seen_end or ihdr != (W, H, 8, 2, 0, 0, 0):
        raise ValueError("T03 accepts exactly 320x240 RGB8 noninterlaced PNG")
    rowbytes = W * 3
    expected = H * (rowbytes + 1)
    stream = zlib.decompressobj()
    scan = stream.decompress(b"".join(compressed), expected + 1)
    if len(scan) != expected or not stream.eof or stream.unused_data or stream.unconsumed_tail:
        raise ValueError("invalid or oversized PNG scanlines")
    scan += stream.flush()
    if len(scan) != expected:
        raise ValueError("unexpected PNG decompression tail")

    rgb = bytearray(W * H * 3)
    previous = bytearray(rowbytes)
    offset = 0
    for y in range(H):
        kind = scan[y * (rowbytes + 1)]
        line = bytearray(scan[y * (rowbytes + 1) + 1:(y + 1) * (rowbytes + 1)])
        if kind > 4:
            raise ValueError("unsupported PNG filter")
        for i in range(rowbytes):
            left = line[i - 3] if i >= 3 else 0
            above = previous[i]
            upperleft = previous[i - 3] if i >= 3 else 0
            if kind == 1:
                predictor = left
            elif kind == 2:
                predictor = above
            elif kind == 3:
                predictor = (left + above) // 2
            elif kind == 4:
                p = left + above - upperleft
                distances = (abs(p - left), abs(p - above), abs(p - upperleft))
                predictor = (left, above, upperleft)[distances.index(min(distances))]
            else:
                predictor = 0
            line[i] = (line[i] + predictor) & 255
        rgb[offset:offset + rowbytes] = line
        offset += rowbytes
        previous = line
    return bytes(rgb), data


def rgb565_be(rgb):
    out = bytearray(W * H * 2)
    for pixel in range(W * H):
        r, g, b = rgb[3 * pixel:3 * pixel + 3]
        value = (r >> 3) << 11 | (g >> 2) << 5 | (b >> 3)
        out[2 * pixel] = value >> 8
        out[2 * pixel + 1] = value & 255
    return bytes(out)


def encode_rle(raw):
    output = bytearray()
    offsets = [0]
    for y in range(H):
        line = raw[y * W * 2:(y + 1) * W * 2]
        col = 0
        while col < W:
            pair = line[2 * col:2 * col + 2]
            count = 1
            while col + count < W and count < 255 and line[2 * (col + count):2 * (col + count + 1)] == pair:
                count += 1
            output.extend((count, pair[0], pair[1]))
            col += count
        offsets.append(len(output))
    return bytes(output), offsets


def check_roundtrip(raw, rle, offsets):
    result = bytearray()
    for y in range(H):
        pos, end = offsets[y:y + 2]
        row = bytearray()
        while pos < end:
            n, high, low = rle[pos:pos + 3]
            if n == 0:
                raise ValueError("zero RLE count")
            row.extend(bytes((high, low)) * n)
            pos += 3
        if len(row) != W * 2 or pos != end:
            raise ValueError("RLE row mismatch")
        result.extend(row)
    if bytes(result) != raw:
        raise ValueError("RLE round trip differs from RGB565")


def c_array(name, data):
    lines = []
    for at in range(0, len(data), 16):
        lines.append("    " + ", ".join("0x%02x" % b for b in data[at:at + 16]) + ",")
    return "const uint8_t " + name + "[" + str(len(data)) + "] = {\n" + "\n".join(lines) + "\n};\n"


def export_c(raw, rle, offsets):
    src = ["/* Generated deterministically by tools/t03_export.py. G03 preview only. */",
           "#include <stddef.h>", "#include <stdint.h>",
           c_array("t03_g03_raw565", raw), c_array("t03_g03_rle", rle),
           "const uint32_t t03_g03_row_offsets[241] = {"]
    for at in range(0, len(offsets), 8):
        src.append("    " + ", ".join(str(v) + "u" for v in offsets[at:at + 8]) + ",")
    src.append("};")
    src.append("const size_t t03_g03_rle_size = sizeof(t03_g03_rle);")
    src.append("")
    return "\n".join(src).encode("ascii")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    rgb, png = read_png(args.input)
    raw = rgb565_be(rgb)
    rle, offsets = encode_rle(raw)
    check_roundtrip(raw, rle, offsets)
    generated = export_c(raw, rle, offsets)
    target = Path(args.output)
    target.parent.mkdir(parents=True, exist_ok=True)
    if not target.is_file() or target.read_bytes() != generated:
        target.write_bytes(generated)
    print(json.dumps({
        "format": "T03-RGB565BE-v1", "width": W, "height": H,
        "source_png_bytes": len(png),
        "source_sha256": hashlib.sha256(png).hexdigest(),
        "raw565_bytes": len(raw),
        "rle_stream_bytes": len(rle),
        "rle_row_index_bytes": 4 * len(offsets),
        "rle_total_bytes": len(rle) + 4 * len(offsets),
        "raw565_sha256": hashlib.sha256(raw).hexdigest(),
        "generated_c_sha256": hashlib.sha256(generated).hexdigest(),
        "rle_lossless": True,
    }, sort_keys=True))


if __name__ == "__main__":
    main()
