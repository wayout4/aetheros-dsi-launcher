#!/usr/bin/env python3
"""Regression tests for the NDS structural validator using synthetic ROM headers."""
import pathlib
import struct
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
VALIDATOR = ROOT / "ci" / "validate_nds.py"


def crc16(data, crc=0xFFFF):
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc >> 1) ^ 0xA001) if crc & 1 else (crc >> 1)
    return crc & 0xFFFF


def make_rom():
    data = bytearray(0x400)
    data[0:12] = b"TEST ROM\0\0\0\0"
    data[12:16] = b"TST1"
    struct.pack_into("<IIII", data, 0x20, 0x200, 0x02000000, 0x02000000, 0x100)
    struct.pack_into("<IIII", data, 0x30, 0x300, 0x03800000, 0x03800000, 0x100)
    struct.pack_into("<H", data, 0x15E, crc16(data[:0x15E]))
    return data


def run_case(name, data, expect_ok):
    with tempfile.TemporaryDirectory() as temp:
        path = pathlib.Path(temp) / "case.nds"
        path.write_bytes(data)
        result = subprocess.run([sys.executable, str(VALIDATOR), str(path)],
                                text=True, capture_output=True)
        ok = result.returncode == 0
        if ok != expect_ok:
            raise AssertionError(
                f"{name}: expected success={expect_ok}, got {result.returncode}; "
                f"stdout={result.stdout!r}; stderr={result.stderr!r}"
            )


def main():
    valid = make_rom()
    run_case("valid header", valid, True)
    bad_crc = bytearray(valid)
    bad_crc[0] ^= 1
    run_case("corrupt header CRC", bad_crc, False)
    bad_bounds = bytearray(valid)
    struct.pack_into("<I", bad_bounds, 0x2C, 0x500)
    struct.pack_into("<H", bad_bounds, 0x15E, crc16(bad_bounds[:0x15E]))
    run_case("out-of-bounds ARM9 segment", bad_bounds, False)
    bad_entry = bytearray(valid)
    struct.pack_into("<I", bad_entry, 0x24, 0x02000200)
    struct.pack_into("<H", bad_entry, 0x15E, crc16(bad_entry[:0x15E]))
    run_case("entry point outside loaded ARM9 segment", bad_entry, False)
    print("PASS: 4 validator regression cases")


if __name__ == "__main__":
    main()
