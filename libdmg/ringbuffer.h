#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include "environment.h"

namespace libdmg
{
	class RingBuffer
	{
	
	private:

		uint8_t* data;
		uint16_t size;
		uint16_t readOffset, writeOffset;

		bool full;

	public:
		RingBuffer(uint16_t size);
		~RingBuffer();

		uint8_t ReadByte();
		void WriteByte(uint8_t value);

		bool Empty() const { return !full && (writeOffset == readOffset); }
		bool Full() const { return full; }

		uint16_t Size() const { return size; }
		uint16_t Length() const { return full ? size : (writeOffset - readOffset); }
	};
}

#endif