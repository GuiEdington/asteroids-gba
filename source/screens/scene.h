#ifndef SCENE_H
#define SCENE_H

typedef struct {
    void (*init)();    // Carregamento de VRAM/Paletas
    void (*update)();  // Lógica de jogo e entrada
    void (*draw)();    // Atualização da OAM (VBlank)
} Scene;

#endif