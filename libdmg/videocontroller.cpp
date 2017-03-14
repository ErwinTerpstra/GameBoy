#include "videocontroller.h"

#include "cpu.h"
#include "memory.h"

#include "gameboy.h"

#include "util.h"

using namespace libdmg;

VideoController::VideoController(CPU& cpu, Memory& memory, uint8_t* videoBuffer) : 
	cpu(cpu), memory(memory), videoBuffer(videoBuffer), scanline(0), 
	ticks(0), modeTicks(0), currentMode(MODE_VBLANK)
{
	lcdControlRegister = memory.RetrievePointer(GB_REG_LCDC);
	statRegister = memory.RetrievePointer(GB_REG_STAT);
}

void VideoController::Sync()
{
	const uint64_t& targetTicks = cpu.Ticks();
	
	while (ticks < targetTicks)
		Step();
}

void VideoController::Step()
{
	switch (currentMode)
	{
		case MODE_HBLANK:

			if (++modeTicks == GB_HBLANK_DURATION)
			{
				if (scanline + 1 == GB_SCREEN_HEIGHT)
					SwitchMode(MODE_VBLANK);
				else
					SwitchMode(MODE_SEARCHING_OAM);

				SetScanline(scanline + 1);
			}

			break;

		case MODE_VBLANK:
			if (modeTicks > 0 && (modeTicks % 500) == 0)
				SetScanline(scanline + 1);

			if (++modeTicks == GB_VBLANK_DURATION)
			{
				scanline = 0;
				SwitchMode(MODE_SEARCHING_OAM);
			}
			break;

		case MODE_SEARCHING_OAM:

			if (++modeTicks == GB_SEARCH_OAM_DURATION)
				SwitchMode(MODE_TRANSFERRING_DATA);

			break;

		case MODE_TRANSFERRING_DATA:

			if (++modeTicks == GB_TRANSFER_DATA_DURATION)
				SwitchMode(MODE_HBLANK);

			break;
	}

	++ticks;
}

void VideoController::SwitchMode(Mode mode)
{
	currentMode = mode;
	modeTicks = 0;

	// Clear the mode bits
	*statRegister &= 0xFC;

	// Write the new mode
	*statRegister |= mode;

	switch (mode)
	{
		case MODE_HBLANK:
			if (READ_BIT(*statRegister, STAT_HBLANK_INTERRUPT))
				cpu.RequestInterrupt(CPU::INT_LCD_STAT);

			break;

		case MODE_VBLANK:
			if (READ_BIT(*statRegister, STAT_VBLANK_INTERRUPT))
				cpu.RequestInterrupt(CPU::INT_LCD_STAT);

			break;

		case MODE_SEARCHING_OAM:
			if (READ_BIT(*statRegister, STAT_SEARCH_OAM_INTERRUPT))
				cpu.RequestInterrupt(CPU::INT_LCD_STAT);

			break;

	}
}

void VideoController::SetScanline(uint8_t scanline)
{
	this->scanline = scanline;

	// Write the new scan line to the LY register
	memory.WriteByte(GB_REG_LY, scanline);

	// If the new scanline matches the value in the LYC register, trigger the STAT interrupt
	if (scanline == memory.ReadByte(GB_REG_LYC))
	{
		if (READ_BIT(*statRegister, STAT_LYC_INTERRUPT))
			cpu.RequestInterrupt(CPU::INT_LCD_STAT);

		*statRegister = SET_BIT(*statRegister, STAT_LYC_COINCEDENCE);
	}
}