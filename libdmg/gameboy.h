#ifndef _GAME_BOY_H_
#define _GAME_BOY_H_

#define GB_CLOCK_FREQUENCY			4194304
#define GB_FRAME_SEQUENCER_PERIOD	512
#define GB_AUDIO_SAMPLE_PERIOD		128

#define GB_SCREEN_WIDTH				160
#define GB_SCREEN_HEIGHT			144

#define GB_BG_WIDTH					256
#define GB_BG_HEIGHT				256

#define GB_TILE_WIDTH				8
#define GB_TILE_HEIGHT				8
#define GB_TILE_SIZE				GB_TILE_WIDTH * GB_TILE_HEIGHT / 4

#define GB_MAX_CARTRIDGE_SIZE		8 * 1024 * 1024

#define GB_HBLANK_DURATION			204
#define GB_VBLANK_DURATION			4560
#define GB_SEARCH_OAM_DURATION		80
#define GB_TRANSFER_DATA_DURATION	172

#define GB_MAX_SCANLINE				153

#define GB_MAX_SPRITES				40

#define GB_ROM				0x0000
#define GB_CRAM				0xA000
#define GB_VRAM				0x8000

#define GB_BG_MAP_0			0x9800
#define GB_BG_MAP_1			0x9C00
#define GB_TILE_DATA_0		0x8000
#define GB_TILE_DATA_1		0x9000

#define GB_OAM				0xFE00
#define GB_IO_REGISTERS		0xFF00
#define GB_HIMEM			0xFF80

#define GB_REG_JOYP			0xFF00			// Joypad

#define GB_REG_DIV			0xFF04			// Fixed divider timer
#define GB_REG_TIMA			0xFF05			// Timer counter
#define GB_REG_TMA			0xFF06			// Timer modulo
#define GB_REG_TAC			0xFF07			// Timer control

#define GB_REG_NR10			0xFF10
#define GB_REG_NR11			0xFF11
#define GB_REG_NR12			0xFF12
#define GB_REG_NR13			0xFF13
#define GB_REG_NR14			0xFF14

#define GB_REG_NR21			0xFF16
#define GB_REG_NR22			0xFF17
#define GB_REG_NR23			0xFF18
#define GB_REG_NR24			0xFF19

#define GB_REG_NR30			0xFF1A
#define GB_REG_NR31			0xFF1B
#define GB_REG_NR32			0xFF1C
#define GB_REG_NR33			0xFF1D
#define GB_REG_NR34			0xFF1E

#define GB_REG_NR41			0xFF20
#define GB_REG_NR42			0xFF21
#define GB_REG_NR43			0xFF22
#define GB_REG_NR44			0xFF23

#define GB_REG_NR50			0xFF24
#define GB_REG_NR51			0xFF25
#define GB_REG_NR52			0xFF26

#define GB_REG_WAVE			0xFF30

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

#define GB_REG_WY			0xFF4A			// Window Y
#define GB_REG_WX			0xFF4B			// Window X

#define GB_REG_IF			0xFF0F			// Interrupt flag
#define GB_REG_IE			0xFFFF			// Interrupt enable

#define GB_MBC1_MAX_RAM		0x8000

#endif