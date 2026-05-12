#include <gba.h>
#include "scene.h"
#include "../entities/player.h"
#include "../engine/bullet_manager.h"
#include "../engine/asteroid_manager.h"
#include "../engine/background_manager.h"

// Sprites
#include "spaceship.h"
#include "asteroids_g.h"
#include "asteroids_m.h"
#include "asteroids_p.h"

// ==========================================
// ESTADO LOCAL DA FASE (Variáveis Static)
// ==========================================
static Player player;

// Supondo que você já tenha essas structs definidas em algum lugar

// O seu buffer local da OAM para não escrever direto na VRAM fora do VBlank
static OBJATTR shadow_oam[128]; 

#define SPRITE_TILE(pos) (SPRITE_GFX + ((pos) * 16))

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

void createProgrammaticBulletSprite() {
    // 1. Configura a Cor: O índice 255 da paleta vai ser Branco Puro.
    SPRITE_PALETTE[255] = RGB5(31, 31, 31);
    
    // O pulo do gato: 32 blocos * 16 (meias-palavras de 16 bits) = 1024 bytes de offset
    u16* destination16 = SPRITE_TILE(BULLET_TILE_POS);

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

// ==========================================
// INIT: Chamado uma vez quando a fase carrega
// ==========================================
void game_init() {
    // Ativa Modo 0, Background do Espaço e Sprites!
    REG_DISPCNT = MODE_0 | BG0_ON | OBJ_ON | OBJ_1D_MAP;

    // 1. Limpa a shadow_oam
    for(int i = 0; i < 128; i++) {
        shadow_oam[i].attr0 = ATTR0_DISABLED;
    }

    loadSpritesInVram();
    createProgrammaticBulletSprite();

    OBJAFFINE *shadow_affine = (OBJAFFINE*)shadow_oam; // Reinterpretamos o início da shadow_oam como uma área de OBJAFFINE para o player
    // 2. Inicializa entidades
    bg_manager_init();
    player_init(&player, &shadow_oam[0], &shadow_affine[0], 0); // Player fica no índice 0
    bullet_manager_init(&shadow_oam[1], 1); // Tiros começam no índice 1
    asteroid_manager_init(&shadow_oam[18], 18); // Asteroides começam no índice 18 (1 para player + 16 para tiros)
    asteroid_manager_spawn(6);
}

// ==========================================
// UPDATE: Chamado todo frame (Apenas Matemática e Física)
// ==========================================
void game_update() {
    scanKeys();
    u16 keys = keysHeld();
    u16 keys_pressed = keysDown();
    player_update(&player, keys);
    if (keys_pressed & KEY_A) {
        bullet_manager_spawn(player.x, player.y, player.angle);
    }
    bullet_manager_update();
    asteroid_manager_update();
    bg_manager_update(player.dx, player.dy);
}

// ==========================================
// DRAW: Chamado todo frame (Apenas Prepara a OAM)
// ==========================================
void game_draw() {
    // 1. Prepara OAM do Player
    // Assumindo que você alterou o player_draw para atualizar a shadow_oam passada no init
    player_draw(&player); 
    bullet_manager_draw(); // Isso vai atualizar a shadow_oam dos tiros
    asteroid_manager_draw();
}

// ==========================================
// EXPORTA A CENA
// ==========================================
const Scene level_game = {
    game_init,
    game_update,
    game_draw
};