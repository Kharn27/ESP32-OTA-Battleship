#!/usr/bin/env python3
"""Validate CYD WARSHIP's graphic asset and screen catalog (stdlib only).

This checks structure and file integrity, NOT artistic approval or CYD hardware.
Run from any directory: python3 scripts/check_graphic_pipeline.py
"""
from __future__ import annotations

import hashlib
import json
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets"
SCREENS = ROOT / "maquettes"
GRAPHIC_EXTENSIONS = {".png", ".jpg", ".jpeg", ".gif", ".webp", ".bmp", ".svg", ".apng", ".ico"}
HEX64 = re.compile(r"[0-9a-f]{64}\Z")
IDENTIFIER = re.compile(r"[a-z0-9]+(?:[._-][a-z0-9]+)*\Z")
APPROVAL_PREFIX = "https://github.com/Kharn27/ESP32-OTA-Battleship/"


def fail(message: str) -> None:
    raise ValueError(message)


def load_json(path: Path) -> dict:
    try:
        document = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        fail(f"{path.relative_to(ROOT)}: cannot load JSON: {exc}")
    if not isinstance(document, dict):
        fail(f"{path.relative_to(ROOT)}: expected JSON object")
    return document


def file_in(raw: object, directory: Path, field: str) -> Path:
    if not isinstance(raw, str) or not raw or "\\" in raw or raw.startswith("/"):
        fail(f"{field}: invalid repository-relative path")
    path = (ROOT / raw).resolve()
    if not path.is_relative_to(directory.resolve()) or not path.is_file():
        fail(f"{field}: missing file or path escapes {directory.relative_to(ROOT)}: {raw!r}")
    return path


def sha256(path: Path) -> str:
    hasher = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(65536), b""):
            hasher.update(block)
    return hasher.hexdigest()


def approval(value: object, field: str) -> None:
    if not isinstance(value, str) or not value.startswith(APPROVAL_PREFIX):
        fail(f"{field}: require a traceable PO approval GitHub URL")
    if not ("/issues/" in value or "/pull/" in value):
        fail(f"{field}: require an Issue/PR decision link, not a generic URL")


def graphic_files(folder: Path) -> set[Path]:
    return {p.resolve() for p in folder.rglob("*") if p.is_file() and p.suffix.lower() in GRAPHIC_EXTENSIONS}


def check_manifest() -> tuple[dict[str, Path], set[str]]:
    doc = load_json(ASSETS / "manifest.json")
    if doc.get("schema_version") != 1 or not isinstance(doc.get("assets"), list):
        fail("assets/manifest.json: expected schema_version 1 and assets array")
    identifiers: dict[str, Path] = {}
    registered: set[Path] = set()
    fingerprints: set[str] = set()
    for index, entry in enumerate(doc["assets"]):
        field = f"assets[{index}]"
        if not isinstance(entry, dict):
            fail(f"{field}: expected object")
        name = entry.get("id")
        if not isinstance(name, str) or not IDENTIFIER.fullmatch(name) or name in identifiers:
            fail(f"{field}.id: missing, invalid or duplicate")
        asset = file_in(entry.get("path"), ASSETS, f"{field}.path")
        if asset.suffix.lower() not in GRAPHIC_EXTENSIONS:
            fail(f"{field}.path: expected graphical export")
        if asset in registered:
            fail(f"{field}.path: duplicate catalog path")
        expected_sha = entry.get("sha256")
        if not isinstance(expected_sha, str) or not HEX64.fullmatch(expected_sha):
            fail(f"{field}.sha256: expected lowercase SHA-256")
        if sha256(asset) != expected_sha:
            fail(f"{field}: file bytes differ from approved SHA-256: {asset.relative_to(ROOT)}")
        if not isinstance(entry.get("source_ref"), str) or not entry["source_ref"].strip():
            fail(f"{field}.source_ref: source/provenance missing")
        approval(entry.get("po_validation_ref"), f"{field}.po_validation_ref")
        status = entry.get("device_status")
        if status not in ("ON_DEVICE_UNVERIFIED", "ON_DEVICE_VERIFIED"):
            fail(f"{field}.device_status: invalid/missing")
        if status == "ON_DEVICE_VERIFIED":
            evidence = entry.get("device_evidence")
            if not isinstance(evidence, str) or not evidence.strip():
                fail(f"{field}.device_evidence: physical evidence required")
        identifiers[name] = asset
        registered.add(asset)
        fingerprints.add(expected_sha)
    unregistered = graphic_files(ASSETS) - registered
    if unregistered:
        files = ", ".join(str(p.relative_to(ROOT)) for p in sorted(unregistered))
        fail(f"unregistered graphic files under assets/: {files}")
    return identifiers, fingerprints


