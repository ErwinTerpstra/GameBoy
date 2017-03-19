#ifndef _VIDEO_CONTROLLER_H_
#define _VIDEO_CONTROLLER_H_

#include "environment.h"

namespace libdmg
{
	class CPU;
	class Memory;

	class VideoController
	{

	public:
		enum Mode
		{
			MODE_HBLANK = 0,
			MODE_VBLANK = 1,
			MODE_SEARCHING_OAM = 2,
			MODE_TRANSFERRING_DATA = 3
		};

		void(*VBlankCallback)();

	private:
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

		uint8_t* lcdControlRegister;
		uint8_t* statRegister;

		uint8_t scanline;
		uint64_t ticks;
		uint16_t modeTicks;
		Mode currentMode;

	public:
		VideoController(CPU& cpu, Memory& memory, uint8_t* videoBuffer);

		void Sync();
		void DrawTileset();

		Mode CurrentMode() const { return currentMode; }
		uint8_t Scanline() const { return scanline; }
	private:
		void Step();

		void DrawLine();

		void DecodeTile(uint16_t tileAddress, uint8_t* tileBuffer);

		void SwitchMode(Mode mode);
		void SetScanline(uint8_t scanline);
	};
}

#endif