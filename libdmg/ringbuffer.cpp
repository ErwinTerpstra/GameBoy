#include "ringbuffer.h"

#include "debug.h"

using namespace libdmg;

RingBuffer::RingBuffer(uint16_t size) : size(size), readOffset(0), writeOffset(0), full(false)
{
	data = new uint8_t[size];
}

RingBuffer::~RingBuffer()
{
	if (data != NULL)
	{
		delete[] data;
		data = NULL;
	}
}

uint8_t RingBuffer::ReadByte()
{
	assert(!Empty());

	uint8_t value = data[readOffset];
	readOffset = (readOffset + 1) % size;

	full = false;

	return value;
}

void RingBuffer::WriteByte(uint8_t value)
{
	data[writeOffset] = value;

	writeOffset = (writeOffset + 1) % size;

	if (full)
		readOffset = (readOffset + 1) % size;
	else
		full = (writeOffset == readOffset);
}