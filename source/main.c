#include <gba.h>
#include <stdbool.h>
#include <stdlib.h>

// Import dos sprites
#include "spaceship.h" 
#include "space_bg.h"  
#include "asteroids_g.h" 
#include "asteroids_m.h"
#include "asteroids_p.h"

// Defines
#include "config.h"
#include "entities/player.h"
#include "screens/scene.h"
#include "engine/state_manager.h"
#include "screens/level_game.c"

// Desenhando uma "esfera" de 8x8 pixels na mão
const u8 bullet_gfx[64] __attribute__ ((section(".iwram"), aligned(4))) = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0, 255, 255,   0,   0,   0,
    0,   0,   0, 255, 255,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

#define SPRITE_TILE(pos) (SPRITE_GFX + ((pos) * 16))

extern const Scene level_game; // Declaramos a cena para o State Manager
extern OBJATTR shadow_oam[128];

void loadSpritesInVram() {
	// Método para carregar os gráficos e paletas dos sprites na memória de vídeo (VRAM) do GBA.
	dmaCopy(spaceshipPal, SPRITE_PALETTE, spaceshipPalLen);
    dmaCopy(asteroids_gPal, SPRITE_PALETTE + PAL_SIZE, asteroids_gPalLen);
	dmaCopy(spaceshipTiles, SPRITE_GFX, spaceshipTilesLen);

    // 2. Carrega os gráficos (Tiles) de cada tamanho para seus respectivos lugares na VRAM
    // (Lembrando daquela matemática de offsets que fizemos antes)
    dmaCopy(asteroids_gTiles, SPRITE_TILE(AST_G_TILE_POS), asteroids_gTilesLen);
    dmaCopy(asteroids_mTiles, SPRITE_TILE(AST_M_TILE_POS), asteroids_mTilesLen);
    dmaCopy(asteroids_pTiles, SPRITE_TILE(AST_P_TILE_POS), asteroids_pTilesLen);
}

void loadBackground() {
    // 1. Configura o Maestro do Background 0 (REG_BG0CNT)
    // - BG_256_COLOR: Usa paleta de 8-bits.
    // - CHAR_BASE(0): Diz à GPU "Pegue os gráficos no Bloco de Memória 0".
    // - MAP_BASE(28): Diz à GPU "A planilha do mapa está guardada no Bloco 28".
    // - 3 (Prioridade): Garante que o fundo fique ATRÁS da nave e dos tiros (prioridades vão de 0 a 3, sendo 3 o mais fundo).
    REG_BG0CNT = BG_256_COLOR | CHAR_BASE(0) | MAP_BASE(28) | 3;

    // 2. Copia a Paleta
    // BG_PALETTE é diferente do SPRITE_PALETTE! Eles vivem em espaços separados.
    dmaCopy(space_bgPal, BG_PALETTE, space_bgPalLen);

    // 3. Copia os Tiles (Gráficos) para o Char Block 0 (Endereço Físico Base: 0x06000000)
    dmaCopy(space_bgTiles, (void*)0x06000000, space_bgTilesLen);

    // 4. Copia o Mapa para o Screen Block 28
    // Como cada Screen Block tem 2048 bytes (2KB), multiplicamos 28 por 2048.
    dmaCopy(space_bgMap, (void*)(0x06000000 + (28 * 2048)), space_bgMapLen);
}

void initialDisplayConfig() {
	// --- CONFIGURAÇÃO DE HARDWARE OBRIGATÓRIA ---
    
    // irqInit(): Inicializa o sistema de interrupções (IRQs) da biblioteca libgba.
    // Ele limpa e prepara a tabela interna na memória RAM onde a CPU vai procurar 
    // instruções quando um hardware (como o monitor ou os botões) pedir atenção.
    irqInit();

    // REG_DISPSTAT: Display Status Register (Comunicação com o Monitor).
    // O operador |= liga o bit VBLANK_IRQ_ENABLE sem desligar outras configurações.
    // Isso autoriza fisicamente o chip do LCD a disparar um sinal elétrico de alerta 
    // no exato milissegundo em que terminar de desenhar a tela (Vertical Blank).
    REG_DISPSTAT |= VBLANK_IRQ_ENABLE; 

    // irqEnable(): Configuração da CPU principal (ARM7TDMI).
    // Diz ao processador para "abrir os ouvidos" e aceitar especificamente a 
    // interrupção IRQ_VBLANK. Sem isso, o monitor emite o sinal, mas a CPU ignora.
    irqEnable(IRQ_VBLANK);


    // --- CONFIGURAÇÃO DO DISPLAY E MEMÓRIA DE VÍDEO ---
    // REG_DISPCNT: Display Control Register (O maestro do chip de vídeo).
    // MODE_0     -> Define a arquitetura da tela para renderizar blocos 2D clássicos (Tiles).
    // OBJ_ENABLE -> Liga o motor de renderização da OAM. Sem isso, os sprites ficam invisíveis.
    // OBJ_1D_MAP -> Mapeamento 1D. Diz ao hardware para ler a memória VRAM de sprites como 
    //               uma "fita" contínua e linear, o que permite copiarmos arrays 
    //               diretamente do C usando dmaCopy sem nos preocupar com matrizes 2D.
    REG_DISPCNT = MODE_0 | OBJ_ENABLE | OBJ_1D_MAP | BG0_ENABLE;
}

void createProgrammaticBullet() {
    // 1. Configura a Cor: O índice 255 da paleta vai ser Branco Puro.
    SPRITE_PALETTE[255] = RGB5(31, 31, 31);

    // Usamos ponteiros de 16 bits (u16*) para forçar a CPU a usar o barramento correto
    u16* vram_obj16 = (u16*)SPRITE_GFX;
    
    // O pulo do gato: 32 blocos * 16 (meias-palavras de 16 bits) = 1024 bytes de offset
    u16* destination16 = vram_obj16 + (32 * 16);

    // 3. Forja o gráfico direto na Placa de Vídeo em pacotes de 16 bits
    // Como agrupamos de 2 em 2, o laço só roda 32 vezes (para os 64 pixels)
    for (int i = 0; i < 32; i++) {
        // Lemos 2 pixels adjacentes do nosso array de 8 bits
        u16 pixel1 = bullet_gfx[i * 2];       // Pixel da esquerda
        u16 pixel2 = bullet_gfx[i * 2 + 1];   // Pixel da direita
        
        // Empacotamos os dois em uma única variável de 16 bits
        // Como o GBA é Little-Endian (lê de trás pra frente), o pixel da esquerda
        // fica nos 8 bits mais baixos, e o da direita é deslocado (<< 8) para os bits mais altos.
        destination16[i] = pixel1 | (pixel2 << 8);
    }
}

int main(void) {
    srand(REG_TM0CNT);

	initialDisplayConfig();
    
	loadSpritesInVram();

	loadBackground();

	createProgrammaticBullet();

    set_scene(&level_game); // Diz ao State Manager para carregar a fase do jogo

    while (1) {
		sm_update();        
        sm_draw();
		
        VBlankIntrWait();

		dmaCopy(shadow_oam, OAM, 128 * sizeof(OBJATTR));
    }
}