#ifndef _VIDEO_CONTROLLER_H_
#define _VIDEO_CONTROLLER_H_

#include "environment.h"

namespace libdmg
{
	class CPU;
	class Memory;

	class VideoController
	{

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

		enum Mode
		{
			MODE_HBLANK = 0,
			MODE_VBLANK = 1,
			MODE_SEARCHING_OAM = 2,
			MODE_TRANSFERRING_DATA = 3
		};

		CPU& cpu;
		Memory& memory;

		uint8_t* videoBuffer;

		uint8_t* lcdControlRegister;
		uint8_t* statRegister;

		uint8_t scanline;
		uint16_t ticks;
		Mode currentMode;

	public:
		VideoController(CPU& cpu, Memory& memory, uint8_t* videoBuffer);

		void Step();

	private:
		void SwitchMode(Mode mode);

		void SetScanline(uint8_t scanline);
	};
}

#endif