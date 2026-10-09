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

    require(re.search(r"#define\\s+MAX_ENTRIES\\s+128\\b", launcher),
            "launcher capacity must remain 128 entries")
    for token in ("KEY_UP", "KEY_DOWN", "KEY_L", "KEY_R", "KEY_B", "KEY_Y", "KEY_A"):
        require(token in launcher, f"launcher navigation control missing: {token}")
    require("does not chainload ROMs" in launcher,
            "launcher must not falsely imply selected ROMs are launched")
    require("s.checksum != checksum(&s)" in game,
            "save loader must reject invalid save checksums")
    require("s.magic != SAVE_MAGIC" in game and "s.version != 1" in game,
            "save loader must validate magic and version")
    require("fwrite(&game, sizeof(game), 1, f)" in game,
            "save path must write the complete save structure")
    require("fread(&s, 1, sizeof(s), f)" in game,
            "load path must read and validate the complete save structure")
    numbered = re.findall(r"^\\d+\\. \\[.+?\\]\\(", catalog, flags=re.MULTILINE)
    require(len(numbered) == 85,
            f"expanded catalog must contain exactly 85 numbered candidates; found {len(numbered)}")
    require("not 85 runtime-tested games" in catalog,
            "catalog must disclose that candidates are not runtime tested")
    require("does not prove runtime boot" in readme,
            "README must distinguish structural checks from runtime boot")
    print("PASS: launcher controls/capacity and honest chainload status")
    print("PASS: save/load integrity guards are present (source-level only)")
    print("PASS: expanded catalog has 85 candidates and compatibility disclaimer")
    print("PASS: runtime-test limitation is disclosed")


if __name__ == "__main__":
    main()
