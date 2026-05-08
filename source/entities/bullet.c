#include "bullet.h"
#include "../config.h" 

void bullet_init(Bullet *b, int start_x, int start_y, int angle, OBJATTR *attribs) {
    b->x = start_x + (4 << FLOAT_SHIFT);
    b->y = start_y + (4 << FLOAT_SHIFT);
    b->dx = GET_SIN(angle) << 1;
    b->dy = -GET_COS(angle) << 1;
    b->angle = angle;
    b->obj = attribs;
    b->active = true;

    b->obj->attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE; 
    b->obj->attr1 = ATTR1_SIZE_8;
    b->obj->attr2 = ATTR2_PALETTE(0) | 32;
}

void bullet_update(Bullet *b) {
    if (!b->active) return;
    b->x += b->dx;
    b->y += b->dy;
    if (b->x < 0 || b->x > SCREEN_WIDTH << FLOAT_SHIFT || b->y < 0 || b->y > SCREEN_HEIGHT << FLOAT_SHIFT) {
        b->active = false;
    }
}

void bullet_draw(Bullet *b) {
    if (b->active) {
        int screen_x = (b->x >> FLOAT_SHIFT) ;
        int screen_y = (b->y >> FLOAT_SHIFT) ;

        b->obj->attr0 = (b->obj->attr0 & ~MASK_Y) | (screen_y & MASK_Y);
        b->obj->attr1 = (b->obj->attr1 & ~MASK_X) | (screen_x & MASK_X);
    }
}

void bullet_destroy(Bullet *b) {
    b->active = false;
    b->obj->attr0 = ATTR0_DISABLED;
}