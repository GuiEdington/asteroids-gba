// config.h
#pragma once // Isso impede que o arquivo seja lido duas vezes e cause erros!
#include <gba_types.h>

// --- CONFIGURAÇÕES DE TELA ---
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

// --- MÁSCARAS E REGISTRADORES ---
#define MASK_Y 0x00FF
#define MASK_X 0x01FF
#define VBLANK_IRQ_ENABLE 0x08

// --- CONFIGURAÇÕES DO JOGO ---
#define MAX_SPEED (6 << 8)


// --- MAPA DA VRAM (SPRITES) ---
#define SPACESHIP_TILE_POS 0
#define AST_G_TILE_POS 16
#define AST_M_TILE_POS (AST_G_TILE_POS + 64)
#define AST_P_TILE_POS (AST_M_TILE_POS + 16)
#define BULLET_TILE_POS (AST_P_TILE_POS + 4)
#define EXPLOSION_TILE_POS (BULLET_TILE_POS + 2) 
#define PAL_SIZE 16

// Quantidade de tiles que cada sprite ocupa na VRAM
#define TILE_OFFSET_LARGE  16  // 32x32 = 16 tiles
#define TILE_OFFSET_MEDIUM 4   // 16x16 = 4 tiles
#define TILE_OFFSET_SMALL  1   // 8x8   = 1 tile

// LUT de Seno e Cosseno (32 valores para 360 graus, ou seja, 11.25 graus por passo)
extern const s16 sin_lut[32] __attribute__ ((section(".iwram")));

extern const u8 bullet_gfx[64] __attribute__ ((section(".iwram"), aligned(4)));

#define GET_SIN(a) (sin_lut[(u8)(a)])
#define GET_COS(a) (sin_lut[((u8)((a) + 64))])

#define FLOAT_SHIFT 8

// --- CONFIGURAÇÕES DE BACKGROUND ---
#define BG_CBB(n) ((n) << 2)
#define BG_SBB(n) ((n) << 8)

// 
#define ALL_BG 0x3F
#define BRIGHTNESS_DECREASE_MODE (3 << 6)
