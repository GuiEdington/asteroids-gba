#ifndef PLAYER_H
#define PLAYER_H

#include <gba.h>
#include <stdbool.h>

// Usamos ponto fixo 8.8 (multiplicamos por 256)
#define FLOAT_SHIFT 8
#define PLAYER_SIZE 16

typedef struct {
    int x, y;          // Posição (Ponto Fixo)
    int dx, dy;        // Vetores de Velocidade (Ponto Fixo)
    int angle;         // Índice da LUT (0-31)
    OBJATTR *obj;      // Ponteiro para o atributo na OAM
    int tile_index;    // Índice do tile inicial na VRAM
    bool active;      // Flag para controle de existência (útil para tiros e asteroides)
} Player;

// Métodos "Públicos"
void player_init(Player *p, OBJATTR *attribs, int tile_index);
void player_update(Player *p, u16 keys);
void player_draw(Player *p, OBJAFFINE *affine);

#endif