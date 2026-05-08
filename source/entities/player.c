#include "player.h"
#include "../config.h" 

void player_init(Player *p, OBJATTR *attribs, OBJAFFINE *affine, int tile_index) {
    p->x = ((SCREEN_WIDTH/2) - (PLAYER_SIZE/2)) << FLOAT_SHIFT;
	p->y = ((SCREEN_HEIGHT/2) - (PLAYER_SIZE/2)) << FLOAT_SHIFT;
    p->dx = 0;
    p->dy = 0;
    p->angle = 0;
    p->tile_index = tile_index;
    p->obj = attribs;
    p->affine = affine;
    p->active = true;

    // Configuração inicial do Hardware (OAM)
    p->obj->attr0 = ATTR0_COLOR_16 | ATTR0_SQUARE | ATTR0_ROTSCALE_DOUBLE; 
    p->obj->attr1 = ATTR1_SIZE_16 | ATTR1_ROTDATA(0);
    p->obj->attr2 = ATTR2_PALETTE(0) | tile_index;
}

void player_update(Player *p, u16 keys) {
    // 1. Rotação (Gerencia apenas o ângulo)
    if (keys & KEY_LEFT)  p->angle--;
    if (keys & KEY_RIGHT) p->angle++;

    // 2. Aceleração (Física de Asteroids)
    if (keys & KEY_UP) {
        p->dx += GET_SIN(p->angle) >> 4; 
        p->dy -= GET_COS(p->angle) >> 4; 
    }

    // Retro propulsão (Freio)
    if (keys & KEY_DOWN) {
        p->dx -= GET_SIN(p->angle) >> 6;
        p->dy += GET_COS(p->angle) >> 6;
    }

    // 3. Fricção (Opcional: faz a nave parar aos poucos)
    // p->dx = (p->dx * 250) >> 8; 
    // p->dy = (p->dy * 250) >> 8;

    if (p->dx > MAX_SPEED) p->dx = MAX_SPEED;
    else if (p->dx < -MAX_SPEED) p->dx = -MAX_SPEED;

    // Trava o eixo Y
    if (p->dy > MAX_SPEED) p->dy = MAX_SPEED;
    else if (p->dy < -MAX_SPEED) p->dy = -MAX_SPEED;

    // 4. Aplica movimento à posição
    p->x += p->dx >> 2; // Ajuste o shift para controlar a sensibilidade
    p->y += p->dy >> 2;

    // 5. Wrap-around da tela (Nave reaparece do outro lado)
    if (p->x < -PLAYER_SIZE << FLOAT_SHIFT) p->x = SCREEN_WIDTH << FLOAT_SHIFT;
    if (p->x > SCREEN_WIDTH << FLOAT_SHIFT) p->x = -PLAYER_SIZE << FLOAT_SHIFT;
    if (p->y < -PLAYER_SIZE << FLOAT_SHIFT) p->y = SCREEN_HEIGHT << FLOAT_SHIFT;
    if (p->y > SCREEN_HEIGHT << FLOAT_SHIFT) p->y = -PLAYER_SIZE << FLOAT_SHIFT;
}

void player_draw(Player *p) {
    // Converte ponto fixo de volta para pixels inteiros para a OAM
    int screen_x = (p->x >> FLOAT_SHIFT) ;
    int screen_y = (p->y >> FLOAT_SHIFT) ;

    int render_x = screen_x - (PLAYER_SIZE / 2);
    int render_y = screen_y - (PLAYER_SIZE / 2);

    p->obj->attr0 = (p->obj->attr0 & ~MASK_Y) | (render_y & MASK_Y);
    p->obj->attr1 = (p->obj->attr1 & ~MASK_X) | (render_x & MASK_X);

    int cos_val = GET_COS(p->angle);
    int sin_val = GET_SIN(p->angle);

    p->affine->pa = cos_val;
    p->affine->pb = sin_val;
    p->affine->pc = -sin_val;
    p->affine->pd = cos_val;
}