# AetherOS NitroLauncher + Starbound Courier verification roadmap

## Latest automated build (2026-10-09)

- Successful GitHub Actions run: https://github.com/wayout4/aetheros-dsi-launcher/actions/runs/37888452407
- Verified commit: `6be0bcf621fc9cce7ca967bc598ebe975e2275c3`
- BlocksDS compiled `NitroLauncher.nds` and `game/StarboundCourier.nds`.
- Five synthetic ROM-validator regression cases passed, including rejection of a wrapped ARM9 load-address range.
- Four source-level project-contract checks passed. These are regression guards, not runtime tests.
- Both generated ROMs passed title/game-code, ROM segment bounds, entry/load address, and header CRC checks.
- SHA-256 files were generated and the full package and ROM-only artifacts uploaded.
- NitroLauncher supports up to 128 entries and browsing directories. It does not chainload selected games.

## Release gates — do not mark complete until evidence exists

1. **Emulator boot:** not yet performed. Boot each exact final artifact in a named emulator/version, record its version, settings, ROM SHA-256, boot result, and screenshots/logs.
2. **Physical DSi boot:** not yet performed. On a real console, record DSi model/region, firmware, SD card/filesystem, TWiLight Menu++ version, ROM SHA-256, boot result, and observed behavior.
3. **Launcher interaction:** verify FAT initialization, folder traversal, parent navigation, sorting, paging, rescan, handling empty/unreadable folders, malformed ROM inspection, and Start-to-exit on emulator and hardware.
4. **Game behavior:** verify collision, all three crystals, delivery, completion, and corrupt/missing save handling.
5. **Save persistence:** collect at least one crystal, save, exit normally, relaunch, confirm position and collection persist; then test corrupt and truncated save files. Repeat on emulator and hardware. A source-level check does not satisfy this gate.
6. **Direct launch/handoff:** not implemented. Integrate a pinned, documented TWiLight Menu++/nds-bootstrap handoff and test on supported setups before claiming selected games launch directly from NitroLauncher.
7. **85 additional games:** the catalog is a discovery list, not 85 verified games. Each candidate needs an identified legitimate release, license/redistribution review, companion-data notes, a recorded hash, and individual runtime testing before it can be called working.
8. **Release artifact:** download the newest successful package, verify the included SHA-256 files, and retain test evidence against the exact commit and artifact hashes.

## Hardware test record template

For each test, record:
- Date and tester:
- Device/model and firmware:
- SD card brand/capacity/filesystem:
- TWiLight Menu++ version and any flashcart/loader:
- ROM filename, commit, and SHA-256:
- Test case and exact steps:
- Expected result:
- Actual result:
- Pass/fail, notes, and photo/video or log evidence:

Do not infer a hardware pass from a successful CI build, valid header CRC, emulator result, or a catalog link.

## Homebrew catalog

- [Seven-game starter catalog](HOMEBREW_CATALOG.md)
- [85 additional discovery candidates](EXPANDED_HOMEBREW_CATALOG.md)

These links guide users to community catalog pages; they do not claim all candidates are currently downloadable, legally redistributable, or tested.

## Original game: AetherOS: Starbound Courier

The current ROM is a small prototype: move around a map, collect three crystals, deliver them to the beacon, and save/load progress using `starbound.sav`. It is not a polished or complete game. Runtime checks for collisions, mission completion, save persistence, and corrupt-save handling remain open.

## Release principle

Only claim a test has passed when a recorded result from that test exists. Structural ROM validation is useful, but it does not substitute for emulator or physical-console boot tests.
