# AetherOS NitroLauncher + Starbound Courier verification roadmap

## Confirmed automated build status

- The latest successful pre-package build is available through GitHub Actions and compiles both NitroLauncher and Starbound Courier with BlocksDS.
- CI runs four validator regression cases and validates each generated ROM's title/game code, ARM9/ARM7 bounds, entry/load addresses, and header CRC.
- CI records SHA-256 checksums.
- The launcher supports up to 128 local entries, navigates folders, and inspects selected ROM headers.
- A new workflow change assembles a full ZIP with both original ROMs, checksums, setup notes, and homebrew discovery catalogs. The run for that change must finish successfully before its artifact is available.

## Explicitly unverified release gates

1. Direct launch from NitroLauncher: not implemented. A supported TWiLight Menu++ / nds-bootstrap handoff must be integrated against a pinned, documented interface, not guessed.
2. Additional homebrew: the 85-game catalog is a discovery list only. The repository does not bundle those third-party binaries. Their exact releases, licenses, data dependencies, and compatibility must be individually reviewed.
3. Emulator test: not performed by the structural validator. A ROM that passes header checks is not thereby proven to boot.
4. Physical DSi test: not performed. Hardware boot, SD access, navigation, and handoff need a real device test.
5. Starbound Courier persistence: save/load code is present, but a gameplay-save-exit-restart session has not been verified on emulator or physical DSi.
6. Release artifact: use the newest successful workflow run, and check its SHA-256 files after downloading.

## Homebrew catalog

- [Seven-game starter catalog](HOMEBREW_CATALOG.md)
- [85 additional discovery candidates](EXPANDED_HOMEBREW_CATALOG.md)

These links guide users to community catalog pages; they do not claim all candidates are currently downloadable, legally redistributable, or tested.

## Original game: AetherOS: Starbound Courier

The current ROM is a small playable prototype: move around a map, collect three crystals, deliver them to the beacon, and save/load progress using `starbound.sav`. It is not a polished or complete game. Runtime checks for map collisions, mission completion, persistence, and corrupt save handling remain open.

## Release principle

Only claim a test has passed when there is a recorded result from that test. Structural ROM validation is useful, but it does not substitute for emulator or physical-console boot tests.
