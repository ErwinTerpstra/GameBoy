#ifndef _VIDEO_CONTROLLER_H_
#define _VIDEO_CONTROLLER_H_

#include "environment.h"
#include "memorypointer.h"

namespace libdmg
{
	class CPU;
	class Memory;
	class MemoryBank;

	class Video
	{

	public:
		enum Mode
		{
			MODE_HBLANK = 0,
			MODE_VBLANK = 1,
			MODE_SEARCHING_OAM = 2,
			MODE_TRANSFERRING_DATA = 3
		};

		enum Layer
		{
			LAYER_BACKGROUND = 0,
			LAYER_WINDOW = 1,
			LAYER_SPRITES = 2
		};

		void(*VBlankCallback)();

	private:
		struct Sprite
		{
			uint8_t y;
			uint8_t x;
			uint8_t tileIdx;
			uint8_t flags;
		};

		enum SpriteFlags
		{
			SPRITE_PALETTE = 4,
			SPRITE_FLIP_X = 5,
			SPRITE_FLIP_Y = 6,
			SPRITE_ORDER = 7,
		};

		enum LCDCFlags
		{
			LCDC_BG_ENABLE = 0,
			LCDC_SPRITE_ENABLE = 1,
			LCDC_SPRITE_SIZE = 2,
			LCDC_BG_MAP_SELECT = 3,
			LCDC_BG_DATA_SELECT = 4,
			LCDC_WINDOW_ENABLE = 5,
			LCDC_WINDOW_MAP_SELECT = 6,
			LCDC_DISPLAY_ENABLE = 7
		};

		enum STATFlags
		{
			STAT_LYC_COINCEDENCE = 2,
			STAT_HBLANK_INTERRUPT = 3,
			STAT_VBLANK_INTERRUPT = 4,
			STAT_SEARCH_OAM_INTERRUPT = 5,
			STAT_LYC_INTERRUPT = 6,
		};

		CPU& cpu;
		Memory& memory;

		uint8_t* videoBuffer;

		MemoryPointer lcdControlRegister;
		MemoryPointer statRegister;
		MemoryPointer paletteRegister;
		MemoryPointer scanlineRegister;
		MemoryPointer scanlineCompareRegister;

		MemoryBank* vram;

		uint8_t scanline;
		uint64_t ticks;
		uint16_t modeTicks;
		Mode currentMode;

		bool layerStates[3];

	public:
		Video(CPU& cpu, Memory& memory, uint8_t* videoBuffer);

		void Reset();
		void Sync(const uint64_t& targetTicks);
		void DrawTileset();

		void SetLayerState(Layer layer, bool state) { layerStates[layer] = state; }
		bool GetLayerState(Layer layer) const { return layerStates[layer]; }

		Mode CurrentMode() const { return currentMode; }
		uint8_t Scanline() const { return scanline; }
	private:
		void Step();

		void DrawLine();

		void DrawMap(uint16_t mapAddress, uint16_t tileDataAddress, uint8_t palette, uint8_t scrollX, uint8_t scrollY);
		void DrawSprites();

		uint8_t GetPixel(uint8_t x, uint8_t y);
		void SetPixel(uint8_t x, uint8_t y, uint8_t color);

		void DecodeTile(uint16_t tileAddress, uint8_t* tileBuffer);
		void DecodeTile(uint16_t tileAddress, uint8_t tileY, uint8_t* tileBuffer);

		void SwitchMode(Mode mode);
		void SetScanline(uint8_t scanline);
	};
}

#endif