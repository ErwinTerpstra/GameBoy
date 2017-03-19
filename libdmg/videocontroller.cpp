#include "videocontroller.h"

#include "cpu.h"
#include "memory.h"

#include "gameboy.h"

#include "util.h"
#include "debug.h"

using namespace libdmg;

VideoController::VideoController(CPU& cpu, Memory& memory, uint8_t* videoBuffer) : 
	VBlankCallback(NULL),
	cpu(cpu), memory(memory), videoBuffer(videoBuffer), 
	scanline(0), ticks(0), modeTicks(0), currentMode(MODE_VBLANK)
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
			{
				DrawLine();
				SwitchMode(MODE_HBLANK);
			}

			break;
	}

	++ticks;
}

void VideoController::DrawTileset()
{
	uint16_t tileAddress = GB_TILE_DATA_0;
	uint8_t tileBuffer[GB_TILE_SIZE];

	memset(videoBuffer, 0, GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT / 4);

	for (uint16_t tileIdx = 0; tileIdx < 192; ++tileIdx)
	{
		uint16_t gridX = (tileIdx % 20) * GB_TILE_WIDTH;
		uint16_t gridY = (tileIdx / 20) * GB_TILE_HEIGHT;

		DecodeTile(tileAddress + tileIdx * GB_TILE_SIZE, tileBuffer);

		for (uint8_t tileY = 0; tileY < GB_TILE_HEIGHT; ++tileY)
		{
			videoBuffer[(((gridY + tileY) * GB_SCREEN_WIDTH) + gridX) / 4 + 0] = tileBuffer[tileY * 2 + 0];
			videoBuffer[(((gridY + tileY) * GB_SCREEN_WIDTH) + gridX) / 4 + 1] = tileBuffer[tileY * 2 + 1];
		}
	}

}

void VideoController::DrawLine()
{
	uint16_t bgMapAddress = READ_BIT(*lcdControlRegister, LCDC_BG_MAP_SELECT) ? GB_BG_MAP_1 : GB_BG_MAP_0;
	uint16_t bgTileDataAddresss = READ_BIT(*lcdControlRegister, LCDC_BG_DATA_SELECT) ? GB_TILE_DATA_0 : GB_TILE_DATA_1;
	uint8_t bgPalette = memory.ReadByte(GB_REG_BGP);

	uint8_t scrollX = memory.ReadByte(GB_REG_SCX);
	uint8_t scrollY = memory.ReadByte(GB_REG_SCY);

	uint8_t mapY = ((scrollY + scanline) % GB_BG_HEIGHT);
	uint8_t tileY = mapY / GB_TILE_HEIGHT;
	uint8_t tileLocalY = mapY % GB_TILE_HEIGHT;

	uint8_t tileBuffer[GB_TILE_SIZE];

	for (uint8_t x = 0; x < GB_SCREEN_WIDTH; ++x)
	{
		uint8_t mapX = ((scrollX + x) % GB_BG_WIDTH);
		uint8_t tileX = mapX / GB_TILE_WIDTH;

		// Read the index of the tile to use for this pixel
		uint8_t tileIdx = memory.ReadByte(bgMapAddress + tileY * (GB_BG_WIDTH / GB_TILE_WIDTH) + tileX);
		
		uint8_t tileLocalX = mapX % GB_TILE_WIDTH;

		uint16_t tileAddress = bgTileDataAddresss + GB_TILE_SIZE * tileIdx;
		DecodeTile(tileAddress, tileBuffer);

		// Read the tile data byte
		uint8_t tilePixelOffset = (tileLocalY * GB_TILE_WIDTH) + tileLocalX;
		uint8_t tileData = tileBuffer[tilePixelOffset / 4];
		uint8_t tileBit = (tilePixelOffset % 4) << 1;

		// Retrieve the palette color index from the 4 pixel byte
		uint8_t paletteColorIdx = (tileData >> tileBit) & 0x03;

		// Retrieve the color from the palette
		uint8_t color = (bgPalette >> (paletteColorIdx << 1)) & 0x03;
		
		// Calculate the address of the pixel
		uint16_t pixel = (scanline *  GB_SCREEN_WIDTH) + x;
		uint16_t pixelAddress = pixel / 4;
		
		// Retrieve the curent video byte containing the pixel
		uint8_t videoByte = videoBuffer[pixelAddress];
		uint8_t videoBit = (pixel % 4) << 1;

		// Clear the previous color
		videoByte &= ~(0x03 << videoBit);

		// Write the new color
		videoByte |= color << videoBit;
		videoBuffer[pixelAddress] = videoByte;
	}
}

void VideoController::DecodeTile(uint16_t tileAddress, uint8_t* tileBuffer)
{
	for (uint8_t tileY = 0; tileY < GB_TILE_HEIGHT; ++tileY)
	{
		uint8_t lowerByte = memory.ReadByte(tileAddress + (tileY << 1) + 0);
		uint8_t upperByte = memory.ReadByte(tileAddress + (tileY << 1) + 1);

		uint8_t leftByte = 0;
		uint8_t rightByte = 0;

		for (uint8_t pixel = 0; pixel < 4; ++pixel)
		{
			leftByte |= READ_BIT(lowerByte, 7 - pixel) << ((pixel << 1) + 0);
			leftByte |= READ_BIT(upperByte, 7 - pixel) << ((pixel << 1) + 1);
		}

		for (uint8_t pixel = 0; pixel < 4; ++pixel)
		{
			rightByte |= READ_BIT(lowerByte, 3 - pixel) << ((pixel << 1) + 0);
			rightByte |= READ_BIT(upperByte, 3 - pixel) << ((pixel << 1) + 1);
		}

		tileBuffer[tileY * 2 + 0] = leftByte;
		tileBuffer[tileY * 2 + 1] = rightByte;
	}

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
			if (VBlankCallback != NULL)
				VBlankCallback();

			cpu.RequestInterrupt(CPU::INT_VBLANK);

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