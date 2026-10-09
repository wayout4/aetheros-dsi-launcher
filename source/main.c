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

static uint16_t header_crc16(const unsigned char *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; bit++)
            crc = (crc & 1) ? (uint16_t)((crc >> 1) ^ 0xA001) : (uint16_t)(crc >> 1);
    }
    return crc;
}
static uint32_t read_le32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static bool validate_rom_file(const char *name, char *why, size_t why_len) {
    FILE *fp = fopen(name, "rb");
    if (!fp) { snprintf(why, why_len, "Cannot open file"); return false; }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp); snprintf(why, why_len, "Cannot read file size"); return false;
    }
    long length = ftell(fp);
    unsigned char h[0x200];
    if (length < (long)sizeof(h) || fseek(fp, 0, SEEK_SET) != 0 ||
        fread(h, 1, sizeof(h), fp) != sizeof(h)) {
        fclose(fp); snprintf(why, why_len, "File is too small or unreadable"); return false;
    }
    fclose(fp);
    if (h[0] == 0 || memcmp(h, "HOMEBREW", 8) == 0 ||
        memcmp(h + 0x0C, "####", 4) == 0) {
        snprintf(why, why_len, "Placeholder NDS header"); return false;
    }
    uint32_t a9 = read_le32(h + 0x20), s9 = read_le32(h + 0x2C);
    uint32_t a7 = read_le32(h + 0x30), s7 = read_le32(h + 0x3C);
    if (a9 < 0x200 || s9 == 0 || (uint64_t)a9 + s9 > (uint64_t)length ||
        a7 < 0x200 || s7 == 0 || (uint64_t)a7 + s7 > (uint64_t)length) {
        snprintf(why, why_len, "ARM9/ARM7 segment bounds invalid"); return false;
    }
    uint16_t stored = (uint16_t)h[0x15E] | ((uint16_t)h[0x15F] << 8);
    if (stored != header_crc16(h, 0x15E)) {
        snprintf(why, why_len, "Header CRC mismatch"); return false;
    }
    snprintf(why, why_len, "Header and segment checks passed");
    return true;
}

static void draw(const char *notice) {
    consoleClear();
    printf("\x1b[0;0HAetherOS NitroLauncher\n==============================\n");
    printf("Folder: %.35s\n\n", folder);
    if (!storage_ok) {
        printf("SD/FAT storage unavailable.\n");
        printf("Start from TWiLight Menu++ with\nreadable SD/flashcart storage.\n");
    } else if (!count) {
        printf("No .nds files in this folder.\n");
        printf("Put homebrew here or launch this\napp from the target folder.\n");
    } else {
        printf("Found %u .nds file(s), max %u.\n", count, MAX_ROMS);
        printf("UP/DOWN select  A details\nL/R page  B rescan\n\n");
        for (unsigned r = 0; r < ROWS && first+r < count; r++)
            printf("%s%s\n", first+r == selected ? "> " : "  ", roms[first+r]);
    }
    if (notice) printf("\n%s", notice);
    printf("\nSTART: exit to loader");
}
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    consoleDemoInit();
    printf("Starting NitroLauncher...\n");
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
            char detail[96];
            bool valid = validate_rom_file(roms[selected], detail, sizeof(detail));
            printf("Selected homebrew\n==============================\n%s\n\n", roms[selected]);
            printf("Header check: %s\n%s\n\n", valid ? "PASS" : "FAIL", detail);
            printf("This launcher does not chainload ROMs.\n");
            printf("To run it, return to TWiLight Menu++\nand select the ROM there.\n\n");
            printf("Press B to return to the list.");
            for (;;) { swiWaitForVBlank(); scanKeys(); uint32_t d=keysDown(); if(d&KEY_START)return 0; if(d&KEY_B)break; }
            draw(NULL);
        }
    }
}
