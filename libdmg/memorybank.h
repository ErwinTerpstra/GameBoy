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
	};
}

#endif