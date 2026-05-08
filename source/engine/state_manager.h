#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "../screens/scene.h"

// Permite que uma tela mude para outra
void set_scene(const Scene* new_scene);

// Funções que o main.c vai chamar no loop
void sm_update();
void sm_draw();

#endif