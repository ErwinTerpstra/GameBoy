#ifndef _MEMORY_BUFFER_H_
#define _MEMORY_BUFFER_H_

#include "memorybank.h"

namespace libdmg
{
	class MemoryBuffer : public MemoryBank
	{
	private:

		uint8_t* buffer;

	public:

		MemoryBuffer(uint8_t* buffer) : buffer(buffer) { }

		const uint8_t* RetrievePointer(uint16_t address) const { return buffer + address; }

	};
}

#endif