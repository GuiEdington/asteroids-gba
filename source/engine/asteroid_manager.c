#include "asteroid_manager.h"
#include "../config.h"

static OBJATTR *oam_ref;
static int oam_offset;
static Asteroid asteroid_pool[MAX_ASTEROIDS_POOL];

void get_random_spawn_position(int *out_x, int *out_y, int asteroid_size) {
    int edge =  rand() & 3; // 0=topo, 1=direita, 2=baixo, 3=esquerda
    switch (edge) {
        case 0: // Topo
            *out_x = rand() % SCREEN_WIDTH;
            *out_y = -asteroid_size; // Spawn fora da tela
            break;
        case 1: // Direita            
            *out_x = SCREEN_WIDTH + asteroid_size; // Spawn fora da tela
            *out_y = rand() % SCREEN_HEIGHT;
            break;
        case 2: // Baixo
            *out_x = rand() % SCREEN_WIDTH;
            *out_y = SCREEN_HEIGHT + asteroid_size; // Spawn fora da tela
            break;
        case 3: // Esquerda
            *out_x = -asteroid_size; // Spawn fora da tela
            *out_y = rand() % SCREEN_HEIGHT;
            break;
    }
}

void asteroid_manager_init(OBJATTR *oam_buffer, int oam_start_index) {
    oam_offset = oam_start_index;
    oam_ref = oam_buffer;
    for(int i = 0; i < MAX_ASTEROIDS_POOL; i++) {
        oam_buffer[oam_start_index + i].attr0 = ATTR0_DISABLED;
        asteroid_pool[i].active = false; 
    }
}

void asteroid_manager_spawn(int count) {
    int size = ASTEROID_LARGE; // Sempre spawnamos asteroides grandes, eles vão se dividir quando destruídos
    for (int i = 0; i < count; i++) {
        int spawn_x, spawn_y;
        get_random_spawn_position(&spawn_x, &spawn_y, size);
        asteroid_init(&asteroid_pool[i], spawn_x, spawn_y, size, rand() & 255, &oam_ref[oam_offset + i]);
    }
}

void asteroid_manager_update() {
    for (int i = 0; i < MAX_ASTEROIDS_POOL; i++) {
        if (asteroid_pool[i].active) {
           asteroid_update(&asteroid_pool[i]);
        }
    }
}

void asteroid_manager_draw() {
    for (int i = 0; i < MAX_ASTEROIDS_POOL; i++) {
        if (asteroid_pool[i].active) {
            asteroid_draw(&asteroid_pool[i]);
        }
    }
}

int asteroid_manager_check_hit(int hit_x, int hit_y, int hit_radius) {
    for (int i = 0; i < MAX_ASTEROIDS_POOL; i++) {
        if (!asteroid_pool[i].active) continue;

        Asteroid *a = &asteroid_pool[i];

        // 1. Separa o Visual (para achar o centro) do Físico (para bater)
        int visual_half = 0;
        int hitbox_radius = 0;

        if (a->size == ASTEROID_LARGE) {
            visual_half = 16;       // Sprite é 32x32, metade é 16
            hitbox_radius = 12;     // Colisão real "encolhida" (ajuste a gosto)
        } else if (a->size == ASTEROID_MEDIUM) {
            visual_half = 8;        // Sprite é 16x16, metade é 8
            hitbox_radius = 5;      // Colisão real "encolhida"
        } else if (a->size == ASTEROID_SMALL) {
            visual_half = 4;        // Sprite é 8x8, metade é 4
            hitbox_radius = 2;      // Colisão bem no miolinho
        }

        // 2. Pega as posições brutas
        int ast_px = a->x >> FLOAT_SHIFT;
        int ast_py = a->y >> FLOAT_SHIFT;
        int hit_px = hit_x >> FLOAT_SHIFT;
        int hit_py = hit_y >> FLOAT_SHIFT;

        // 3. Centraliza a matemática usando o VISUAL
        ast_px += visual_half;
        ast_py += visual_half;
        
        // Assumindo que a bala passada no parâmetro também precisa ser centralizada
        // (Se a sua bala é 8x8, o hit_radius original que você passava era 4)
        hit_px += hit_radius; 
        hit_py += hit_radius; 

        // 4. Calcula a distância entre os centros
        int dx = ast_px - hit_px;
        int dy = ast_py - hit_py;
        int distance_squared = (dx * dx) + (dy * dy);

        // 5. A BATIDA: Usa as hitboxes ENCOLHIDAS
        // Reduza também a hitbox da bala para 1 ou 2, para o tiro ficar mais "afiado"
        int bullet_hitbox = 2; 
        int radius_sum = hitbox_radius + bullet_hitbox;
        int radius_squared = radius_sum * radius_sum;

        if (distance_squared < radius_squared) {
            return i; 
        }
    }
    return -1;
}

int get_next_free_index() {
    for (int i = 0; i < MAX_ASTEROIDS_POOL; i++) {
        if (!asteroid_pool[i].active) {
            return i;
        }
    }
    return -1; // Nenhum slot livre
}

int destroy_asteroid(int index) {
    if (index < 0 || index >= MAX_ASTEROIDS_POOL) return 0; // Segurança

    Asteroid *a = &asteroid_pool[index];
    if (!a->active) return 0; // Já está inativo
    int free_index = get_next_free_index();
    asteroid_destroy(a); // Chama a função de destruição da entidade (que pode ser usada para efeitos, sons, etc)

    // Se não for um asteroide pequeno, spawn dos filhos
    if (a->size == ASTEROID_LARGE) {
        if (free_index != -1) {
            asteroid_init(&asteroid_pool[index], a->x >> FLOAT_SHIFT, a->y >> FLOAT_SHIFT, ASTEROID_MEDIUM, rand() & 255, &oam_ref[oam_offset + index]);
            asteroid_init(&asteroid_pool[free_index], a->x >> FLOAT_SHIFT, a->y >> FLOAT_SHIFT, ASTEROID_MEDIUM, rand() & 255, &oam_ref[oam_offset + free_index]);
        }
    } else if (a->size == ASTEROID_MEDIUM) {
        if (free_index != -1) {
            asteroid_init(&asteroid_pool[index], a->x >> FLOAT_SHIFT, a->y >> FLOAT_SHIFT, ASTEROID_SMALL, rand() & 255, &oam_ref[oam_offset + index]);
            asteroid_init(&asteroid_pool[free_index], a->x >> FLOAT_SHIFT, a->y >> FLOAT_SHIFT, ASTEROID_SMALL, rand() & 255, &oam_ref[oam_offset + free_index]);
        }
    }
    return a->size;
}