#!/usr/bin/env python3
"""Structural checks for Nintendo DS ROMs; does not prove runtime boot."""
import pathlib
import struct
import sys


def crc16(data, crc=0xFFFF):
    # Nintendo DS BIOS swiCRC16: reflected CRC-16/IBM (poly 0xA001).
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc >> 1) ^ 0xA001) if crc & 1 else (crc >> 1)
    return crc & 0xFFFF


def validate(path):
    p = pathlib.Path(path)
    d = p.read_bytes()
    if len(d) < 0x200:
        raise ValueError("file too small for NDS header")
    title = d[:12].split(b"\0", 1)[0].decode("ascii", "replace").strip()
    code = d[12:16].decode("ascii", "replace")
    if not title or title.upper() == "HOMEBREW":
        raise ValueError(f"placeholder title {title!r}")
    if code == "####" or not all(32 <= ord(c) <= 126 for c in code):
        raise ValueError(f"invalid game code {code!r}")
    a9, e9, l9, s9 = struct.unpack_from("<IIII", d, 0x20)
    a7, e7, l7, s7 = struct.unpack_from("<IIII", d, 0x30)
    for name, off, size in (("ARM9", a9, s9), ("ARM7", a7, s7)):
        if off < 0x200 or size <= 0 or off + size > len(d):
            raise ValueError(f"{name} segment out of bounds")
    for name, entry, load, size in (("ARM9", e9, l9, s9), ("ARM7", e7, l7, s7)):
        if not entry or not load or entry < load or entry >= load + size:
            raise ValueError(f"{name} entry/load address invalid")
    stored = struct.unpack_from("<H", d, 0x15E)[0]
    computed = crc16(d[:0x15E])
    if stored != computed:
        raise ValueError(f"header CRC stored={stored:#06x} computed={computed:#06x}")
    return f"PASS: {p} {len(d)} bytes; title={title!r}; code={code!r}; CRC={stored:#06x}"


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: validate_nds.py ROM.nds")
    try:
        print(validate(sys.argv[1]))
        print("Structural checks passed only; emulator/physical boot remains unverified.")
    except (OSError, ValueError) as exc:
        raise SystemExit(f"FAIL: {exc}") from exc


if __name__ == "__main__":
    main()
