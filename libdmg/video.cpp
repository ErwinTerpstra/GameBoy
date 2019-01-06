#include "video.h"

#include "cpu.h"
#include "memory.h"
#include "memorybank.h"

#include "gameboy.h"

#include "util.h"
#include "debug.h"

using namespace libdmg;

Video::Video(CPU& cpu, Memory& memory, uint8_t* videoBuffer) :
	VBlankCallback(NULL),
	cpu(cpu), memory(memory), videoBuffer(videoBuffer),
	scanline(0), ticks(0), modeTicks(0), currentMode(MODE_VBLANK),
	lcdControlRegister(memory, GB_REG_LCDC), statRegister(memory, GB_REG_STAT),
	paletteRegister(memory, GB_REG_BGP), 
	scanlineRegister(memory, GB_REG_LY), scanlineCompareRegister(memory, GB_REG_LYC)
{
	layerStates[LAYER_BACKGROUND] = true;
	layerStates[LAYER_WINDOW] = true;
	layerStates[LAYER_SPRITES] = true;

	Memory::MemoryRange* memoryRange = memory.FindMemoryRange(GB_VRAM);
	vram = memoryRange->bank;
}

void Video::Reset()
{
	ticks = 0;
	modeTicks = 0;
	scanline = 0;
	currentMode = MODE_VBLANK;
}

void Video::Sync(const uint64_t& targetTicks)
{
	while (ticks < targetTicks)
		Step();
}

void Video::Step()
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

