#include <gba.h>
#include "font.h" // Sprite sheet da fonte
#include "../config.h"

// Apontamos para lugares vazios na VRAM
#define TEXT_CBB 1
#define TEXT_SBB 29
#define STATIC_TEXT_SBB 28

// Ponteiro apontando para a grade do BG2
u16* text_map = (u16*)SCREEN_BASE_BLOCK(TEXT_SBB);
u16* static_text_map = (u16*)SCREEN_BASE_BLOCK(STATIC_TEXT_SBB);

void text_init(int init_press_start) {
    dmaCopy(fontPal, &BG_PALETTE[15 * 16], fontPalLen);
    dmaCopy(fontTiles, CHAR_BASE_BLOCK(TEXT_CBB), fontTilesLen);

    for(int i = 0; i < 1024; i++) {
        text_map[i] = 0;
        static_text_map[i] = 0;
    }
    // Usamos o REG_BG2CNT agora.
    // Prioridade 0 (Fica por cima de tudo: da Logo e do Espaço)
    REG_BG2CNT = BG_CBB(TEXT_CBB) | BG_SBB(TEXT_SBB) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(0);

    if (init_press_start == 1) {
        u16* bg1_map = (u16*)SCREEN_BASE_BLOCK(30);
        for(int i = 500; i < 1024; i++) {
            bg1_map[i] = 0; // Apaga qualquer rastro de texto antigo que ficou sob a logo
        }
        REG_BG3CNT = BG_CBB(TEXT_CBB) | BG_SBB(STATIC_TEXT_SBB) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(1);
    }
}

void text_draw(int grid_x, int grid_y, const unsigned char *text) {
    for(int i = 0; text[i] != '\0'; i++) {
        int map_index = (grid_y * 32) + (grid_x + i);

        // Pega a letra (ex: 65) e AVISA o hardware: "Use a paleta 15!"
        u16 tile_entry = text[i] | (15 << 12);

        text_map[map_index] = tile_entry;
    }
}

void text_draw_static(int grid_x, int grid_y, const unsigned char *text) {
    for(int i = 0; text[i] != '\0'; i++) {
        int map_index = (grid_y * 32) + (grid_x + i);
        static_text_map[map_index] = text[i] | (15 << 12);
    }
}