#include <gba.h> // Ou a sua biblioteca de headers (tonc.h, etc)
#include <stdlib.h> // Necessário para o srand()
#include "../engine/state_manager.h"
#include "../engine/text_engine.h"
#include "level_game.c" // A próxima cena para onde vamos transitar

// Os headers gerados pelo Grit com os seus gráficos
#include "space_background.h"
#include "asteroids_title.h" // Inclua o novo arquivo gerado pelo Grit
// #include "title_sprites.h" // Onde estaria a sua logo e o "Press START"

// Declaramos a próxima cena para podermos fazer a transição
extern const Scene level_game;

int title_bg_scroll_x = 0;
static u32 title_frame_counter = 0;

// Adicione isso no topo do arquivo para converter o bloco na posição exata dos bits
#define BG_CBB(n) ((n) << 2)
#define BG_SBB(n) ((n) << 8)
#define BG2_TARGET (1 << 2)
#define ALPHA_BLENDING_EFFECT (1 << 6)
#define BLEND_BG0 (1 << 8)
#define BLEND_BG1 (1 << 9)
#define BLEND_BG3 (1 << 11)
#define BLEND_BD (1 << 13)
#define FADE_RATE 127
#define MAX_OPACITY 16


static void title_init() {
    title_frame_counter = 0;
    title_bg_scroll_x = 0;
    text_init(); 

    // ==========================================
    // 1. CARREGA O ESPAÇO (Fundo)
    // ==========================================
    // Copia a Paleta para o Banco 0
    dmaCopy(space_backgroundPal, BG_PALETTE, space_backgroundPalLen);
    
    // CBB 0: Onde moram os gráficos do espaço
    dmaCopy(space_backgroundTiles, CHAR_BASE_BLOCK(0), space_backgroundTilesLen);
    // SBB 31: Onde mora o mapa do espaço
    dmaCopy(space_backgroundMap, SCREEN_BASE_BLOCK(31), space_backgroundMapLen);

    // ==========================================
    // 2. CARREGA O TÍTULO (Sobreposição)
    // ==========================================
    // Copia a Paleta para o Banco 1 (Somamos 16 cores para não esmagar a do espaço)
    dmaCopy(asteroids_titlePal, (BG_PALETTE + 16), asteroids_titlePalLen);
    
    // CBB 2: Gaveta separada para não esmagar os gráficos do espaço!
    dmaCopy(asteroids_titleTiles, CHAR_BASE_BLOCK(2), asteroids_titleTilesLen);
    // SBB 30: Gaveta separada para o mapa do título!
    dmaCopy(asteroids_titleMap, SCREEN_BASE_BLOCK(30), asteroids_titleMapLen);


    // ==========================================
    // 3. CONFIGURA OS REGISTRADORES DAS CAMADAS
    // ==========================================
    // BG0: Espaço (Usa Paleta 0, Prioridade 1)
    REG_BG0CNT = BG_CBB(0) | BG_SBB(31) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(3);

    // BG1: Título (Usa Paleta 1, Prioridade 0 -> Desenha por cima do BG0!)
    // Não precisamos de BG_PRIORITY(0) porque 0 é o padrão.
    REG_BG1CNT = BG_CBB(2) | BG_SBB(30) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(2);
    
    text_draw_static(5, 18, (unsigned char*)"\xA9 2026 Edington Tech");
    text_draw(10, 13, (unsigned char*)"Press START");

    REG_BLDCNT = BG2_TARGET | ALPHA_BLENDING_EFFECT | BLEND_BG0 | BLEND_BG1 | BLEND_BG3 | BLEND_BD;

    // Liga a Tela, liga o Background 0 e liga o Background 1
    REG_DISPCNT = MODE_0 | BG0_ON | BG1_ON | OBJ_ON | OBJ_1D_MAP | BG2_ON | BG3_ON;

    // Centraliza a câmera do Título (BG1) na tela do GBA
    REG_BG1HOFS = 8;
    REG_BG1VOFS = 0;

}

// -------------------------------------------------------------
// 2. ATUALIZAÇÃO LÓGICA (Física e Input)
// -------------------------------------------------------------
static void title_update() {
    title_frame_counter++;

    // A. Animação do Espaço Infinito
    // Desloca a câmera 1 pixel para a esquerda a cada frame.
    if ((title_frame_counter & 3) == 0) { // Desloca a cada 4 frames (1/15 de segundo)
        title_bg_scroll_x = (title_bg_scroll_x - 1) & 0xFF; 
    }

    // B. Leitura dos Botões
    scanKeys(); // Atualiza o estado do hardware de botões
    u16 keys = keysDown(); // Pega apenas os botões que foram pressionados NESTE frame

    // C. A Transição
    if (keys & KEY_START) {
        // Usa o tempo exato que o jogador demorou na tela como "Semente" 
        // para garantir que a fase sempre nasça diferente!
        srand(title_frame_counter);

        // Troca a fita do jogo! O State Manager assume o controle no próximo frame.
        set_scene(&level_game); 
    }
}

// -------------------------------------------------------------
// 3. RENDERIZAÇÃO (Piscar e Atualizar a OAM)
// -------------------------------------------------------------
static void title_draw() {
    REG_BG0HOFS = title_bg_scroll_x;
    // A. Lógica do Texto Piscante
    int cycle = title_frame_counter & FADE_RATE; 
    int text_opacity; 

    if (cycle < ((FADE_RATE + 1) >> 1)) {
        // Cresce a opacidade do texto nos primeiros 64 frames
        // Dividimos por 2 pois o valor máximo da opacidade é 16, e o ciclo vai de 0 a 127 (FADE_RATE)
        text_opacity = cycle >> 2; 
    } else {
        // Diminui a opacidade do texto nos próximos 64 frames
        text_opacity = (FADE_RATE - cycle) >> 2; 
    }
    int background_opacity = MAX_OPACITY - text_opacity;

    // Configura  o registrador de blend para criar o efeito de fade
    // Esse registrador opera com 16 bits, onde os 8 bits inferiores 
    // são para a opacidade do primeiro alvo (neste caso, o texto) e 
    // os 8 bits superiores são para o segundo alvo (o fundo). Esses
    // bits  esperam  valores  de  0  a  16,  onde  0  é  totalmente 
    // transparente e 16 é totalmente opaco.
    // Texto Opaco (16)     :  00000000 00010000
    // Fundo Opaco (16 << 8):  00010000 00000000
    //                        -----------------
    // Resultado do OR ( | ):  00010000 00010000
    // por isso empurramos o valor do background_opacity 8 bits para 
    // a esquerda.
    REG_BLDALPHA = text_opacity | (background_opacity << 8);
}

// -------------------------------------------------------------
// 4. EXPORTAÇÃO DA CENA
// -------------------------------------------------------------
// Este é o único elemento público deste arquivo. 
// O main.c vai pegar essa struct e colocar dentro do set_scene()
const Scene title_screen = {
    .init = title_init,
    .update = title_update,
    .draw = title_draw,
    .exit = NULL // Como configuramos o REG_DISPCNT = 0 no set_scene, não precisamos de um exit manual aqui.
};