void Video::DrawTileset()
{
	uint16_t tileAddress = GB_TILE_DATA_0;
	uint8_t tileBuffer[GB_TILE_SIZE];

	memset(videoBuffer, 0, GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT / 4);

	for (uint16_t tileIdx = 0; tileIdx < 256; ++tileIdx)
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

uint8_t Video::GetPixel(uint8_t x, uint8_t y)
{
	// Calculate the address of the pixel
	uint16_t pixel = (y *  GB_SCREEN_WIDTH) + x;
	uint16_t pixelAddress = pixel / 4;

	// Retrieve the curent video byte containing the pixel
	uint8_t videoByte = videoBuffer[pixelAddress];
	uint8_t videoBit = (pixel % 4) << 1;

	return (videoByte >> videoBit) & 0x03;
}

void Video::SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
	// Calculate the address of the pixel
	uint16_t pixel = (y *  GB_SCREEN_WIDTH) + x;
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

void Video::DrawLine()
{
	if (READ_BIT(*lcdControlRegister, LCDC_BG_ENABLE))
	{
		// Draw BG
		if (layerStates[LAYER_BACKGROUND])
		{
			uint16_t bgMapAddress = READ_BIT(*lcdControlRegister, LCDC_BG_MAP_SELECT) ? GB_BG_MAP_1 : GB_BG_MAP_0;
			uint16_t bgTileDataAddresss = READ_BIT(*lcdControlRegister, LCDC_BG_DATA_SELECT) ? GB_TILE_DATA_0 : GB_TILE_DATA_2;

			uint8_t scrollX = memory.ReadByte(GB_REG_SCX);
			uint8_t scrollY = memory.ReadByte(GB_REG_SCY);

			DrawMap(0, 0, bgMapAddress, bgTileDataAddresss, *paletteRegister, scrollX, scrollY);
		}

		// Draw window
		if (layerStates[LAYER_WINDOW] && READ_BIT(*lcdControlRegister, LCDC_WINDOW_ENABLE))
		{
			uint16_t windowMapAddress = READ_BIT(*lcdControlRegister, LCDC_WINDOW_MAP_SELECT) ? GB_BG_MAP_1 : GB_BG_MAP_0;
			uint16_t windowTileDataAddresss = READ_BIT(*lcdControlRegister, LCDC_BG_DATA_SELECT) ? GB_TILE_DATA_0 : GB_TILE_DATA_2;

			uint8_t offsetX = memory.ReadByte(GB_REG_WX) - 7;
			uint8_t offsetY = memory.ReadByte(GB_REG_WY);

			if (offsetX < GB_SCREEN_WIDTH && offsetY < GB_SCREEN_HEIGHT && scanline >= offsetY)
				DrawMap(offsetX, offsetY, windowMapAddress, windowTileDataAddresss, *paletteRegister, 0, 0);
		}
	}
	else
		memset(videoBuffer + scanline * GB_SCREEN_WIDTH / 4, 0x00, GB_SCREEN_WIDTH / 4);

	if (layerStates[LAYER_SPRITES] && READ_BIT(*lcdControlRegister, LCDC_SPRITE_ENABLE))
		DrawSprites();
}

void Video::DrawMap(uint8_t offsetX, uint8_t offsetY, uint16_t mapAddress, uint16_t tileDataAddress, uint8_t palette, uint8_t scrollX, uint8_t scrollY)
{
	uint8_t mapY = ((scrollY + scanline - offsetY) % GB_BG_HEIGHT);
	uint8_t tileY = mapY / GB_TILE_HEIGHT;
	uint8_t tileLocalY = mapY % GB_TILE_HEIGHT;

	bool signedTileIndices = tileDataAddress == GB_TILE_DATA_2;

	for (uint8_t x = offsetX; x < GB_SCREEN_WIDTH; ++x)
	{
		uint8_t mapX = ((scrollX + x - offsetX) % GB_BG_WIDTH);
		uint8_t tileX = mapX / GB_TILE_WIDTH;

		// Read the index of the tile to use for this pixel
		uint8_t tileIdx = vram->ReadByte(mapAddress - GB_VRAM + tileY * (GB_BG_WIDTH / GB_TILE_WIDTH) + tileX);
	
		uint8_t tileLocalX = mapX % GB_TILE_WIDTH;

		uint16_t tileAddress = tileDataAddress; 
		
		if (signedTileIndices)
			tileAddress += GB_TILE_SIZE * DECODE_SIGNED_BYTE(&tileIdx);
		else
			tileAddress += GB_TILE_SIZE * tileIdx;

		uint8_t tilePixel;
		DecodeTile(tileAddress, tileLocalY, tileLocalX, tilePixel);

		// Retrieve the color from the palette
		uint8_t color = (palette >> (tilePixel << 1)) & 0x03;
		
		SetPixel(x, scanline, color);
	}
}

void Video::DrawSprites()
{
	uint8_t spriteBuffer[sizeof(Sprite)];
	const Sprite* sprite = (Sprite*)&spriteBuffer[0];

	bool doubleSize = READ_BIT(*lcdControlRegister, LCDC_SPRITE_SIZE);
	uint8_t height = GB_TILE_HEIGHT << (doubleSize ? 1 : 0);

	for (uint8_t spriteIdx = 0; spriteIdx < GB_MAX_SPRITES; ++spriteIdx)
	{
		memory.ReadBuffer(&spriteBuffer[0], GB_OAM + spriteIdx * sizeof(Sprite), sizeof(Sprite));

		int16_t minY = sprite->y - (GB_TILE_HEIGHT << 1);
		int16_t maxY = minY + height - 1;

		// Test if this sprite overlaps this scanline
		if (scanline < minY || scanline > maxY)
			continue;

		// Read sprite attribute flags
		bool behindBackground = READ_BIT(sprite->flags, SPRITE_ORDER);
		bool flipX = READ_BIT(sprite->flags, SPRITE_FLIP_X);
		bool flipY = READ_BIT(sprite->flags, SPRITE_FLIP_Y);

		// Find out the local tile Y
		uint8_t tileY;
		if (flipY)
			tileY = maxY - scanline;
		else
			tileY = scanline - minY;

		// Select the correct tile index
		uint8_t tileIdx = sprite->tileIdx;
		if (doubleSize)
		{
			if (tileY >= GB_TILE_HEIGHT)
				tileIdx |= 0x01;
			else
				tileIdx &= 0xFE;
		}

		// Read the tile data for this sprite
		uint8_t tileBuffer[2];
		DecodeTile(GB_TILE_DATA_0 + GB_TILE_SIZE * tileIdx, tileY, tileBuffer);

		// Read the palette to use
		uint8_t palette = memory.ReadByte(READ_BIT(sprite->flags, SPRITE_PALETTE) ? GB_REG_OBP1 : GB_REG_OBP0);

		for (uint8_t tileX = 0; tileX < GB_TILE_WIDTH; ++tileX)
		{
			int16_t x = sprite->x - GB_TILE_WIDTH + tileX;

			if (x < 0 || x >= GB_SCREEN_WIDTH)
				continue;

			// Check if we need to render the sprite above or below the background
			uint8_t color;

			if (behindBackground)
			{
				color = GetPixel((uint8_t) x, scanline);

				if (color > 0)
					continue;
			}

			// Read the tile data byte
			uint8_t tilePixelOffset = flipX ? (GB_TILE_WIDTH - 1 - tileX) : tileX;
			uint8_t tileData = tileBuffer[tilePixelOffset / 4];
			uint8_t tileBit = (tilePixelOffset % 4) << 1;

			// Retrieve the 2 bit palette color index from the 4 pixel byte
			uint8_t paletteColorIdx = (tileData >> tileBit) & 0x03;

			if (paletteColorIdx == 0)
				continue;

			// Retrieve the color from the palette
			color = (palette >> (paletteColorIdx << 1)) & 0x03;

			SetPixel((uint8_t) x, scanline, color);
		}
	}
}

void Video::DecodeTile(uint16_t tileAddress, uint8_t* tileBuffer)
{
	for (uint8_t tileY = 0; tileY < GB_TILE_HEIGHT; ++tileY)
		DecodeTile(tileAddress, tileY, tileBuffer + tileY * 2);
}

void Video::DecodeTile(uint16_t tileAddress, uint8_t tileY, uint8_t* tileBuffer)
{
	tileAddress = tileAddress - GB_VRAM + (tileY << 1);
	uint8_t lowerByte = vram->ReadByte(tileAddress + 0);
	uint8_t upperByte = vram->ReadByte(tileAddress + 1);

	uint8_t leftByte = 0;
	uint8_t rightByte = 0;

	for (uint8_t pixel = 0; pixel < 4; ++pixel)
	{
		leftByte |= READ_BIT(lowerByte, 7 - pixel) << ((pixel << 1) + 0);
		leftByte |= READ_BIT(upperByte, 7 - pixel) << ((pixel << 1) + 1);

		rightByte |= READ_BIT(lowerByte, 3 - pixel) << ((pixel << 1) + 0);
		rightByte |= READ_BIT(upperByte, 3 - pixel) << ((pixel << 1) + 1);
	}

	tileBuffer[0] = leftByte;
	tileBuffer[1] = rightByte;
}
void Video::DecodeTile(uint16_t tileAddress, uint8_t tileY, uint8_t tileX, uint8_t& tilePixel)
{
	tileAddress = tileAddress - GB_VRAM + (tileY << 1);
	uint8_t lowerByte = vram->ReadByte(tileAddress + 0);
	uint8_t upperByte = vram->ReadByte(tileAddress + 1);

	tilePixel = (READ_BIT(upperByte, 7 - tileX) << 1) | (READ_BIT(lowerByte, 7 - tileX));
}

void Video::SwitchMode(Mode mode)
{
	currentMode = mode;
	modeTicks = 0;

	// Clear the mode bits
	statRegister = (*statRegister & 0xFC);

	// Write the new mode
	statRegister = (*statRegister | mode);

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

void Video::SetScanline(uint8_t scanline)
{
	this->scanline = scanline;

	// Write the new scan line to the LY register
	scanlineRegister = scanline;

	// If the new scanline matches the value in the LYC register, trigger the STAT interrupt
	if (scanline == *scanlineCompareRegister)
	{
		if (READ_BIT(*statRegister, STAT_LYC_INTERRUPT))
			cpu.RequestInterrupt(CPU::INT_LCD_STAT);

		statRegister = SET_BIT(*statRegister, STAT_LYC_COINCEDENCE);
	}
}