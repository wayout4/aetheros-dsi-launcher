// SPDX-License-Identifier: MIT
// AetherOS: Starbound Courier — playable first prototype for Nintendo DS.
#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#define MAP_W 16
#define MAP_H 10
#define SAVE_PATH "starbound.sav"
#define SAVE_MAGIC 0x53424331u

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t x, y, crystals, delivered, collected_mask;
    uint32_t checksum;
} SaveData;

static const char *world[MAP_H] = {
    "################",
    "#....#.........#",
    "#....#..C......#",
    "#..............#",
    "#..C......##...#",
    "#..............#",
    "#......C.......#",
    "#...........B..#",
    "#..............#",
    "################"
};
static const uint8_t crystal_x[3] = {8, 3, 7};
static const uint8_t crystal_y[3] = {2, 4, 6};
static char map[MAP_H][MAP_W + 1];
static SaveData game = {SAVE_MAGIC, 1, 1, 1, 0, 0, 0, 0};
static bool storage_ok;

static uint32_t checksum(const SaveData *s) {
    const uint8_t *p = (const uint8_t *)s;
    uint32_t h = 2166136261u;
    for (unsigned i = 0; i < offsetof(SaveData, checksum); i++)
        h = (h ^ p[i]) * 16777619u;
    return h;
}
static void reset_map(void) {
    for (int y = 0; y < MAP_H; y++) memcpy(map[y], world[y], MAP_W + 1);
    for (unsigned i = 0; i < 3; i++)
        if (game.collected_mask & (1u << i)) map[crystal_y[i]][crystal_x[i]] = '.';
}
static bool save_game(void) {
    if (!storage_ok) return false;
    game.magic = SAVE_MAGIC;
    game.version = 1;
    game.checksum = checksum(&game);
    FILE *f = fopen(SAVE_PATH, "wb");
    if (!f) return false;
    bool ok = fwrite(&game, sizeof(game), 1, f) == 1;
    if (fclose(f) != 0) ok = false;
    return ok;
}
static bool load_game(void) {
    if (!storage_ok) return false;
    SaveData s;
    FILE *f = fopen(SAVE_PATH, "rb");
    if (!f) return false;
    size_t n = fread(&s, 1, sizeof(s), f);
    fclose(f);
    if (n != sizeof(s) || s.magic != SAVE_MAGIC || s.version != 1 ||
        s.checksum != checksum(&s) || s.x >= MAP_W || s.y >= MAP_H ||
        s.crystals > 3 || s.delivered > 1 || (s.collected_mask & ~7u)) return false;
    unsigned bits = !!(s.collected_mask & 1) + !!(s.collected_mask & 2) + !!(s.collected_mask & 4);
    if (bits != s.crystals) return false;
    game = s;
    return true;
}
static void draw(const char *message) {
    consoleClear();
    printf("\x1b[0;0HAETHEROS: STARBOUND COURIER\n");
    printf("==============================\n");
    printf("Mission: collect 3 crystals, reach B.\n");
    printf("Crystals: %u/3    %s\n\n", game.crystals,
           game.delivered ? "DELIVERED!" : "IN FLIGHT");
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++)
            putchar(x == game.x && y == game.y ? '@' : map[y][x]);
        putchar('\n');
    }
    printf("\nD-pad: move  A: collect/deliver\nB: save  START: save & exit\n");
    printf("Legend @ you  C crystal  B beacon\n");
    if (message) printf("\n%s", message);
}
int main(void) {
    consoleDemoInit();
    storage_ok = fatInitDefault();
    bool loaded = load_game();
    reset_map();
    char message[96];
    if (loaded) snprintf(message, sizeof(message), "Save loaded; welcome back, courier.");
    else snprintf(message, sizeof(message), "%s. Find the crystals!",
                  storage_ok ? "New voyage" : "SD unavailable; progress is temporary");
    draw(message);
    for (;;) {
        swiWaitForVBlank();
        scanKeys();
        uint32_t k = keysDown();
        if (k & KEY_START) { (void)save_game(); return 0; }
        if (k & KEY_B) {
            bool saved = save_game();
            draw(saved ? "Progress saved to starbound.sav." : "SAVE FAILED: check SD and free space.");
            continue;
        }
        if (k & KEY_A) {
            if (map[game.y][game.x] == 'C') {
                bool collected = false;
                for (unsigned i = 0; i < 3; i++) {
                    if (crystal_x[i] == game.x && crystal_y[i] == game.y &&
                        !(game.collected_mask & (1u << i))) {
                        game.collected_mask |= (uint8_t)(1u << i);
                        game.crystals++;
                        map[game.y][game.x] = '.';
                        bool saved = save_game();
                        if (saved) snprintf(message, sizeof(message), "Crystal secured (%u/3), saved.", game.crystals);
                        else snprintf(message, sizeof(message), "Crystal collected, but SAVE FAILED.");
                        collected = true;
                        break;
                    }
                }
                if (!collected) snprintf(message, sizeof(message), "Crystal already collected.");
            } else if (map[game.y][game.x] == 'B') {
                if (game.crystals == 3) {
                    game.delivered = 1;
                    if (save_game()) snprintf(message, sizeof(message), "MISSION COMPLETE! Delivery saved.");
                    else snprintf(message, sizeof(message), "Delivery complete, but SAVE FAILED.");
                } else snprintf(message, sizeof(message), "Beacon needs 3 crystals. Keep exploring!");
            } else {
                if (save_game()) snprintf(message, sizeof(message), "Ship log saved.");
                else snprintf(message, sizeof(message), "SAVE FAILED: check SD and free space.");
            }
            draw(message);
            continue;
        }
        int nx = game.x, ny = game.y;
        if (k & KEY_UP) ny--;
        else if (k & KEY_DOWN) ny++;
        else if (k & KEY_LEFT) nx--;
        else if (k & KEY_RIGHT) nx++;
        else continue;
        if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || map[ny][nx] == '#') continue;
        game.x = (uint8_t)nx; game.y = (uint8_t)ny;
        if (map[ny][nx] == 'C') snprintf(message, sizeof(message), "Crystal found! Press A to collect.");
        else if (map[ny][nx] == 'B') snprintf(message, sizeof(message), "Delivery beacon. Press A to deliver.");
        else snprintf(message, sizeof(message), "Explore the planet.");
        draw(message);
    }
}
