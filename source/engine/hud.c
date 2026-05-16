#include <stdio.h>
#include <gba.h>
#include "text_engine.h" // A sua engine de texto

// Estado do Jogador
int player_score = 0;
int player_lives = 3;

// A nossa "Dirty Flag" (Começa em 1 para desenhar no primeiro frame)
int hud_needs_update = 1; 

void hud_init() {
    // 1. Garante que a memória da fonte está carregada no CBB 1 e SBB 29 (BG2)
    // (Pode usar a sua função text_init() original aqui)
    // Passa 0 para não configurar o BG3 para o "Press START", já que isso é só para a tela de título
    text_init(0); 

    // 2. Desenha os Textos Estáticos (O que não muda)
    // Coluna 21, Linha 1 (Canto superior direito)
    text_draw(1, 2, (unsigned char*)"Lives:"); 
}

void hud_update() {
    // Só consome processamento SE a flag avisar que houve mudança!
    if (hud_needs_update) {
        char score_buffer[10];
        char lives_buffer[5];
        
        // Formata o Score com 5 dígitos (ex: 00050)
        sprintf(score_buffer, "%05d", player_score);
        // Desenha logo na frente da palavra "SCORE:" (Coluna 8)
        text_draw(1, 1, (unsigned char*)score_buffer);
        
        // Formata a Vida com 1 dígito
        sprintf(lives_buffer, "%d", player_lives);
        // Desenha logo na frente da palavra "LIVES:" (Coluna 28)
        text_draw(8, 2, (unsigned char*)lives_buffer);
        
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