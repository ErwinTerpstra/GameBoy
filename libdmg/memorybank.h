#ifndef _MEMORY_BANK_H_
#define _MEMORY_BANK_H_

#include "environment.h"

namespace libdmg
{
	class MemoryBank
	{
	public:

	public:

		uint8_t* RetrievePointer(uint16_t address)
		{
			return const_cast<uint8_t*>(static_cast<const MemoryBank*>(this)->RetrievePointer(address));
		}
		
		virtual const uint8_t* RetrievePointer(uint16_t address) const = 0;

		virtual uint8_t ReadByte(uint16_t address) const = 0;
		virtual uint16_t ReadShort(uint16_t address) const = 0;
		
		virtual void WriteByte(uint16_t address, uint8_t value) = 0;
		virtual void WriteShort(uint16_t address, uint16_t value) = 0;
	};
}

#endif