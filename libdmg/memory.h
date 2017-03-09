#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "environment.h"

namespace libdmg
{
	class Memory
	{

	public:

	private:
		
		uint8_t* buffer;
	public:

		Memory(uint8_t* buffer);

		uint8_t* RetrievePointer(uint16_t address);
		const uint8_t* RetrievePointer(uint16_t address) const;

		void WriteByte(uint16_t address, uint8_t value);
		void WriteShort(uint16_t address, uint16_t value);
		void WriteBuffer(const uint8_t* buffer, uint16_t startAddress, uint16_t size);

		uint8_t ReadByte(uint16_t address) const;
		uint16_t ReadShort(uint16_t address) const;

		void Read(uint16_t address, uint8_t& value) const;
		void Read(uint16_t address, uint16_t& value) const;

	};
}

#endif