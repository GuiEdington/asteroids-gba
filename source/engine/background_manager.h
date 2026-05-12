#include <gba_types.h>

#ifndef BACKGROUND_MANAGER_H
#define BACKGROUND_MANAGER_H

#define BG_CBB(n) ((n) << 2)
#define BG_SBB(n) ((n) << 8)

void bg_manager_init();
void bg_manager_update(s32 ship_dx, s32 ship_dy);

#endif