#include "scene.h"
#include <gba.h>
#include "../config.h"

#include "../engine/text_engine.h"
#include "../engine/state_manager.h"

#define MAX_FADE 16
// Bit 2 (BG2) + Modo Brightness Decrease (0x0040)
// Isso equivale exatamente a ativar o efeito de escurecer no BG2
#define GAME_OVER_BLD_CFG (0x0040 | (1 << 2))

extern const Scene title_screen;

int game_over_is_fading;
int game_over_is_fading_in;
int game_over_is_fading_out;
int game_over_frame_counter;
int game_over_fade_level = MAX_FADE;
int game_over_update_fade = 0;

static void game_over_init() {
    // 1. Ativa o BG2 no hardware
    REG_DISPCNT = MODE_0 | BG2_ON;
    
    // Limpa os sprites do gameplay antigo mandando-os para fora da tela
    // for (int i = 0; i < 128; i++) {
    //     OAM_MEM[i].attr0 = (OAM_MEM[i].attr0 & 0xFF00) | 160;
    // }

    // Inicializa a engine no BG2
    text_init(0); 
    
    game_over_is_fading = 1;
    game_over_is_fading_in = 1;
    game_over_is_fading_out = 0;
    game_over_frame_counter = 0;
    game_over_fade_level = MAX_FADE;
    game_over_update_fade = 1; 

    // 2. CONFIGURAÇÃO PURA DE HARDWARE (Substituindo as macros ausentes):
    // 0x0040 ativa o modo de decréscimo de brilho.
    // (1 << 2) seleciona o BG2 como o alvo a ser escurecido.
    REG_BLDCNT = BRIGHTNESS_DECREASE_MODE | ALL_BG;
    REG_BLDY = MAX_FADE; // Começa totalmente escuro

    BG_PALETTE[0] = RGB5(0, 0, 0);
    
    // Escreve o texto no mapa do BG2
    text_draw(11, 9, (unsigned char*)"GAME OVER");
}

static void game_over_update() {
    scanKeys(); 
    u16 keys = keysDown(); 

    game_over_frame_counter++;

    if (game_over_is_fading) {
        if ((game_over_frame_counter & 3) == 0) {
            game_over_update_fade = 1; 

            if (game_over_is_fading_in)  game_over_fade_level--;
            if (game_over_is_fading_out) game_over_fade_level++;

            // Fim do Fade In (Tela clareou 100%)
            if (game_over_fade_level <= 0 && game_over_is_fading_in) {
                game_over_is_fading_in = 0;
                game_over_is_fading = 0;
                game_over_fade_level = 0;
                
                // Desliga completamente os efeitos para liberar a PPU e estabilizar o FPS
                REG_BLDCNT = 0; 
                REG_BLDY = 0;
                
                scanKeys();
                keysDown();
            }
            
            // Fim do Fade Out (Tela escureceu 100%)
            if (game_over_fade_level >= MAX_FADE && game_over_is_fading_out) {
                game_over_is_fading_out = 0;
                game_over_is_fading = 0;
                
                REG_BLDCNT = 0; 
                REG_BLDY = 0;
                
                set_scene(&title_screen); 
                return;
            }
        } else {
            game_over_update_fade = 0;
        }
        return; 
    }

    if (keys & KEY_START) {
        game_over_is_fading_out = 1;
        game_over_is_fading = 1;
        game_over_fade_level = 0;
        
        // Reativa a configuração de decréscimo de brilho focada no BG2 para o Fade Out
        REG_BLDCNT = BRIGHTNESS_DECREASE_MODE | ALL_BG;
    }
}

static void game_over_draw() {
    if (game_over_update_fade) {
        REG_BLDY = game_over_fade_level;
        game_over_update_fade = 0; 
    }
}

const Scene game_over = {
    game_over_init,
    game_over_update,
    game_over_draw
};