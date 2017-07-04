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

		const uint8_t* RetrievePointer(uint16_t address) const { return buffer + address;  }

		uint8_t ReadByte(uint16_t address) const { return buffer[address]; }
		uint16_t ReadShort(uint16_t address) const { return DECODE_SHORT(buffer + address); }

		void WriteByte(uint16_t address, uint8_t value) { WRITE_BYTE(buffer + address, value); }
		void WriteShort(uint16_t address, uint16_t value) { WRITE_SHORT(buffer + address, value); }


	};
}

#endif