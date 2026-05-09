#ifndef BULLET_H
#define BULLET_H

#include <gba.h>
#include <stdbool.h>

typedef struct {
    int x, y;          // Posição (Ponto Fixo)
    int dx, dy;        // Vetores de Velocidade (Ponto Fixo)
    int angle;         // Índice da LUT (0-31)
    OBJATTR *obj;      // Ponteiro para o atributo na OAM
    bool active;      // Flag para controle de existência (útil para tiros e asteroides)
} Bullet;

// Métodos "Públicos"
void bullet_init(Bullet *b, int start_x, int start_y, int angle, OBJATTR *attribs);
void bullet_update(Bullet *b);
void bullet_draw(Bullet *b);
void bullet_destroy(Bullet *b);

#endif