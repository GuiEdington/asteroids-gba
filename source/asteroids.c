#include <gba.h>
#include <stdbool.h>   // Necessário para o 'bool' funcionar
#include "spaceship.h" // Import do sprite da nave e sua paleta


#define MASK_Y 0x00FF
#define MASK_X 0x01FF
#define VBLANK_IRQ_ENABLE 0x08

const int SHEIGHT = 160;
const int SWIDTH = 240;
const int PLAYER_SIZE = 16;

// LUT de Seno e Cosseno (32 valores para 360 graus, ou seja, 11.25 graus por passo)
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

void cleanOam(OBJATTR* oam) {
	// Limpa a OAM (Object Attribute Memory) para garantir que nenhum sprite residual seja renderizado.
	for(int i = 0; i < 128; i++) {
		oam[i].attr0 = ATTR0_DISABLED;
	}
}

void loadSpritesInVram() {
	// Método para carregar os gráficos e paletas dos sprites na memória de vídeo (VRAM) do GBA.
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

    Entity player;
	player.x = ((SWIDTH/2) - (PLAYER_SIZE/2)) << 8;
	player.y = ((SHEIGHT/2) - (PLAYER_SIZE/2)) << 8;
	player.dx = 0;
	player.dy = 0;
	player.active = true;
	player.angle = 0;

	OBJATTR shadow_oam[128];
	OBJAFFINE* affine = (OBJAFFINE*)shadow_oam;
	cleanOam(shadow_oam);

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
            /* Aplica a mesma trigonometria, mas com os sinais invertidos.
             * A nave "acelera para trás" na direção que o bico está apontando.
             * Usei >> 8 (dividir por 256) para a ré ser mais fraca que a aceleração frontal.
             */
            player.dx -= GET_SIN(player.angle) >> 6;
            player.dy += GET_COS(player.angle) >> 6;
        }

		// APLICAÇÃO DA FÍSICA
		player.x += player.dx;
		player.y += player.dy;

		// CÓPIA TRUNCADA PARA DESENHO
		int draw_x = player.x >> 8;
		int draw_y = player.y >> 8;

		// LÓGICA DE TELA INFINITA (Wrap-around)
		if (draw_x < -PLAYER_SIZE) {
			player.x = SWIDTH << 8;
		} else if (draw_x > SWIDTH) {
			player.x = -PLAYER_SIZE << 8;
		}

		if (draw_y < -PLAYER_SIZE) {
            player.y = SHEIGHT << 8;
        } else if (draw_y > SHEIGHT) {
            player.y = -PLAYER_SIZE << 8;
        }

		// Atualiza a posição de desenho caso tenha saído da tela
		draw_x = player.x >> 8;
        draw_y = player.y >> 8;

		// Como o hardware dobrou a "caixa" (de 16 para 32), o gráfico foi parar no meio dela.
        // Nós subtraímos a metade do tamanho original (8 pixels) para recentralizar a nave
        // na posição física exata onde o jogador acha que ela está.
        int render_x = draw_x - (PLAYER_SIZE / 2);
        int render_y = draw_y - (PLAYER_SIZE / 2);

		// MATRIZ DE ROTAÇÃO
        // Pega os valores da LUT e injeta na Matriz 0 da OAM
        int cos_val = GET_COS(player.angle);
        int sin_val = GET_SIN(player.angle);

		affine[0].pa = cos_val;
        affine[0].pb = sin_val;
        affine[0].pc = -sin_val;
        affine[0].pd = cos_val;

		// RENDERIZAÇÃO
        // ATTR0_ROTSCALE_DOUBLE avisa que o sprite vai girar e solicita uma área de buffer dupla para evitar clipping.
        // ATTR1_ROTDATA(0) diz para ele usar a matriz que acabamos de configurar ali em cima.
        shadow_oam[0].attr0 = (render_y & MASK_Y) | ATTR0_COLOR_256 | ATTR0_SQUARE | ATTR0_ROTSCALE_DOUBLE;
        shadow_oam[0].attr1 = (render_x & MASK_X) | ATTR1_SIZE_16 | ATTR1_ROTDATA(0);
        shadow_oam[0].attr2 = ATTR2_PALETTE(0) | 0;

		VBlankIntrWait();

		dmaCopy(shadow_oam, OAM, 128 * sizeof(OBJATTR));
    }
}