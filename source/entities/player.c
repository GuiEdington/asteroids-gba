#include "player.h"
#include "../config.h" 

int player_game_over;

void player_init(Player *p, OBJATTR *attribs, OBJAFFINE *affine, int tile_index) {
    p->x = ((SCREEN_WIDTH/2) - (PLAYER_SIZE/2)) << FLOAT_SHIFT;
	p->y = ((SCREEN_HEIGHT/2) - (PLAYER_SIZE/2)) << FLOAT_SHIFT;
    p->dx = 0;
    p->dy = 0;
    p->angle = 0;
    p->invuln_timer = 0;
    p->tile_index = tile_index;
    p->obj = attribs;
    p->affine = affine;
    p->state = STATE_ALIVE;
    p->death_timer = 0;
    p->animation_frame = 0;
    p->animation_counter = 0;
    p->accelerating = 0;
    
    player_game_over = 0;

    // Configuração inicial do Hardware (OAM)
    p->obj->attr0 = ATTR0_COLOR_16 | ATTR0_SQUARE | ATTR0_ROTSCALE_DOUBLE; 
    p->obj->attr1 = ATTR1_SIZE_16 | ATTR1_ROTDATA(0);
    p->obj->attr2 = ATTR2_PALETTE(0) | ATTR2_PRIORITY(1) | tile_index;
}

void player_update_alive(Player *p, u16 keys, u16 keys_released) {
    // 1. Rotação (Gerencia apenas o ângulo)
    if (keys & KEY_LEFT)  p->angle -= 4;
    if (keys & KEY_RIGHT) p->angle += 4;

    // 2. Aceleração (Física de Asteroids)
    if (keys & KEY_UP) {
        p->dx += (GET_SIN(p->angle) << 1) >> 3; 
        p->dy -= (GET_COS(p->angle) << 1) >> 3;
        p->accelerating = 1;
        p->animation_counter++;
        if (p->animation_counter >= 5) {
            p->animation_counter = 0;
            p->animation_frame++;
            if (p->animation_frame > 3) p->animation_frame = 1; // Supondo que a animação de propulsão tenha 3 frames
        } 
    }

    if (keys_released & KEY_UP) {
        p->animation_frame = 0; // Volta para o frame normal quando solta o acelerador
        p->accelerating = 0;
    }

    // Retro propulsão (Freio)
    if (keys & KEY_DOWN) {
        p->dx -= GET_SIN(p->angle) >> 5;
        p->dy += GET_COS(p->angle) >> 5;
    }

    // Fricção (Faz a nave parar aos poucos)
    p->dx = (p->dx * 250) >> 8; 
    p->dy = (p->dy * 250) >> 8;

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

    if (p->invuln_timer > 0) p->invuln_timer--; // Desconta 1 frame
}

void player_update_exploding(Player *p) {
    p->animation_counter++;
    if (p->animation_counter >= 5) { // A cada 10 frames, muda o frame da animação
        p->animation_counter = 0;
        if (p->animation_frame >= 5) { // Supondo que a animação tenha 5 frames
            p->state = STATE_DEAD; // Animação terminou, jogador está morto
            p->death_timer = 120; // 2 segundos antes de poder ressuscitar
        }
        p->animation_frame++;
    }
}

void player_die(Player *p, int lives) {
    if (lives <= 0) {
        player_game_over = 1;
    }
    p->state = STATE_EXPLODING;
    p->animation_counter = 0;
}

void player_update(Player *p, u16 keys, u16 keys_released) {
    switch (p->state) {
        case STATE_ALIVE:
            player_update_alive(p, keys, keys_released);
            break;
        case STATE_EXPLODING:
            player_update_exploding(p);
            break;
        case STATE_DEAD:
            // Talvez queira adicionar uma tela de Game Over ou reiniciar o jogo
            if (p->death_timer > 0) {
                p->death_timer--;
            } else if (!player_game_over) {
                // Deu os 2 segundos! Resuscita o jogador
                p->obj->attr0 = ATTR0_COLOR_16 | ATTR0_SQUARE | ATTR0_ROTSCALE_DOUBLE; 
                p->state = STATE_ALIVE;
                p->x = ((SCREEN_WIDTH/2) - (PLAYER_SIZE/2)) << FLOAT_SHIFT;
                p->y = ((SCREEN_HEIGHT/2) - (PLAYER_SIZE/2)) << FLOAT_SHIFT;
                p->dx = 0;
                p->dy = 0;
                p->angle = 0;
                p->invuln_timer = 120; // 2 segundos de invulnerabilidade
                p->obj->attr2 = ATTR2_PALETTE(0) | ATTR2_PRIORITY(1) | p->tile_index; // Volta para o tile da nave
                p->death_timer = 0;
                p->animation_frame = 0;
                p->animation_counter = 0;
            }
            break;
    }    
}

void player_draw(Player *p) {
    if (p->state == STATE_DEAD) {
        // Se o jogador estiver morto, não desenha nada
        p->obj->attr0 = ATTR0_DISABLED;
        return;
    }

    if (p->state == STATE_EXPLODING) {
        if (p->animation_frame < 5) {
            p->obj->attr2 = ATTR2_PALETTE(2) | ATTR2_PRIORITY(1) | (EXPLOSION_TILE_POS + p->animation_frame * 4);
        }
        return;
    }

    if (p->state == STATE_ALIVE && p->accelerating) {
        p->obj->attr2 = ATTR2_PALETTE(0) | ATTR2_PRIORITY(1) | (SPACESHIP_TILE_POS + p->animation_frame * 4);
    } else {
        p->obj->attr2 = ATTR2_PALETTE(0) | ATTR2_PRIORITY(1) | SPACESHIP_TILE_POS;
    }

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

    // Efeito visual: Faz a nave piscar a cada 4 frames usando operador bit a bit
    // Se o bit 2 estiver ligado, nós escondemos o sprite
    if (p->invuln_timer & 4) {
        // Esconde a nave manipulando o Atributo 0 da OAM
        // (Assumindo que você usa libgba ou acessa a OAM direto)
        p->obj->attr0 = (p->obj->attr0 & ~MASK_Y) | (160 & MASK_Y); 
    } else {
        // Mostra a nave (Remove a flag de hide)
        p->obj->attr0 = (p->obj->attr0 & ~MASK_Y) | (render_y & MASK_Y);
    }
}