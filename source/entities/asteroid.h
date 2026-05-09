#ifndef ASTEROID_H
#define ASTEROID_H

#include <gba.h>
#include <stdbool.h>
#include <stdlib.h>



typedef struct {
    int x, y;          // Posição (Ponto Fixo)
    int dx, dy;        // Vetores de Velocidade (Ponto Fixo)
    int size;          // Tamanho do asteroide (0=grande, 1=médio, 2=pequeno)
    OBJATTR *obj;      // Ponteiro para o atributo na OAM
    bool active;      // Flag para controle de existência
} Asteroid;

enum AsteroidSize {
    ASTEROID_LARGE = 32,
    ASTEROID_MEDIUM = 16,
    ASTEROID_SMALL = 8
};

// Métodos "Públicos"
void asteroid_init(Asteroid *a, int start_x, int start_y, enum AsteroidSize size, int angle, OBJATTR *attribs);
void asteroid_update(Asteroid *a);
void asteroid_draw(Asteroid *a);
void asteroid_destroy(Asteroid *a);

#endif
