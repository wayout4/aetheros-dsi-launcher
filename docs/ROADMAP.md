# AetherOS NitroLauncher + Starbound Courier roadmap

## Current confirmed baseline
- BlocksDS build workflow is configured to compile `NitroLauncher.nds`.
- CI runs `ci/validate_nds.py` to check title/game code, ARM9/ARM7 segment bounds, entry/load fields, and header CRC, then records SHA-256 as a build artifact.
- Launcher source initializes FAT, lists up to 48 local `.nds` files, sorts names, pages with L/R, and checks a selected file's header.
- The README/source explicitly says selection does not chainload. CI structural checks do not prove runtime boot.

## Release gates (do not mark complete until evidenced)
1. Green Actions run and downloadable `NitroLauncher-NDS` artifact.
2. Downloaded ROM passes the repository validator and SHA-256 is recorded.
3. Boot test in a DS emulator.
4. Physical DSi test: boot, SD scan, navigation, corrupt-file handling, exit, and TWiLight Menu++ launch.
5. Launch handoff test using a documented and licensed integration; verify save paths and return behavior.
6. Release a versioned artifact with build commit, checksum, and test matrix.

## Seven-game setup
See [HOMEBREW_CATALOG.md](HOMEBREW_CATALOG.md). The catalog is a download guide, not a bundle of copyrighted ROMs. Seven local files fit within the current 48-entry listing cap.

## Original game: AetherOS: Starbound Courier
Build separately from the launcher to keep the ROMs independently testable. Start with a small native DS demo:
- title screen and controls/tutorial
- top-screen exploration and bottom-screen map/inventory
- one planet, one delivery quest, dialogue, simple combat
- save/load with versioned save data and safe defaults
- test build with BlocksDS, structural validation, emulator boot, then physical DSi playtest

Do not describe the game as complete until those milestones are implemented and tested.