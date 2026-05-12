#include <gba.h> // Ou a sua biblioteca de headers (tonc.h, etc)
#include <stdlib.h> // Necessário para o srand()
#include "../engine/state_manager.h"
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

static void title_init() {
    title_frame_counter = 0;
    title_bg_scroll_x = 0;

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
    REG_BG0CNT = BG_CBB(0) | BG_SBB(31) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(1);

    // BG1: Título (Usa Paleta 1, Prioridade 0 -> Desenha por cima do BG0!)
    // Não precisamos de BG_PRIORITY(0) porque 0 é o padrão.
    REG_BG1CNT = BG_CBB(2) | BG_SBB(30) | BG_16_COLOR | BG_SIZE_0;

    // Liga a Tela, liga o Background 0 e liga o Background 1
    REG_DISPCNT = MODE_0 | BG0_ON | BG1_ON | OBJ_ON | OBJ_1D_MAP;

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
    // 32 em binário cria um ciclo de liga/desliga a cada meio segundo
    if (title_frame_counter & 32) {
        // Remove a flag de desativado (O texto aparece)
        // shadow_oam[indice_do_start].attr0 &= ~ATTR0_DISABLED; 
    } else {
        // Adiciona a flag de desativado (O texto some)
        // shadow_oam[indice_do_start].attr0 |= ATTR0_DISABLED;  
    }

    // B. Copia a Shadow OAM para a OAM real da Placa de Vídeo
    // (A mesma função que você já usa no seu game loop original)
    // copy_oam_to_vram(); 
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