#ifndef ASTEROID_MANAGER_H
#define ASTEROID_MANAGER_H

#include <gba.h>
#include <stdlib.h>
#include "../entities/asteroid.h"

#define MAX_ASTEROIDS_POOL 64

void asteroid_manager_init(OBJATTR *oam_buffer, int oam_start_indexs);
void asteroid_manager_update();
void asteroid_manager_draw();
void asteroid_manager_spawn(int count);

#endif