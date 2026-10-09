# SPDX-License-Identifier: MIT
BLOCKSDS ?= /opt/blocksds/core
NAME := NitroLauncher
GAME_TITLE := AetherOS Nitro
GAME_SUBTITLE := DSi Homebrew Browser
GAME_SUBTITLE2 := Safe Launcher
LIBS := -lnds9 -lc
include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile

# Set real NDS header identity; -b above controls the banner text/icon.
NDSTOOL_ARGS += -t "AETHEROS NIT" -g "AOSL" -m "01"
