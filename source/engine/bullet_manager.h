#ifndef BULLET_MANAGER_H
#define BULLET_MANAGER_H

#include <gba.h>
#include "../entities/bullet.h"

// Definimos como potência de 2 para o nosso buffer circular!
#define MAX_BULLETS 16
#define BULLET_MASK 15

// Inicializa o manager passando o buffer da OAM e a partir de qual índice os tiros começam
void bullet_manager_init(OBJATTR *oam_buffer, int oam_start_index);

// Faz o loop de física e limpeza (desativa quem saiu da tela)
void bullet_manager_update();

// Faz o loop de renderização (atualiza a OAM)
void bullet_manager_draw();

// O "gatilho" chamado pelo level_game quando o jogador aperta A
void bullet_manager_spawn(int start_x, int start_y, int angle);

#endif