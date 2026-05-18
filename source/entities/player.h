#ifndef PLAYER_H
#define PLAYER_H

#include <gba.h>
#include <stdbool.h>

// Usamos ponto fixo 8.8 (multiplicamos por 256)
#define FLOAT_SHIFT 8
#define PLAYER_SIZE 16

typedef enum {
    STATE_ALIVE,
    STATE_EXPLODING,
    STATE_DEAD 
} PlayerState;

typedef struct {
    int x, y;          // Posição (Ponto Fixo)
    int dx, dy;        // Vetores de Velocidade (Ponto Fixo)
    u8 angle;         // Índice da LUT de Seno/Cosseno (0-255)
    OBJATTR *obj;      // Ponteiro para o atributo na OAM
    OBJAFFINE *affine;  // Estrutura para rotação (se necessário)
    int tile_index;    // Índice do tile inicial na VRAM
    int invuln_timer;  // Timer para invulnerabilidade após ser atingido (opcional)
    PlayerState state; // Estado atual do jogador (vivo, explodindo, morto)
    int death_timer;  // Timer para controlar a duração da animação de morte (opcional)
    int animation_frame; // Frame atual da animação (opcional)
    int animation_counter;

} Player;

// Métodos "Públicos"
void player_init(Player *p, OBJATTR *attribs, OBJAFFINE *affine, int tile_index);
void player_update(Player *p, u16 keys);
void player_draw(Player *p);

#endif