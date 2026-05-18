#include <gba.h> // Ou a sua biblioteca de headers (tonc.h, etc)
#include <stdlib.h> // Necessário para o srand()
#include "../engine/state_manager.h"
#include "../engine/text_engine.h"
#include "../engine/background_manager.h" // Gerencia o fundo do espaço (scroll infinito)
#include "scene.h"

// Os headers gerados pelo Grit com os seus gráficos
#include "space_background.h"
#include "asteroids_title.h" // Inclua o novo arquivo gerado pelo Grit

// Declaramos a próxima cena para podermos fazer a transição
extern const Scene level_game;

int title_bg_scroll_x;
u32 title_frame_counter;
int title_is_fading_out;
int title_is_fading_in;
int title_fade_level;
int text_opacity;
int background_opacity; 

// Defines
#define BG_CBB(n) ((n) << 2)
#define BG_SBB(n) ((n) << 8)
#define BG2_TARGET (1 << 2)            // 00000000 00000000 00000000 00000100
#define ALPHA_BLENDING_EFFECT (1 << 6) // 00000000 00000000 00000000 00100000
#define BLEND_BG0 (1 << 8)             // 00000000 00000000 00000001 00000000
#define BLEND_BG1 (1 << 9)             // 00000000 00000000 00000010 00000000
#define BLEND_BG3 (1 << 11)            // 00000000 00000000 00001000 00000000
#define BLEND_BD (1 << 13)             // 00000000 00000000 00100000 00000000
#define FADE_RATE 127
#define MAX_OPACITY 16
#define MAX_FADE 16
#define BG_WIDTH_MASK 0xFF // 255


static void title_init() {
    title_frame_counter = 0;
    title_bg_scroll_x = 0;
    title_bg_scroll_x = 0;
    title_frame_counter = 0;
    title_is_fading_out = 0;
    title_is_fading_in = 1;
    title_fade_level = MAX_FADE;
    // Inicializa o sistema de texto para podermos desenhar o "Press START" e o copyright
    text_init(1); // Passa 1 para configurar o BG3 para o "Press START", já que isso é só para a tela de título

    // Inicializa o gerenciador de fundo (BG0)
    bg_manager_init();

    dmaCopy(asteroids_titlePal, (BG_PALETTE + 16), asteroids_titlePalLen);
    
    // CBB 2: Gaveta separada para não esmagar os gráficos do espaço!
    dmaCopy(asteroids_titleTiles, CHAR_BASE_BLOCK(2), asteroids_titleTilesLen);
    // SBB 30: Gaveta separada para o mapa do título!
    dmaCopy(asteroids_titleMap, SCREEN_BASE_BLOCK(30), asteroids_titleMapLen);


    // ==========================================
    // 3. CONFIGURA OS REGISTRADORES DAS CAMADAS
    // ==========================================
    REG_BG1CNT = BG_CBB(2) | BG_SBB(30) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(2);
    
    text_draw_static(5, 18, (unsigned char*)"\xA9 2026 Edington Tech");
    text_draw(10, 13, (unsigned char*)"Press START");

    REG_BLDCNT = ALL_BG | BRIGHTNESS_DECREASE_MODE;

    REG_BLDY = title_fade_level;

    // Liga a Tela, liga o Background 0 e liga os Backgrounds 1, 2 e 3 para o efeito de blend 
    REG_DISPCNT = MODE_0 | BG0_ON | BG1_ON | BG2_ON | BG3_ON;

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
        title_bg_scroll_x = (title_bg_scroll_x - 1) & BG_WIDTH_MASK; 
    }

    if (title_is_fading_out) {
        if ((title_frame_counter & 3) == 0) { // Desloca a cada 4 frames (1/15 de segundo)
            title_fade_level++;
            REG_BLDY = title_fade_level; // Aumenta a intensidade do fade a cada 4 frames
            if (title_fade_level >= MAX_FADE) {
                // Quando o fade estiver completo, desligamos a tela para evitar artefatos visuais
                REG_DISPCNT = 0;
                title_is_fading_out = 0;
                set_scene(&level_game); 
            }
        }
        return;
    }

    if (title_is_fading_in) {
        if ((title_frame_counter & 3) == 0) { // Desloca a cada 4 frames (1/15 de segundo)
            title_fade_level--;
            REG_BLDY = title_fade_level; // Aumenta a intensidade do fade a cada 4 frames
            if (title_fade_level <= 0) {
                title_is_fading_in = 0;
                title_fade_level = 0;
                REG_BLDY = 0; // Desliga o efeito de fade para liberar a
                REG_BLDCNT = BG2_TARGET | ALPHA_BLENDING_EFFECT | BLEND_BG0 | BLEND_BG1 | BLEND_BG3 | BLEND_BD;
            }
        }
        return;
    }

    int cycle = title_frame_counter & FADE_RATE; 
    if (cycle < ((FADE_RATE + 1) >> 1)) {
        // Cresce a opacidade do texto nos primeiros 64 frames
        // Dividimos por 2 pois o valor máximo da opacidade é 16, e o ciclo vai de 0 a 127 (FADE_RATE)
        text_opacity = cycle >> 2; 
    } else {
        // Diminui a opacidade do texto nos próximos 64 frames
        text_opacity = (FADE_RATE - cycle) >> 2; 
    }
    background_opacity = MAX_OPACITY - text_opacity;

    // B. Leitura dos Botões
    scanKeys(); // Atualiza o estado do hardware de botões
    u16 keys = keysDown(); // Pega apenas os botões que foram pressionados NESTE frame

    // C. A Transição
    if (keys & KEY_START) {
        title_is_fading_out = 1;
        title_fade_level = 0;
        REG_BLDCNT = ALL_BG | BRIGHTNESS_DECREASE_MODE;
        // Usa o tempo exato que o jogador demorou na tela como "Semente" 
        // para garantir que a fase sempre nasça diferente!
        srand(title_frame_counter);
    }
    return;
}

// -------------------------------------------------------------
// 3. RENDERIZAÇÃO (Piscar e Atualizar a OAM)
// -------------------------------------------------------------
static void title_draw() {
    bg_manager_update(title_bg_scroll_x, 0); // Atualiza o scroll do fundo

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

static void title_exit() {
    // 1. Desativa a flag de fade para que, ao voltar ao título, ele não pule sozinho
    title_is_fading_out = 0;
    title_fade_level = 0;
    title_frame_counter = 0;
    title_bg_scroll_x = 0;

    // 2. Reseta os registradores de efeitos especiais de cor
    // Isso evita que o efeito de transparência do título afete os sprites do jogo
    REG_BLDCNT = 0;
    REG_BLDALPHA = 0;
    REG_BLDY = 0;
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
    .exit = title_exit // Agora usamos a função title_exit para limpar corretamente os registradores e flags.
};