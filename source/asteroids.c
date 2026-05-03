#include <gba.h>
#include <stdbool.h>   // Necessário para o 'bool' funcionar
#include "spaceship.h" 

typedef struct {
    int x, y;
    int dx, dy;
    int angle;
    bool active;
} Entity;

int SHEIGHT = 160;
int SWIDTH = 240;

void cleanOam() {
	for(int i = 0; i < 128; i++) {
		OAM[i].attr0 = ATTR0_DISABLED;
	}
}

void loadSprintesInVram() {
	dmaCopy(spaceshipTiles, SPRITE_GFX, spaceshipTilesLen);
	dmaCopy(spaceshipPal, SPRITE_PALETTE, spaceshipPalLen);
}

void initialDisplayConfig() {
	// --- 1. CONFIGURAÇÃO DE HARDWARE OBRIGATÓRIA ---
    // Avisa o chip de vídeo para mandar o sinal que acorda o VBlankIntrWait()
    irqInit();
    REG_DISPSTAT |= 0x08;
    irqEnable(IRQ_VBLANK);
	// Background color (fundo azul escuro)
    BG_PALETTE[0] = RGB5(0, 0, 51);
    // 2. Configuração do Display para Sprites
    REG_DISPCNT = MODE_0 | OBJ_ENABLE | OBJ_1D_MAP;
}

int main(void) {
	initialDisplayConfig();
    
	loadSprintesInVram();

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

		OAM[0].attr0 = (draw_y & 0x00FF) | ATTR0_COLOR_256 | ATTR0_SQUARE;
        OAM[0].attr1 = (draw_x & 0x01FF) | ATTR1_SIZE_16;
        OAM[0].attr2 = ATTR2_PALETTE(0) | 0;
    }
}