#ifndef _MEMORY_BANK_H_
#define _MEMORY_BANK_H_

#include "environment.h"

namespace libdmg
{
	class MemoryBank
	{
	public:

	public:

		virtual uint8_t ReadByte(uint16_t address) const = 0;
		virtual void WriteByte(uint16_t address, uint8_t value) = 0;
	};
}

#endif