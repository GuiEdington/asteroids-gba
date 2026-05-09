#include "asteroid_manager.h"

static int max_asteroids;
static int current_asteroid_count;
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
    current_asteroid_count = 0;
    for(int i = 0; i < MAX_ASTEROIDS_POOL; i++) {
        oam_buffer[oam_start_index + i].attr0 = ATTR0_DISABLED;
        asteroid_pool[i].active = false; 
    }
}

void asteroid_manager_spawn(int count) {
    for (int i = 0; i < count; i++) {
        int spawn_x, spawn_y;
        get_random_spawn_position(&spawn_x, &spawn_y, ASTEROID_LARGE);
        asteroid_init(&asteroid_pool[current_asteroid_count], spawn_x, spawn_y, ASTEROID_LARGE, rand(), &oam_ref[oam_offset + current_asteroid_count]);
        current_asteroid_count++;
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