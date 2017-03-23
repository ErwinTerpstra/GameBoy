#ifndef _GAME_BOY_H_
#define _GAME_BOY_H_

#define GB_CLOCK_FREQUENCY			4194304

#define GB_SCREEN_WIDTH				160
#define GB_SCREEN_HEIGHT			144

#define GB_BG_WIDTH					256
#define GB_BG_HEIGHT				256

#define GB_TILE_WIDTH				8
#define GB_TILE_HEIGHT				8
#define GB_TILE_SIZE				GB_TILE_WIDTH * GB_TILE_HEIGHT / 4

#define GB_MAX_CARTRIDGE_SIZE		8 * 1024 * 1024
#define GB_MAIN_MEM_SIZE			64 * 1024

#define GB_HBLANK_DURATION			204
#define GB_VBLANK_DURATION			4560
#define GB_SEARCH_OAM_DURATION		80
#define GB_TRANSFER_DATA_DURATION	172

#define GB_MAX_SCANLINE				153

#define GB_MAX_SPRITES				40

#define GB_BG_MAP_0			0x9800
#define GB_BG_MAP_1			0x9C00
#define GB_TILE_DATA_0		0x8000
#define GB_TILE_DATA_1		0x8800

#define GB_OAM				0xFE00
#define GB_IO_REGISTERS		0xFF00
#define GB_HIMEM			0xFF80

#define GB_REG_JOYP			0xFF00			// Joypad

#define GB_REG_DIV			0xFF04			// Fixed divider timer
#define GB_REG_TIMA			0xFF05			// Timer counter
#define GB_REG_TMA			0xFF06			// Timer modulo
#define GB_REG_TAC			0xFF07			// Timer control

#define GB_REG_LCDC			0xFF40			// LCD control
#define GB_REG_STAT			0xFF41			// LCD status

#define GB_REG_SCY			0xFF42			// Scroll Y
#define GB_REG_SCX			0xFF43			// Scroll X
#define GB_REG_LY			0xFF44			// LCD Y-coordinate
#define GB_REG_LYC			0xFF45			// LY compare value

#define GB_REG_DMA			0xFF46			// DMA transfer start address

#define GB_REG_BGP			0xFF47			// Background palette data
#define GB_REG_OBP0			0xFF48			// Object palette 0 data
#define GB_REG_OBP1			0xFF49			// Object palette 1 data

#define GB_REG_IF			0xFF0F			// Interrupt flag
#define GB_REG_IE			0xFFFF			// Interrupt enable


#endif