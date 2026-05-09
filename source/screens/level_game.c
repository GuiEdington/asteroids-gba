#include <gba.h>
#include "scene.h"
#include "../entities/player.h"
#include "../engine/bullet_manager.h"
#include "../engine/asteroid_manager.h"
// No futuro, você incluirá "bullet.h" e "asteroid.h" aqui

// ==========================================
// ESTADO LOCAL DA FASE (Variáveis Static)
// ==========================================
static Player player;

// Supondo que você já tenha essas structs definidas em algum lugar

// O seu buffer local da OAM para não escrever direto na VRAM fora do VBlank
static OBJATTR shadow_oam[128]; 

// ==========================================
// INIT: Chamado uma vez quando a fase carrega
// ==========================================
void game_init() {
    // 1. Limpa a shadow_oam
    for(int i = 0; i < 128; i++) {
        shadow_oam[i].attr0 = ATTR0_DISABLED;
    }

    OBJAFFINE *shadow_affine = (OBJAFFINE*)shadow_oam; // Reinterpretamos o início da shadow_oam como uma área de OBJAFFINE para o player
    // 2. Inicializa entidades
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