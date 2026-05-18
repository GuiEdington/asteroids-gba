#include "bullet_manager.h"
#include "asteroid_manager.h"
#include <stddef.h>
#include "../config.h"

// Estado Privado do Manager (Static garante que ninguém fora daqui acesse)
static Bullet bullets[MAX_BULLETS];
static int next_bullet_index = 0;
static OBJATTR *shadow_oam_ref;
static int oam_offset;

void bullet_manager_init(OBJATTR *oam_buffer, int oam_start_index) {
    shadow_oam_ref = oam_buffer;
    oam_offset = oam_start_index;
    next_bullet_index = 0;

    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
        shadow_oam_ref[oam_offset + i].attr0 = ATTR0_DISABLED; 
    }
}

void bullet_manager_spawn(int start_x, int start_y, int angle) {
    // Calcula qual slot da OAM pertence a este tiro específico
    OBJATTR *bullet_oam = &shadow_oam_ref[oam_offset + next_bullet_index];

    // Chama a inicialização da entidade (aquela que fizemos no passo anterior)
    bullet_init(&bullets[next_bullet_index], start_x, start_y, angle, bullet_oam);

    // O truque de 1 ciclo do Buffer Circular!
    next_bullet_index = (next_bullet_index + 1) & BULLET_MASK;
}

int bullet_manager_update() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullet_update(&bullets[i]); // A física está dentro de bullet.c agora!
            // if (bullets[i].active) {
            //     // O raio da nossa bala 8x8 é 4 pixels
            //     int hit_index = asteroid_manager_get_collider(bullets[i].x, bullets[i].y, 4);

            //     if (hit_index != -1) {
            //         // Destrói a bala
            //         bullets[i].active = false;
            //         shadow_oam_ref[oam_offset + i].attr0 = ATTR0_DISABLED;

            //         // Destrói o asteroide (que vai desencadear o spawn dos filhos)
            //         destroy_bullet(i); // Garante que a bala seja escondida da OAM imediatamente
            //         return destroy_asteroid(hit_index); 
            //     }
            // }
        }
    }
    return 0;
}

void bullet_manager_draw() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullet_draw(&bullets[i]); // O desenho está dentro de bullet.c!
        } else {
            destroy_bullet(i); // Garante que tiros inativos sejam escondidos da OAM
        }
    }
}

int bullet_manager_get_collider(int index, int *cx, int *cy, int *radius) {
    if (index < 0 || index >= MAX_BULLETS) return 0; // Segurança
    if (!bullets[index].active) return 0;

    Bullet *b = &bullets[index];
    *cx = (b->x >> FLOAT_SHIFT) + 4;
    *cy = (b->y >> FLOAT_SHIFT) + 4;
    *radius = 2; // Raio da bala 4x4 é 2 pixels
    return 1; // Colisão ativa
}

int destroy_bullet(int index) {
    if (index < 0 || index >= MAX_BULLETS) return 0; // Segurança
    if (!bullets[index].active) return 0; // Já está inativo

    bullets[index].active = false;
    shadow_oam_ref[oam_offset + index].attr0 = ATTR0_DISABLED; // Esconde da OAM
    return 1;
}
