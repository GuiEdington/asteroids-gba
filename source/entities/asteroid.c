#include "asteroid.h"
#include "../config.h"

void asteroid_init(Asteroid *a, int start_x, int start_y, enum AsteroidSize size, int angle, OBJATTR *attribs) {
    a->x = start_x << FLOAT_SHIFT;
    a->y = start_y << FLOAT_SHIFT;
    int base_speed = (MAX_SPEED >> (ASTEROID_LARGE >> 3)); // Mantém velocidade atual do LARGE
    int speed = base_speed;
    switch (size) {
        case ASTEROID_MEDIUM:
            speed = base_speed + (base_speed >> 2); // +25%
            break;
        case ASTEROID_SMALL:
            speed = base_speed + (base_speed >> 1); // +50%
            break;
        case ASTEROID_LARGE:
        default:
            break;
    }
    a->dx = (GET_SIN(angle) * speed) >> 8;
    a->dy = (-GET_COS(angle) * speed) >> 8;
    a->size = size;
    a->obj = attribs;
    a->active = true;

    // Configura o sprite baseado no tamanho
    a->obj->attr0 = ATTR0_COLOR_16 | ATTR0_SQUARE; 
    int ast_type = rand() & 3;
    switch (size) {
        case ASTEROID_LARGE:
            a->obj->attr1 = ATTR1_SIZE_32;
            a->obj->attr2 = ATTR2_PALETTE(1) | (AST_G_TILE_POS + ast_type * ( ASTEROID_LARGE >> 1));
            break;
        case ASTEROID_MEDIUM:
            a->obj->attr1 = ATTR1_SIZE_16;
            a->obj->attr2 = ATTR2_PALETTE(1) | (AST_M_TILE_POS + ast_type * ASTEROID_MEDIUM);
            break;
        case ASTEROID_SMALL:
            a->obj->attr1 = ATTR1_SIZE_8;
            a->obj->attr2 = ATTR2_PALETTE(1) | (AST_P_TILE_POS + ast_type * ASTEROID_SMALL);
            break;
    }
}

void asteroid_update(Asteroid *a) {
    if (!a->active) return;
    a->x += a->dx;
    a->y += a->dy;

    // Tamanho máximo do sprite
    int margin = (a->size << FLOAT_SHIFT); 
     // 5. Wrap-around da tela (Reaparece do outro lado)
    if (a->x < -margin) a->x = SCREEN_WIDTH << FLOAT_SHIFT;
    if (a->x > SCREEN_WIDTH << FLOAT_SHIFT) a->x = -margin;
    if (a->y < -margin) a->y = SCREEN_HEIGHT << FLOAT_SHIFT;
    if (a->y > SCREEN_HEIGHT << FLOAT_SHIFT) a->y = -margin;
    
}

void asteroid_draw(Asteroid *a) {
    if (a->active) {
        int screen_x = (a->x >> FLOAT_SHIFT) ;
        int screen_y = (a->y >> FLOAT_SHIFT) ;

        a->obj->attr0 = (a->obj->attr0 & ~MASK_Y) | (screen_y & MASK_Y);
        a->obj->attr1 = (a->obj->attr1 & ~MASK_X) | (screen_x & MASK_X);
    }
}

void asteroid_destroy(Asteroid *a) {
    a->active = false;
    a->obj->attr0 = ATTR0_DISABLED;
}