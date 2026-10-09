# AetherOS DSi NitroLauncher

A Nintendo DS/DSi homebrew browser built with the BlocksDS SDK. It initializes FAT through libnds, lists `.nds` files and subfolders, and supports D-pad navigation, paging, and folder traversal.

## Build and download

GitHub Actions builds the ROM in the BlocksDS slim container, checks Nintendo DS header fields, segment bounds and header CRC, calculates SHA-256, and publishes `NitroLauncher.nds` plus its checksum as an artifact.

- [Open GitHub Actions](https://github.com/wayout4/aetheros-dsi-launcher/actions)
- [View source](https://github.com/wayout4/aetheros-dsi-launcher/tree/main/source)

On a local BlocksDS installation, run `make`, then `python3 ci/validate_nds.py NitroLauncher.nds`.

## Use on DSi

1. Open the latest successful Actions run and download the `NitroLauncher-NDS` artifact.
2. Unzip it and copy `NitroLauncher.nds` to your SD card.
3. Start it through TWiLight Menu++.
4. Use Up/Down to select, L/R to page, A to open a folder or inspect a ROM, B to go to the parent folder, Y to rescan, and Start to exit.

## Seven-game homebrew setup

See [the seven-game starter catalog](docs/HOMEBREW_CATALOG.md) for legitimate download sources, game descriptions, and instructions for keeping all seven `.nds` files in the same folder. The current launcher supports up to 48 local `.nds` entries, so all seven fit. Required companion data folders must remain beside their game files.

See [the verification and game-development roadmap](docs/ROADMAP.md) for release gates and the separate AetherOS: Starbound Courier original-game plan.

## Important limitation

This version is a file-browser front end. Standard libnds does not provide a general safe API to chainload arbitrary NDS ROMs from an already-running homebrew application. Selecting a ROM displays instructions to return to TWiLight Menu++ and start it there; it does not pretend to launch it. A direct handoff would need a separately integrated and tested loader.

CI structural validation does not prove runtime boot. Emulator and physical DSi testing are still required before calling the ROM fully verified.
