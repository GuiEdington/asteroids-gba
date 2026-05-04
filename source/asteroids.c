#include <gba.h>
#include <stdbool.h>   // Necessário para o 'bool' funcionar
#include "spaceship.h" // Import do sprite da nave e sua paleta

typedef struct {
    int x, y;
    int dx, dy;
    int angle;
    bool active;
} Entity;

const int SHEIGHT = 160;
const int SWIDTH = 240;

#define MASK_Y 0x00FF
#define MASK_X 0x01FF
#define VBLANK_IRQ_ENABLE 0x08

void cleanOam() {
	for(int i = 0; i < 128; i++) {
		OAM[i].attr0 = ATTR0_DISABLED;
	}
}

void loadSpritesInVram() {
	dmaCopy(spaceshipTiles, SPRITE_GFX, spaceshipTilesLen);
	dmaCopy(spaceshipPal, SPRITE_PALETTE, spaceshipPalLen);
}

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


    // --- CONFIGURAÇÃO DO DISPLAY E MEMÓRIA DE VÍDEO ---

    // BG_PALETTE[0]: Acessa o primeiro espaço da memória de paletas de fundo.
    // O índice [0] é especial no GBA: ele define a "cor de fundo" global (backdrop).
    // Valores RGB5 vão estritamente de 0 a 31. (0,0,31) = Azul puro em brilho máximo.
    BG_PALETTE[0] = RGB5(0, 0, 31);

    // REG_DISPCNT: Display Control Register (O maestro do chip de vídeo).
    // MODE_0     -> Define a arquitetura da tela para renderizar blocos 2D clássicos (Tiles).
    // OBJ_ENABLE -> Liga o motor de renderização da OAM. Sem isso, os sprites ficam invisíveis.
    // OBJ_1D_MAP -> Mapeamento 1D. Diz ao hardware para ler a memória VRAM de sprites como 
    //               uma "fita" contínua e linear, o que permite copiarmos arrays 
    //               diretamente do C usando dmaCopy sem nos preocupar com matrizes 2D.
    REG_DISPCNT = MODE_0 | OBJ_ENABLE | OBJ_1D_MAP;
}

int main(void) {
	initialDisplayConfig();
    
	loadSpritesInVram();

    cleanOam();

    Entity player;
	player.x = 112 << 8;
	player.y = 72 << 8;
	player.dx = 0;
	player.dy = 0;
	player.active = true;

    while (1) {
        scanKeys();
		u16 keys = keysHeld();

		if (keys & KEY_UP) {
			player.dy -= 15;
		}
		if (keys & KEY_DOWN) {
			player.dy += 15;
		}

		player.x += player.dx;
		player.y += player.dy;

		int draw_x = player.x >> 8;
		int draw_y = player.y >> 8;

		if (draw_x < -16) {
			player.x = SWIDTH << 8;
		} else if (draw_x > SWIDTH) {
			player.x = -16 << 8;
		}

		if (draw_y < -16) {
            player.y = SHEIGHT << 8;
        } else if (draw_y > SHEIGHT) {
            player.y = -16 << 8;
        }

		draw_x = player.x >> 8;
        draw_y = player.y >> 8;

		VBlankIntrWait();

		OAM[0].attr0 = (draw_y & MASK_Y) | ATTR0_COLOR_256 | ATTR0_SQUARE;
        OAM[0].attr1 = (draw_x & MASK_X) | ATTR1_SIZE_16;
        OAM[0].attr2 = ATTR2_PALETTE(0) | 0;
    }
}