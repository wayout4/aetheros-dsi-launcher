# AetherOS DSi NitroLauncher

A Nintendo DS/DSi homebrew browser built with the BlocksDS SDK. It initializes FAT through libnds, lists `.nds` files and subfolders, and supports D-pad navigation, paging, and folder traversal.

## Build and download

GitHub Actions builds the ROM in the BlocksDS slim container, checks Nintendo DS header fields, segment bounds and header CRC, calculates SHA-256, and publishes `NitroLauncher.nds` plus its checksum as an artifact.

- [Open GitHub Actions](https://github.com/wayout4/aetheros-dsi-launcher/actions)
- [View source](https://github.com/wayout4/aetheros-dsi-launcher/tree/main/source)

On a local BlocksDS installation, run `make`, then `python3 ci/validate_nds.py NitroLauncher.nds`.

## Use on DSi

1. Open the latest successful Actions run and download the `AetherOS-DS-ROMs` artifact (it contains both `NitroLauncher.nds` and `StarboundCourier.nds`, plus SHA-256 files).
2. Unzip it and copy `NitroLauncher.nds` to your SD card.
3. Start it through TWiLight Menu++.
4. Use Up/Down to select, L/R to page, A to open a folder or inspect a ROM, B to go to the parent folder, Y to rescan, and Start to exit.

## Seven-game homebrew setup

See [the seven-game starter catalog](docs/HOMEBREW_CATALOG.md) for setup notes and [the expanded 85-game discovery catalog](docs/EXPANDED_HOMEBREW_CATALOG.md) for additional homebrew candidates. These are discovery links, not a ROM bundle or a claim that every candidate has been runtime-tested. NitroLauncher can list up to 128 local entries; required companion data folders must remain at the paths specified by each game's author.

See [the verification and game-development roadmap](docs/ROADMAP.md) for release gates and the separate AetherOS: Starbound Courier original-game plan.

## Important limitation

This version is a file-browser front end. Standard libnds does not provide a general safe API to chainload arbitrary NDS ROMs from an already-running homebrew application. Selecting a ROM displays instructions to return to TWiLight Menu++ and start it there; it does not pretend to launch it. A direct handoff would need a separately integrated and tested loader.

CI structural validation does not prove runtime boot. Emulator and physical DSi testing are still required before calling the ROM fully verified.
