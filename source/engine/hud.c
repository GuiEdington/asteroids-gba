#include <stdio.h>
#include <gba.h>
#include "text_engine.h" // A sua engine de texto

#define SHIP_CHAR 0x7F

// Estado do Jogador
int player_score = 0;
int player_lives = 3;

// A nossa "Dirty Flag" (Começa em 1 para desenhar no primeiro frame)
int hud_needs_update = 1; 

void hud_init() {
    text_init(0);
}

void hud_update() {
    // Só consome processamento SE a flag avisar que houve mudança!
    if (hud_needs_update) {
        char score_buffer[10];
        char lives_buffer[10];
        
        // Formata o Score com 5 dígitos (ex: 00050)
        sprintf(score_buffer, "%05d", player_score);
        // Desenha logo na frente da palavra "SCORE:" (Coluna 8)
        text_draw(1, 1, (unsigned char*)score_buffer);
        
        // Formata a Vida com 1 dígito
        sprintf(lives_buffer, "%d", player_lives);
        // Desenha logo na frente da palavra "LIVES:" (Coluna 28)
        text_draw(1, 2, (unsigned char*)"          ");
        int i;
        for (i = 0; i < player_lives; i++) {
            lives_buffer[i] = SHIP_CHAR;
        }
        lives_buffer[i] = '\0';
        text_draw(1, 2, (unsigned char*)lives_buffer); // Desenha um ícone de nave para cada vida
        // Desliga a flag. O hardware desenhará isso de graça até a próxima mudança!
        hud_needs_update = 0; 
    }
}

// Quando um asteroide for destruído, você chama esta função:
void update_score(int points) {
    player_score = points;
    hud_needs_update = 1; // Avisa a HUD para acordar e redesenhar!
}

void update_lives(int lives) {
    player_lives = lives;
    hud_needs_update = 1; // Avisa a HUD para acordar e redesenhar!
}