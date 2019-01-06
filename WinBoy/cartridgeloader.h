#ifndef _CARTRIDGE_LOADER_H_
#define _CARTRIDGE_LOADER_H

#include <string>

namespace WinBoy
{
	class CartridgeLoader
	{
	private:
		std::string romFileName;
		std::string saveFileName;

		uint8_t* romBuffer;
		uint8_t* cramBuffer;

		bool hasSaveFile;

		size_t romSize;
		size_t cramSize;

	public:
		CartridgeLoader();
		~CartridgeLoader();

		bool LoadFile(const char* fileName);

		bool SaveCRam(size_t size);

		bool HasSaveFile() const { return hasSaveFile; }

		uint8_t* RomBuffer() { return romBuffer; }
		uint8_t* CRamBuffer() { return cramBuffer; }
		const size_t RomSize() const { return romSize; }
		const size_t CRamSize() const { return cramSize; }

	private:
		bool ReadFile(const char* fileName, uint8_t* buffer, uint32_t bufferSize, size_t& readBytes);
		bool WriteFile(const char* fileName, uint8_t* buffer, uint32_t bufferSize);
	};

}

#endif