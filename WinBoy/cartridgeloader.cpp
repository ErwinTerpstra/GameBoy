#include "stdafx.h"

#include "libdmg.h"

#include "cartridgeloader.h"
#include "debug.h"

#include <string>

using namespace WinBoy;
using namespace libdmg;

CartridgeLoader::CartridgeLoader() : hasSaveFile(false)
{
	romBuffer = new uint8_t[GB_MAX_CARTRIDGE_SIZE];
	cramBuffer = new uint8_t[GB_MAX_CARTRIDGE_RAM_SIZE];

	// According to SameBoy source, uninitialized MBC RAM should be 0xFF
	memset(romBuffer, 0x00, GB_MAX_CARTRIDGE_SIZE);
	memset(cramBuffer, 0xFF, GB_MAX_CARTRIDGE_RAM_SIZE);
}

CartridgeLoader::~CartridgeLoader()
{
	delete[] romBuffer;
	delete[] cramBuffer;
}

bool CartridgeLoader::LoadFile(const char* fileName)
{
	romFileName = std::string(fileName);

	// Read the ROM file
	if (!ReadFile(romFileName.c_str(), romBuffer, GB_MAX_CARTRIDGE_SIZE, romSize))
	{
		Debug::Print("[WinBoy]: Failed to read ROM file: %s!\n", fileName);
		return false;
	}

	Debug::Print("[WinBoy]: Rom file read with %uKB.\n", romSize >> 10);

	// Attempt to read the save file
	size_t extensionIdx = romFileName.find_last_of('.');
	saveFileName = romFileName.substr(0, extensionIdx) + ".sav";

	hasSaveFile = ReadFile(saveFileName.c_str(), cramBuffer, GB_MAX_CARTRIDGE_RAM_SIZE, cramSize);

	if (hasSaveFile)
		Debug::Print("[CartridgeLoader]: Save file read with %uKB.\n", cramSize >> 10);
	else
		Debug::Print("[CartridgeLoader]: Save file not found.\n");

	return true;
}

bool CartridgeLoader::SaveCRam(size_t size)
{
	cramSize = size;
	return WriteFile(saveFileName.c_str(), cramBuffer, size);
}

bool CartridgeLoader::ReadFile(const char* fileName, uint8_t* buffer, uint32_t bufferSize, size_t& readBytes)
{
	// Open a file handle
	FILE* handle;
	errno_t error = fopen_s(&handle, fileName, "rb");

	if (error != 0)
		return false;

	// Read the file contents
	readBytes = fread(buffer, 1, bufferSize, handle);

	fclose(handle);

	return true;
}

bool CartridgeLoader::WriteFile(const char* fileName, uint8_t* buffer, uint32_t bufferSize)
{
	// Open a file handle
	FILE* handle;
	errno_t error = fopen_s(&handle, fileName, "wb");

	if (error != 0)
		return false;

	// Write the buffer to the file
	size_t writtenBytes = fwrite(buffer, 1, bufferSize, handle);

	fclose(handle);

	return true;
}