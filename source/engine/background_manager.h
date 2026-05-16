#include <gba_types.h>
#include "../config.h"

#ifndef BACKGROUND_MANAGER_H
#define BACKGROUND_MANAGER_H



void bg_manager_init();
void bg_manager_update_opose_ship(s32 ship_dx, s32 ship_dy);
void bg_manager_update(int pos_x, int pos_y);

#endif