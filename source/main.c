// SPDX-License-Identifier: MIT
#include <ctype.h>
#include <dirent.h>
#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#define MAX_ENTRIES 128
#define NAME_LEN 128
#define ROWS 12

typedef struct {
    char name[NAME_LEN];
    bool is_dir;
} Entry;

static Entry entries[MAX_ENTRIES];
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
static bool is_root_path(const char *p) {
    if (!p || !*p) return true;
    size_t n = strlen(p);
    while (n > 1 && (p[n-1] == '/' || p[n-1] == '\\')) n--;
    const char *last = p + n - 1;
    while (last > p && last[-1] != '/' && last[-1] != ':') last--;
    return last == p || (last == p + 1 && p[0] == '/');
}
static void scan_files(void) {
    count = 0;
    // Do not mistake a readable current directory for initialized SD/FAT.
    // fatInitDefault() is the authority for whether removable storage is ready.
    if (!storage_ok) {
        folder[0] = '\0';
        return;
    }
    if (!getcwd(folder, sizeof(folder))) {
        const char *drive = fatGetDefaultDrive();
        snprintf(folder, sizeof(folder), "%s", drive ? drive : "SD");
    }
    DIR *dir = opendir(".");
    if (!dir) { storage_ok = false; return; }
    struct dirent *e;
    while ((e = readdir(dir)) && count < MAX_ENTRIES) {
        if (e->d_name[0] == '.' || !strcmp(e->d_name, "..")) continue;
        struct stat st;
        if (stat(e->d_name, &st) != 0) continue;
        bool is_dir = S_ISDIR(st.st_mode);
        if (!is_dir && !is_nds(e->d_name)) continue;
        snprintf(entries[count].name, NAME_LEN, "%s", e->d_name);
        entries[count].is_dir = is_dir;
        count++;
    }
    closedir(dir);
    // Directories first, then case-insensitive alphabetical order.
    for (unsigned i = 1; i < count; i++) {
        Entry key = entries[i];
        unsigned j = i;
        while (j && ((entries[j-1].is_dir < key.is_dir) ||
              (entries[j-1].is_dir == key.is_dir &&
               strcasecmp(entries[j-1].name, key.name) > 0))) {
            entries[j] = entries[j-1];
            j--;
        }
        entries[j] = key;
    }
    if (selected >= count) selected = count ? count - 1 : 0;
    if (first > selected) first = selected;
    if (selected >= first + ROWS) first = selected - ROWS + 1;
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
    uint32_t a9 = read_le32(h + 0x20), e9 = read_le32(h + 0x24);
    uint32_t l9 = read_le32(h + 0x28), s9 = read_le32(h + 0x2C);
    uint32_t a7 = read_le32(h + 0x30), e7 = read_le32(h + 0x34);
    uint32_t l7 = read_le32(h + 0x38), s7 = read_le32(h + 0x3C);
    if (a9 < 0x200 || s9 == 0 || (uint64_t)a9 + s9 > (uint64_t)length ||
        a7 < 0x200 || s7 == 0 || (uint64_t)a7 + s7 > (uint64_t)length) {
        snprintf(why, why_len, "ARM9/ARM7 ROM segment bounds invalid"); return false;
    }
    if (!e9 || !e7 || !l9 || !l7 ||
        (uint64_t)l9 + s9 > 0x100000000ULL ||
        (uint64_t)l7 + s7 > 0x100000000ULL ||
        e9 < l9 || (uint64_t)e9 >= (uint64_t)l9 + s9 ||
        e7 < l7 || (uint64_t)e7 >= (uint64_t)l7 + s7) {
        snprintf(why, why_len, "ARM entry/load addresses invalid"); return false;
    }
    uint16_t stored = (uint16_t)h[0x15E] | ((uint16_t)h[0x15F] << 8);
    if (stored != header_crc16(h, 0x15E)) {
        snprintf(why, why_len, "Header CRC mismatch"); return false;
    }
    snprintf(why, why_len, "Header CRC, segments and entry points passed");
    return true;
}

static void draw(const char *notice) {
    consoleClear();
    printf("\x1b[0;0HAetherOS NitroLauncher\n==============================\n");
    printf("Folder: %.38s\n\n", folder);
    if (!storage_ok) {
        printf("SD/FAT storage unavailable.\n");
        printf("Start from TWiLight Menu++ with\nreadable SD/flashcart storage.\n");
    } else if (!count) {
        printf("No folders or .nds files here.\n");
        printf("Press B to go up; Y to rescan.\n");
    } else {
        printf("%u entries (max %u).\n", count, MAX_ENTRIES);
        printf("UP/DOWN select  A open/details\nL/R page  B parent  Y rescan\n\n");
        for (unsigned r = 0; r < ROWS && first+r < count; r++) {
            Entry *e = &entries[first+r];
            printf("%s%s%s\n", first+r == selected ? "> " : "  ",
                   e->is_dir ? "[+] " : "    ", e->name);
        }
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
        if (k & KEY_Y) { scan_files(); draw("Rescan complete."); }
        if (k & KEY_B) {
            (void)chdir("..");
            selected = first = 0;
            scan_files();
            draw("Moved to parent folder.");
        }
        if (!storage_ok || !count) continue;
        if (k & KEY_UP) { if (selected) selected--; if (selected < first) first=selected; draw(NULL); }
        if (k & KEY_DOWN) { if (selected+1<count) selected++; if (selected>=first+ROWS) first=selected-ROWS+1; draw(NULL); }
        if (k & KEY_L) { selected = selected>ROWS ? selected-ROWS : 0; first=selected; draw(NULL); }
        if (k & KEY_R) { selected+=ROWS; if (selected>=count) selected=count-1; first=selected; draw(NULL); }
        if (k & KEY_A) {
            Entry *entry = &entries[selected];
            if (entry->is_dir) {
                if (chdir(entry->name) == 0) {
                    selected = first = 0;
                    scan_files();
                    draw("Folder opened.");
                } else draw("Could not open folder.");
                continue;
            }
            consoleClear();
            char detail[112];
            bool valid = validate_rom_file(entry->name, detail, sizeof(detail));
            printf("Selected homebrew\n==============================\n%s\n\n", entry->name);
            printf("Header check: %s\n%s\n\n", valid ? "PASS" : "FAIL", detail);
            printf("This launcher does not chainload ROMs.\n");
            printf("Use TWiLight Menu++ to launch the game.\n\n");
            printf("Press B to return to the list.");
            for (;;) { swiWaitForVBlank(); scanKeys(); uint32_t d=keysDown(); if(d&KEY_START)return 0; if(d&KEY_B)break; }
            draw(NULL);
        }
    }
}
