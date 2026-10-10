#!/usr/bin/env python3
"""Source-level release-contract checks; these do not replace emulator/hardware tests."""
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[1]


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def main():
    launcher = (ROOT / "source" / "main.c").read_text(encoding="utf-8")
    game = (ROOT / "game" / "source" / "main.c").read_text(encoding="utf-8")
    catalog = (ROOT / "docs" / "EXPANDED_HOMEBREW_CATALOG.md").read_text(encoding="utf-8")
    readme = (ROOT / "README.md").read_text(encoding="utf-8")

    require(re.search(r"#define\s+MAX_ENTRIES\s+128\b", launcher),
            "launcher capacity must remain 128 entries")
    for token in ("KEY_UP", "KEY_DOWN", "KEY_L", "KEY_R", "KEY_B", "KEY_Y", "KEY_A"):
        require(token in launcher, f"launcher navigation control missing: {token}")
    require("does not chainload ROMs" in launcher,
            "launcher must not falsely imply selected ROMs are launched")

    # Keep the DS console's ANSI color sequences correctly escaped in C source.
    for color in (r"\\x1b[36;1m", r"\\x1b[35m", r"\\x1b[32;1m", r"\\x1b[31;1m"):
        require(color in launcher, f"launcher color sequence missing: {color}")
    require(r"\\\\x1b" not in launcher and r"\\\\n" not in launcher,
            "launcher must not contain double-escaped ANSI/newline sequences")

    # Prevent scan_files() from overriding fatInitDefault() failure merely
    # because the current working directory happens to be readable.
    scan_match = re.search(r"static void scan_files\(void\) \{(.*?)\n\}", launcher, re.S)
    require(scan_match is not None, "launcher scan_files() implementation missing")
    scan_body = scan_match.group(1)
    guard = scan_body.find("if (!storage_ok)")
    opendir = scan_body.find('opendir(".")')
    require(guard >= 0 and opendir >= 0 and guard < opendir,
            "scan_files() must check FAT initialization before opening a directory")
    require("storage_ok = true" not in scan_body,
            "scan_files() must not manufacture a successful FAT initialization result")
    require("if (!dir) { storage_ok = false; return; }" in scan_body,
            "directory-open failure must mark storage unavailable")

    require("s.checksum != checksum(&s)" in game,
            "save loader must reject invalid save checksums")
    require("s.magic != SAVE_MAGIC" in game and "s.version != 1" in game,
            "save loader must validate magic and version")
    require("fwrite(&game, sizeof(game), 1, f)" in game,
            "save path must write the complete save structure")
    require("fread(&s, 1, sizeof(s), f)" in game,
            "load path must read and validate the complete save structure")
    numbered = re.findall(r"^\d+\. \[.+?\]\(", catalog, flags=re.MULTILINE)
    require(len(numbered) == 85,
            f"expanded catalog must contain exactly 85 numbered candidates; found {len(numbered)}")
    require("not 85 runtime-tested games" in catalog,
            "catalog must disclose that candidates are not runtime tested")
    require("does not prove runtime boot" in readme,
            "README must distinguish structural checks from runtime boot")
    print("PASS: launcher controls/capacity and honest chainload status")
    print("PASS: SD/FAT initialization failure cannot be masked by a readable directory")
    print("PASS: save/load integrity guards are present (source-level only)")
    print("PASS: expanded catalog has 85 candidates and compatibility disclaimer")
    print("PASS: runtime-test limitation is disclosed")


if __name__ == "__main__":
    main()
