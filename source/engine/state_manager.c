#include "state_manager.h"
#include <stddef.h> // Para a macro NULL
#include <gba_video.h>

// Variável estática: Escopo restrito a este arquivo!
static const Scene* current_scene = NULL;

void set_scene(const Scene* new_scene) {
    REG_DISPCNT = 0;
    if (current_scene != NULL && current_scene->exit != NULL) {
        current_scene->exit();
    }

    current_scene = new_scene;
    
    if (current_scene != NULL && current_scene->init != NULL) {
        current_scene->init();
    }
}

void sm_update() {
    if (current_scene != NULL && current_scene->update != NULL) {
        current_scene->update();
    }
}

void sm_draw() {
    if (current_scene != NULL && current_scene->draw != NULL) {
        current_scene->draw();
    }
}