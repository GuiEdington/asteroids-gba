#include <gba.h>
#include "background_manager.h"
#include "space_background.h" // A arte do seu espaço

extern int title_bg_scroll_x; // Variável global para controlar o scroll do fundo (declarada na title_screen.c)

// Variáveis de estado do cenário (Mantidas em Ponto Fixo 8.8!)
static s32 bg_scroll_x = 0; 
static s32 bg_scroll_y = 0; 

void bg_manager_init() {
    // 1. Carrega os gráficos do espaço na VRAM
    dmaCopy(space_backgroundPal, BG_PALETTE, space_backgroundPalLen);
    dmaCopy(space_backgroundTiles, CHAR_BASE_BLOCK(0), space_backgroundTilesLen);
    dmaCopy(space_backgroundMap, SCREEN_BASE_BLOCK(31), space_backgroundMapLen);

    // 2. Configura o registrador (Mesma coisa que estava na tela de título)
    REG_BG0CNT = BG_CBB(0) | BG_SBB(31) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(3);
    
    // 3. Reseta a câmera
    bg_scroll_x = title_bg_scroll_x << 8; // Começa com o scroll do título para uma transição suave
    bg_scroll_y = 0;
}

// Recebe a velocidade atual da nave (em Ponto Fixo 8.8)
void bg_manager_update_opose_ship(s32 ship_dx, s32 ship_dy) {
    
    // 1. O Fator Parallax (>> 3 divide a velocidade por 8)
    // Se a nave vai para a direita (dx positivo), nós SOMAMOS na câmera, 
    // o que empurra a visão para a direita e faz os pixels irem para a esquerda!
    bg_scroll_x += (ship_dx >> 3); 
    bg_scroll_y += (ship_dy >> 3); 

    // 2. Converte do nosso Ponto Fixo 8.8 de volta para Inteiros (Pixels reais)
    int pixel_x = bg_scroll_x >> 8;
    int pixel_y = bg_scroll_y >> 8;

    // 3. Aplica o bitmask de 255 (0xFF) para o Wrap-around perfeito (0 a 255)
    // Isso garante que nunca passemos números negativos ou gigantes para a placa de vídeo
    REG_BG0HOFS = pixel_x & 0xFF;
    REG_BG0VOFS = pixel_y & 0xFF;
}

void bg_manager_update(int pos_x, int pos_y) {
    REG_BG0HOFS = pos_x;
    REG_BG0VOFS = pos_y;
}