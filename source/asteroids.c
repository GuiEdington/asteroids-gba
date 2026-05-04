#include <gba.h>
#include <stdbool.h>   // Necessário para o 'bool' funcionar
#include "spaceship.h" // Import do sprite da nave e sua paleta


#define MASK_Y 0x00FF
#define MASK_X 0x01FF
#define VBLANK_IRQ_ENABLE 0x08

const int SHEIGHT = 160;
const int SWIDTH = 240;
const s16 sin_lut[32] = {
    0, 50, 98, 142, 181, 212, 236, 251, 
    256, 251, 236, 212, 181, 142, 98, 50, 
    0, -50, -98, -142, -181, -212, -236, -251, 
    -256, -251, -236, -212, -181, -142, -98, -50
};

#define GET_SIN(a) (sin_lut[(a) & 31])
#define GET_COS(a) (sin_lut[((a) + 8) & 31])

typedef struct {
    int x, y;
    int dx, dy;
    int angle;
    bool active;
} Entity;

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
	player.angle = 0;

	volatile OBJAFFINE* affine = (volatile OBJAFFINE*)OAM;

    while (1) {
        scanKeys();
		u16 keys = keysHeld();

		// CONTROLE DE ROTAÇÃO
		if (keys & KEY_LEFT)  player.angle--;
        if (keys & KEY_RIGHT) player.angle++;

		// CONTROLE DE MOTOR (Inércia baseada no ângulo usando >> 4 como redutor de potência)
        if (keys & KEY_UP) {
            player.dx += GET_SIN(player.angle) >> 4; 
            player.dy -= GET_COS(player.angle) >> 4; 
        }

        // CONTROLE DE FREIO
        if (keys & KEY_DOWN) {
            /* Amortecedor Inercial (O Verdadeiro "Freio")
             * Reduz gradativamente a velocidade acumulada (dx e dy) em direção a zero,
             * independentemente de qual direção a nave está apontando no momento.
             * Subtrair a variável por ela mesma deslocada em 4 (dx/16) cria uma
             * curva de desaceleração incrivelmente suave e realista.
             */
            player.dx -= (player.dx >> 4);
            player.dy -= (player.dy >> 4);
        }

		// APLICAÇÃO DA FÍSICA
		player.x += player.dx;
		player.y += player.dy;

		// CÓPIA TRUNCADA PARA DESENHO
		int draw_x = player.x >> 8;
		int draw_y = player.y >> 8;

		// LÓGICA DE TELA INFINITA (Wrap-around)
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

		// Atualiza a posição de desenho caso tenha saído da tela
		draw_x = player.x >> 8;
        draw_y = player.y >> 8;

		// MATRIZ DE ROTAÇÃO
        // Pega os valores da LUT e injeta na Matriz 0 da OAM
        int cos_val = GET_COS(player.angle);
        int sin_val = GET_SIN(player.angle);

		VBlankIntrWait();
        
        affine[0].pa = cos_val;
        affine[0].pb = sin_val;
        affine[0].pc = -sin_val;
        affine[0].pd = cos_val;

		// RENDERIZAÇÃO
        // ATTR0_ROTSCALE avisa que o sprite vai girar. 
        // ATTR1_ROTDATA(0) diz para ele usar a matriz que acabamos de configurar ali em cima.
        OAM[0].attr0 = (draw_y & MASK_Y) | ATTR0_COLOR_256 | ATTR0_SQUARE | ATTR0_ROTSCALE;
        OAM[0].attr1 = (draw_x & MASK_X) | ATTR1_SIZE_16 | ATTR1_ROTDATA(0);
        OAM[0].attr2 = ATTR2_PALETTE(0) | 0;
    }
}