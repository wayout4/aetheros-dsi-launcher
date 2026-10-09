// SPDX-License-Identifier: MIT
#include <ctype.h>
#include <dirent.h>
#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define MAX_ROMS 48
#define NAME_LEN 128
#define ROWS 12
static char roms[MAX_ROMS][NAME_LEN];
static unsigned count, selected, first;
static bool storage_ok;
static char folder[256];

static bool is_nds(const char *s) {
    size_t n = strlen(s);
    return n >= 4 && s[n-4] == '.' &&
        tolower((unsigned char)s[n-3]) == 'n' &&
        tolower((unsigned char)s[n-2]) == 'd' &&
        tolower((unsigned char)s[n-1]) == 's';
}
static void scan_files(void) {
    count = 0;
    char *cwd = fatGetDefaultCwd();
    if (cwd) { snprintf(folder, sizeof(folder), "%s", cwd); free(cwd); }
    else snprintf(folder, sizeof(folder), "%s", fatGetDefaultDrive());
    DIR *dir = opendir(folder);
    if (!dir) { storage_ok = false; return; }
    struct dirent *e;
    while ((e = readdir(dir)) && count < MAX_ROMS) {
        if (e->d_name[0] == '.' || !is_nds(e->d_name)) continue;
        snprintf(roms[count++], NAME_LEN, "%s", e->d_name);
    }
    closedir(dir);
    storage_ok = true;
    for (unsigned i = 1; i < count; i++) {
        char key[NAME_LEN]; snprintf(key, sizeof(key), "%s", roms[i]);
        unsigned j = i;
        while (j && strcasecmp(roms[j-1], key) > 0) {
            snprintf(roms[j], NAME_LEN, "%s", roms[j-1]); j--;
        }
        snprintf(roms[j], NAME_LEN, "%s", key);
    }
}
static void draw(const char *notice) {
    consoleClear();
    iprintf("\x1b[0;0HAetherOS NitroLauncher\n==============================\n");
    iprintf("Folder: %.35s\n\n", folder);
    if (!storage_ok) {
        iprintf("SD/FAT storage unavailable.\n");
        iprintf("Start from TWiLight Menu++ with\nreadable SD/flashcart storage.\n");
    } else if (!count) {
        iprintf("No .nds files in this folder.\n");
        iprintf("Put homebrew here or launch this\napp from the target folder.\n");
    } else {
        iprintf("Found %u .nds file(s), max %u.\n", count, MAX_ROMS);
        iprintf("UP/DOWN select  A details\nL/R page  B rescan\n\n");
        for (unsigned r = 0; r < ROWS && first+r < count; r++)
            iprintf("%s%s\n", first+r == selected ? "> " : "  ", roms[first+r]);
    }
    if (notice) iprintf("\n%s", notice);
    iprintf("\nSTART: exit to loader");
}
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    consoleDemoInit();
    iprintf("Starting NitroLauncher...\n");
    storage_ok = fatInitDefault();
    scan_files(); draw(NULL);
    for (;;) {
        swiWaitForVBlank(); scanKeys(); uint32_t k = keysDown();
        if (k & KEY_START) return 0;
        if (k & KEY_B) { storage_ok = fatInitDefault(); scan_files(); selected=first=0; draw("Rescan complete."); }
        if (!storage_ok || !count) continue;
        if (k & KEY_UP) { if (selected) selected--; if (selected < first) first=selected; draw(NULL); }
        if (k & KEY_DOWN) { if (selected+1<count) selected++; if (selected>=first+ROWS) first=selected-ROWS+1; draw(NULL); }
        if (k & KEY_L) { selected = selected>ROWS ? selected-ROWS : 0; first=selected; draw(NULL); }
        if (k & KEY_R) { selected+=ROWS; if (selected>=count) selected=count-1; first=selected; draw(NULL); }
        if (k & KEY_A) {
            consoleClear();
            iprintf("Selected homebrew\n==============================\n%s\n\n", roms[selected]);
            iprintf("This version browses files safely.\n");
            iprintf("libnds does not provide general safe\nchainloading for arbitrary NDS ROMs.\n\n");
            iprintf("Return to TWiLight Menu++ and select\nthe ROM there. Press B to go back.");
            for (;;) { swiWaitForVBlank(); scanKeys(); uint32_t d=keysDown(); if(d&KEY_START)return 0; if(d&KEY_B)break; }
            draw(NULL);
        }
    }
}
