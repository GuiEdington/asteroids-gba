#include <gba_types.h>
#include "../config.h"

#ifndef BACKGROUND_MANAGER_H
#define BACKGROUND_MANAGER_H



void bg_manager_init();
void bg_manager_update(s32 ship_dx, s32 ship_dy);

#endif