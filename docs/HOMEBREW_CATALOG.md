# NitroLauncher: Seven-Game Homebrew Starter Catalog

This is a curated, legal homebrew starter list for AetherOS NitroLauncher. The launcher currently lists local `.nds` files in its working folder (up to 128 entries); it does not download games and does not chainload a selected game. Use TWiLight Menu++ to start a listed game until a tested handoff is integrated.

## Recommended seven

| # | Game | What it is | Official/catalog download |
|---|---|---|---|
| 1 | DScraft | Minecraft-inspired sandbox; extract the game data folder alongside the NDS file as directed by its page. | https://db.universal-team.net/ds/dscraft |
| 2 | TerrariaDS | Original 2D sandbox remake for Nintendo DS. | https://db.universal-team.net/ds/ |
| 3 | Wordle DS | Word puzzle game for DS/DSi. | https://db.universal-team.net/ds/ |
| 4 | Derailed! | Train-running co-op-inspired challenge; catalog lists an alpha release, so expect unfinished content. | https://www.gamebrew.org/wiki/List_of_DS_homebrew_games |
| 5 | MicroCityNDS | City-building/simulation homebrew. | https://db.universal-team.net/ds/ |
| 6 | WHITE SPACE DS | OMORI-inspired homebrew fan game. | https://db.universal-team.net/ds/ |
| 7 | Blimp Chicken DS | 3D arcade airship combat game. | https://www.gamebrew.org/wiki/List_of_DS_homebrew_games |

Catalog listings and versions change. Open each game's page, review its license and installation notes, and use the author's linked release when available. This repository does not redistribute these games or claim that every version has been tested on physical DSi hardware.

## Put all seven in one launcher folder

1. Download the DS/DSi homebrew `.nds` files from their catalog pages.
2. Copy the seven `.nds` files to the same SD-card folder from which NitroLauncher is launched.
3. Keep each game's required companion data folders beside its `.nds` file; DScraft, for example, needs its `dscraft` data directory.
4. Start `NitroLauncher.nds` from TWiLight Menu++.
5. Use Up/Down to select, L/R to page, A to inspect the header, and B to rescan.
6. To actually run a selected game in this version, exit to TWiLight Menu++ and select that game there. The current launcher intentionally does not pretend that selection equals launching.

The current source has a 128-entry listing cap, so seven games fit comfortably. This is a local-file launcher, not a network downloader or a ROM pack.

## Best next technical milestone

Integrate an established launch path such as the DS-Homebrew `nds-bootstrap` / TWiLight Menu++ flow only after confirming the supported invocation contract, license, DSi-mode requirements, and return behavior. Do not copy an arbitrary binary loader into this project without reviewing its license and testing it on emulator and hardware.

References:
- DS-Homebrew guidance for homebrew downloads and legally dumping commercial games: https://wiki.ds-homebrew.com/twilightmenu/faq.html
- Universal-DB DS catalog: https://db.universal-team.net/ds/
- GameBrew DS homebrew list: https://www.gamebrew.org/wiki/List_of_DS_homebrew_games
- nds-bootstrap project and compatibility information: https://github.com/DS-Homebrew/nds-bootstrap
