#ifndef ASTEROID_MANAGER_H
#define ASTEROID_MANAGER_H

#include <gba.h>
#include <stdlib.h>
#include "../entities/asteroid.h"

#define MAX_ASTEROIDS_POOL 64

void asteroid_manager_init(OBJATTR *oam_buffer, int oam_start_index);
void asteroid_manager_update();
void asteroid_manager_draw();
void asteroid_manager_spawn(int count);
int asteroid_manager_get_collider(int index, int *cx, int *cy, int *radius);
int destroy_asteroid(int index);
int get_size(int index);
int asteroid_manager_get_active_count();

#endif