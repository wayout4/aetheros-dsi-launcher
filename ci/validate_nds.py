#!/usr/bin/env python3
"""Structural checks only; does not prove runtime boot."""
import pathlib, struct, sys

def crc16(data, crc=0xFFFF):
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc

def main():
    if len(sys.argv) != 2: raise SystemExit("usage: validate_nds.py ROM.nds")
    p = pathlib.Path(sys.argv[1]); d = p.read_bytes()
    if len(d) < 0x200: raise SystemExit("FAIL: file too small for NDS header")
    title = d[:12].split(b"\0",1)[0].decode("ascii","replace").strip()
    code = d[12:16].decode("ascii","replace")
    if not title or title.upper() == "HOMEBREW": raise SystemExit(f"FAIL: placeholder title {title!r}")
    if code == "####" or not all(32 <= ord(c) <= 126 for c in code): raise SystemExit(f"FAIL: invalid game code {code!r}")
    a9, e9, l9, s9 = struct.unpack_from("<IIII", d, 0x20)
    a7, e7, l7, s7 = struct.unpack_from("<IIII", d, 0x30)
    for name, off, size in (("ARM9",a9,s9),("ARM7",a7,s7)):
        if off < 0x200 or size <= 0 or off+size > len(d): raise SystemExit(f"FAIL: {name} segment out of bounds")
    if not all((e9,l9,e7,l7)): raise SystemExit("FAIL: zero entry/load address")
    stored = struct.unpack_from("<H", d, 0x15E)[0]; computed = crc16(d[:0x15E])
    if stored != computed: raise SystemExit(f"FAIL: header CRC stored={stored:#06x} computed={computed:#06x}")
    print(f"PASS: {p} {len(d)} bytes; title={title!r}; code={code!r}; CRC={stored:#06x}")
    print("Structural checks passed only; emulator/physical boot remains unverified.")

if __name__ == "__main__": main()
