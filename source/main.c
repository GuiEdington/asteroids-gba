#include <gba.h>
#include <stdbool.h>
#include <stdlib.h>

// Defines
#include "config.h"
#include "screens/scene.h"
#include "engine/state_manager.h"
#include "screens/title_screen.h"

OBJATTR shadow_oam[128] __attribute__((aligned(4)));

void initialDisplayConfig() {
	// --- CONFIGURAÇÃO DE HARDWARE OBRIGATÓRIA ---
    
    // irqInit(): Inicializa o sistema de interrupções (IRQs) da biblioteca libgba.
    // Ele limpa e prepara a tabela interna na memória RAM onde a CPU vai procurar 
    // instruções quando um hardware (como o monitor ou os botões) pedir atenção.
    irqInit();

    // REG_DISPSTAT: Display Status Register (Comunicação com o Monitor).
    // O operador |= liga o bit VBLANK_IRQ_ENABLE sem desligar outras configurações.
    // Isso autoriza fisicamente o chip do LCD a disparar um sinal elétrico de alerta 
    // no exato milissegundo em que terminar de desenhar a tela (Vertical Blank).
    REG_DISPSTAT |= VBLANK_IRQ_ENABLE; 

    // irqEnable(): Configuração da CPU principal (ARM7TDMI).
    // Diz ao processador para "abrir os ouvidos" e aceitar especificamente a 
    // interrupção IRQ_VBLANK. Sem isso, o monitor emite o sinal, mas a CPU ignora.
    irqEnable(IRQ_VBLANK);
}

int main(void) {
    srand(REG_TM0CNT);

	initialDisplayConfig();

    set_scene(&title_screen); // Diz ao State Manager para carregar a fase do jogo

    while (1) {
		sm_update();        
		
        VBlankIntrWait();
        
        sm_draw();

		dmaCopy(shadow_oam, OAM, 128 * sizeof(OBJATTR));
    }
    return 0;
}