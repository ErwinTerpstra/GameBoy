#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "environment.h"

#include "memorybank.h"

namespace libdmg
{
	class Cartridge;

	class Memory
	{

	public:
		enum MemoryBanks
		{
			BANK_ROM,
			BANK_VRAM,
			BANK_CRAM,
			BANK_WRAM,
			BANK_ECHO,
			BANK_OAM,
			BANK_NA,
			BANK_IO,
			BANK_HRAM,
			BANK_IE,

			BANK_COUNT
		};

		struct MemoryRange
		{
			uint16_t start, end;
			MemoryBank* bank;
		};

		void (*MemoryWriteCallback)(uint16_t address);
		void (*MemoryReadCallback)(uint16_t address);

	private:
		
		MemoryRange banks[BANK_COUNT];

	public:

		Memory();

		void BindCatridge(Cartridge& cartridge);

		uint8_t* RetrievePointer(uint16_t address)
		{
			return const_cast<uint8_t*>(static_cast<const Memory*>(this)->RetrievePointer(address));
		}

		const uint8_t* RetrievePointer(uint16_t address) const;

		void WriteByte(uint16_t address, uint8_t value);
		void WriteShort(uint16_t address, uint16_t value);
		void WriteBuffer(const uint8_t* srcBuffer, uint16_t startAddress, uint16_t size);
		
		void Copy(uint16_t srcAddress, uint16_t dstAddress, uint16_t size);

		uint8_t ReadByte(uint16_t address) const;
		uint16_t ReadShort(uint16_t address) const;

		void Read(uint16_t address, uint8_t& value) const;
		void Read(uint16_t address, uint16_t& value) const;

	private:
		MemoryRange* FindMemoryRange(uint16_t address)
		{
			return const_cast<MemoryRange*>(static_cast<const Memory*>(this)->FindMemoryRange(address)); 
		}

		const MemoryRange* FindMemoryRange(uint16_t address) const;
	};
}

#endif