#ifndef _MEMORY_BUFFER_H_
#define _MEMORY_BUFFER_H_

#include "memorybank.h"

namespace libdmg
{
	class MemoryBuffer : public MemoryBank
	{
	private:

		uint8_t* buffer;
		bool external;

	public:

		MemoryBuffer(uint16_t size) : external(false)
		{ 
			buffer = new uint8_t[size];
		}

		MemoryBuffer(uint8_t* buffer) : buffer(buffer), external(true)
		{

		}

		~MemoryBuffer()
		{
			if (!external && buffer != NULL)
				delete[] buffer;
		}

		DMG_FORCE_INLINE uint8_t ReadByte(uint16_t address) const { return buffer[address]; }
		DMG_FORCE_INLINE void WriteByte(uint16_t address, uint8_t value) { WRITE_BYTE(buffer + address, value); }


	};
}

#endif