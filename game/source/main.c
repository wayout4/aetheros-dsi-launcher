// SPDX-License-Identifier: MIT
// AetherOS: Starbound Courier — playable first prototype for Nintendo DS.
#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAP_W 16
#define MAP_H 10
#define SAVE_PATH "starbound.sav"

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t x, y, crystals, delivered;
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
static char map[MAP_H][MAP_W + 1];
static SaveData game = {0x53424331, 1, 1, 1, 0, 0, 0};
static bool storage_ok;

static uint32_t checksum(const SaveData *s) {
    const uint8_t *p = (const uint8_t *)s;
    uint32_t h = 2166136261u;
    for (unsigned i = 0; i < sizeof(*s) - sizeof(s->checksum); i++)
        h = (h ^ p[i]) * 16777619u;
    return h;
}
static void reset_map(void) {
    for (int y = 0; y < MAP_H; y++) memcpy(map[y], world[y], MAP_W + 1);
}
static void save_game(void) {
    if (!storage_ok) return;
    game.checksum = checksum(&game);
    FILE *f = fopen(SAVE_PATH, "wb");
    if (!f) return;
    fwrite(&game, sizeof(game), 1, f);
    fclose(f);
}
static bool load_game(void) {
    if (!storage_ok) return false;
    SaveData s;
    FILE *f = fopen(SAVE_PATH, "rb");
    if (!f) return false;
    size_t n = fread(&s, 1, sizeof(s), f);
    fclose(f);
    if (n != sizeof(s) || s.magic != 0x53424331 || s.version != 1 ||
        s.checksum != checksum(&s) || s.x >= MAP_W || s.y >= MAP_H ||
        s.crystals > 3 || s.delivered > 1) return false;
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
        for (int x = 0; x < MAP_W; x++) {
            if (x == game.x && y == game.y) putchar('@');
            else if (map[y][x] == 'C' && game.crystals == 3) putchar('.');
            else putchar(map[y][x]);
        }
        putchar('\n');
    }
    printf("\nD-pad: move  A: interact/save\nB: save  START: save & exit\n");
    printf("Legend @ you  C crystal  B beacon\n");
    if (message) printf("\n%s", message);
}
int main(void) {
    consoleDemoInit();
    storage_ok = fatInitDefault();
    reset_map();
    bool loaded = load_game();
    char message[96];
    if (loaded) snprintf(message, sizeof(message), "Save loaded from %s", SAVE_PATH);
    else snprintf(message, sizeof(message), "%s. Find the crystals!",
                  storage_ok ? "New voyage" : "SD unavailable; progress is temporary");
    draw(message);
    for (;;) {
        swiWaitForVBlank();
        scanKeys();
        uint32_t k = keysDown();
        if (k & KEY_START) { save_game(); return 0; }
        if (k & KEY_B) {
            save_game();
            draw(storage_ok ? "Progress saved." : "No SD: save unavailable.");
            continue;
        }
        if (k & KEY_A) {
            if (map[game.y][game.x] == 'C' && game.crystals < 3) {
                game.crystals++;
                map[game.y][game.x] = '.';
                save_game();
                snprintf(message, sizeof(message), "Crystal secured (%u/3).", game.crystals);
            } else if (map[game.y][game.x] == 'B') {
                if (game.crystals >= 3) {
                    game.delivered = 1;
                    save_game();
                    snprintf(message, sizeof(message), "MISSION COMPLETE! Saved. Press START to exit.");
                } else snprintf(message, sizeof(message), "Beacon needs 3 crystals. Keep exploring!");
            } else {
                save_game();
                snprintf(message, sizeof(message), "Ship log saved. Keep moving, courier.");
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