def png_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as stream:
        head = stream.read(24)
    if len(head) < 24 or head[:8] != b"\x89PNG\r\n\x1a\n" or head[12:16] != b"IHDR":
        fail(f"{path.relative_to(ROOT)}: expected PNG preview")
    return struct.unpack(">II", head[16:24])


def check_screens(identifiers: dict[str, Path], fingerprints: set[str]) -> int:
    count = 0
    registered_images: set[Path] = set()
    for directory in sorted(SCREENS.iterdir()):
        if not directory.is_dir():
            continue
        doc = load_json(directory / "screen.json")
        field = str(directory.relative_to(ROOT))
        if doc.get("schema_version") != 1 or doc.get("id") != directory.name:
            fail(f"{field}/screen.json: schema_version or id mismatch")
        if doc.get("size") != [320, 240]:
            fail(f"{field}/screen.json: screen must be 320x240 landscape")
        used = doc.get("assets")
        if not isinstance(used, list) or not used or any(not isinstance(x, str) for x in used):
            fail(f"{field}/screen.json: assets must be non-empty list of IDs")
        if len(set(used)) != len(used):
            fail(f"{field}/screen.json: duplicate asset IDs")
        missing = set(used) - set(identifiers)
        if missing:
            fail(f"{field}/screen.json: unknown asset IDs: {sorted(missing)}")
        status = doc.get("status")
        if status not in ("CANDIDATE", "VALIDATED_PO_PC"):
            fail(f"{field}/screen.json: invalid status")
        if status == "VALIDATED_PO_PC":
            approval(doc.get("po_validation_ref"), f"{field}.po_validation_ref")
        preview = file_in(doc.get("preview"), directory, f"{field}.preview")
        if preview.suffix.lower() != ".png" or png_size(preview) != (320, 240):
            fail(f"{field}: preview must be a real 320x240 PNG")
        proofs = doc.get("proofs", [])
        if not isinstance(proofs, list):
            fail(f"{field}/screen.json: proofs must be a list")
        declared = {preview}
        for index, item in enumerate(proofs):
            proof = file_in(item, directory, f"{field}.proofs[{index}]")
            declared.add(proof)
        registered_images.update(p for p in declared if p.suffix.lower() in GRAPHIC_EXTENSIONS)
        for image in declared:
            if image.suffix.lower() in GRAPHIC_EXTENSIONS and sha256(image) in fingerprints:
                fail(f"{field}: file duplicates an asset; reference its ID instead: {image.name}")
        count += 1
    unregistered = graphic_files(SCREENS) - registered_images
    if unregistered:
        files = ", ".join(str(p.relative_to(ROOT)) for p in sorted(unregistered))
        fail(f"undeclared visual files under maquettes/: {files}")
    return count


def main() -> int:
    try:
        identifiers, fingerprints = check_manifest()
        screen_count = check_screens(identifiers, fingerprints)
    except (ValueError, OSError) as exc:
        print(f"GRAPHIC_PIPELINE_FAIL: {exc}", file=sys.stderr)
        return 1
    print(f"GRAPHIC_PIPELINE_OK: {len(identifiers)} approved assets; {screen_count} declared screens")
    print("Scope: structure and file integrity only; not PO/artistic or physical CYD validation")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
