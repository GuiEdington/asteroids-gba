#include <gba.h>
#include "scene.h"
#include "../entities/player.h"
#include "../engine/bullet_manager.h"
#include "../engine/asteroid_manager.h"
#include "../engine/background_manager.h"
#include "../engine/hud.h"
#include "../engine/physics.h"

// Sprites
#include "spaceship.h"
#include "asteroids_g.h"
#include "asteroids_m.h"
#include "asteroids_p.h"
#include "explosion.h"

// ==========================================
// ESTADO LOCAL DA FASE (Variáveis Static)
// ==========================================
static Player player;
static int player_score = 0;
static int player_lives = 3;

// Supondo que você já tenha essas structs definidas em algum lugar

// O seu buffer local da OAM para não escrever direto na VRAM fora do VBlank
static OBJATTR shadow_oam[128]; 

#define SPRITE_TILE(pos) (SPRITE_GFX + ((pos) * 16))

void loadSpritesInVram() {
	// Método para carregar os gráficos e paletas dos sprites na memória de vídeo (VRAM) do GBA.
	dmaCopy(spaceshipPal, SPRITE_PALETTE, spaceshipPalLen);
    dmaCopy(asteroids_gPal, SPRITE_PALETTE + PAL_SIZE, asteroids_gPalLen);
    dmaCopy(explosionPal, SPRITE_PALETTE + 2 * PAL_SIZE, explosionPalLen);

    // Carregar o gráfico dos sprites
	dmaCopy(spaceshipTiles, SPRITE_GFX, spaceshipTilesLen);
    dmaCopy(asteroids_gTiles, SPRITE_TILE(AST_G_TILE_POS), asteroids_gTilesLen);
    dmaCopy(asteroids_mTiles, SPRITE_TILE(AST_M_TILE_POS), asteroids_mTilesLen);
    dmaCopy(asteroids_pTiles, SPRITE_TILE(AST_P_TILE_POS), asteroids_pTilesLen);
    dmaCopy(explosionTiles, SPRITE_TILE(EXPLOSION_TILE_POS), explosionTilesLen);
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

void level_game_resolve_collisions() {
    int ast_cx, ast_cy, ast_radius;
    int bul_cx, bul_cy, bul_radius;

    // Loop nas balas
    for (int b = 0; b < MAX_BULLETS; b++) {
        // Pega a bolha de colisão da bala
        if (!bullet_manager_get_collider(b, &bul_cx, &bul_cy, &bul_radius)) continue;

        // Loop nos asteroides
        for (int a = 0; a < MAX_ASTEROIDS_POOL; a++) {
            // Pega a bolha de colisão do asteroide
            if (!asteroid_manager_get_collider(a, &ast_cx, &ast_cy, &ast_radius)) continue;

            // Manda para a matemática pura!
            if (check_circle_collision(bul_cx, bul_cy, bul_radius, ast_cx, ast_cy, ast_radius)) {
                
                // BATEU! Executa as regras de negócio:
                destroy_bullet(b);
                int asteroidSize = get_size(a);
                destroy_asteroid(a);
                if (asteroidSize > 0) {
                    if (asteroidSize == ASTEROID_LARGE) {
                        player_score += 20;
                    } else if (asteroidSize == ASTEROID_MEDIUM) {
                        player_score += 50;
                    } else if (asteroidSize == ASTEROID_SMALL) {
                        player_score += 100;
                    }
                    update_score(player_score);
                    if (player_score >= 10000) {
                        player_lives++;
                        update_lives(player_lives);
                    }
                }
                break; // A bala já sumiu, não precisa testar com outros asteroides
            }
        }
    }

    if (player.state == STATE_ALIVE && player.invuln_timer == 0) {
        // Verifica colisão do jogador com asteroides
        for (int a = 0; a < MAX_ASTEROIDS_POOL; a++) {
            if (!asteroid_manager_get_collider(a, &ast_cx, &ast_cy, &ast_radius)) continue;
            int player_cx = (player.x >> FLOAT_SHIFT) + (PLAYER_SIZE >> 1);
            int player_cy = (player.y >> FLOAT_SHIFT) + (PLAYER_SIZE >> 1);
            int player_radius = (PLAYER_SIZE >> 1) - 1; // Colisão "encolhida"

            if (check_circle_collision(player_cx, player_cy, player_radius, ast_cx, ast_cy, ast_radius)) {
                // Colidiu com um asteroide!
                player_lives--;
                update_lives(player_lives);
                player.state = STATE_EXPLODING;
                break; // Não precisa testar com outros asteroides
            }
        }
    }
}

// ==========================================
// INIT: Chamado uma vez quando a fase carrega
// ==========================================
void game_init() {
    // Ativa Modo 0, Background do Espaço e Sprites!
    REG_BLDCNT = 0; 
    REG_BLDY = 0;
    REG_DISPCNT = MODE_0 | BG0_ON | BG2_ON | OBJ_ON | OBJ_1D_MAP;

    // 1. Limpa a shadow_oam
    for(int i = 0; i < 128; i++) {
        shadow_oam[i].attr0 = ATTR0_DISABLED;
    }

    loadSpritesInVram();
    createProgrammaticBulletSprite();

    OBJAFFINE *shadow_affine = (OBJAFFINE*)shadow_oam; // Reinterpretamos o início da shadow_oam como uma área de OBJAFFINE para o player
    // 2. Inicializa entidades
    hud_init();
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
    level_game_resolve_collisions();
    scanKeys();
    u16 keys = keysHeld();
    u16 keys_pressed = keysDown();
    player_update(&player, keys);
    if (keys_pressed & KEY_A) {
        bullet_manager_spawn(player.x, player.y, player.angle);
    }
    bullet_manager_update();
    asteroid_manager_update();
    bg_manager_update_opose_ship(player.dx, player.dy);
    hud_update();
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