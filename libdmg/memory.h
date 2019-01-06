#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "environment.h"

#include "memorybank.h"
#include "memorypointer.h"

namespace libdmg
{
	class Cartridge;
	class MBC;

	class Memory
	{
	public:
		static const uint8_t MEMORY_BANK_COUNT = 14;

		struct MemoryRange
		{
			uint16_t start, end;
			MemoryBank* bank;
		};

		void (*MemoryWriteCallback)(uint16_t address);
		void (*MemoryReadCallback)(uint16_t address);

		MBC* mbc;

	private:
		
		MemoryRange* banks;


	public:

		Memory();
		~Memory();

		void BindIO(MemoryBank* input, MemoryBank* sound1, MemoryBank* sound2);
		void BindCartridge(Cartridge& cartridge);

		MemoryPointer RetrievePointer(uint16_t address)
		{
			return MemoryPointer(*this, address);
		}

		void WriteByte(uint16_t address, uint8_t value);
		void WriteShort(uint16_t address, uint16_t value);
		void WriteBuffer(const uint8_t* srcBuffer, uint16_t startAddress, uint16_t size);
		
		void Copy(uint16_t srcAddress, uint16_t dstAddress, uint16_t size);

		uint8_t ReadByte(uint16_t address) const;
		uint16_t ReadShort(uint16_t address) const;

		void Read(uint16_t address, uint8_t& value) const;
		void Read(uint16_t address, uint16_t& value) const;
		
		void ReadBuffer(uint8_t* buffer, uint16_t address, uint16_t length) const;

		MemoryRange* FindMemoryRange(uint16_t address)
		{
			return const_cast<MemoryRange*>(static_cast<const Memory*>(this)->FindMemoryRange(address)); 
		}

		const MemoryRange* FindMemoryRange(uint16_t address) const;
	};
}

#endif