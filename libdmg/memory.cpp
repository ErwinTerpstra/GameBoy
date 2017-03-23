#include "memory.h"

#include "util.h"
#include "debug.h"

#include "gameboy.h"

using namespace libdmg;

Memory::Memory(uint8_t* buffer) : buffer(buffer), MemoryWriteCallback(NULL)
{

}

uint8_t* Memory::RetrievePointer(uint16_t address)
{
	return buffer + address;
}

const uint8_t* Memory::RetrievePointer(uint16_t address) const
{
	return buffer + address;
}

void Memory::WriteByte(uint16_t address, uint8_t value)
{
	if (address == GB_REG_DMA)
	{
		uint16_t source = value << 8;
		for (uint8_t offset = 0; offset < 0x9F; ++offset)
			buffer[GB_OAM + offset] = buffer[source + offset];

		return;
	}

	buffer[address] = value;

	if (MemoryWriteCallback != NULL)
		MemoryWriteCallback(address);

}

void Memory::WriteShort(uint16_t address, uint16_t value)
{
	if (MemoryWriteCallback != NULL)
	{
		MemoryWriteCallback(address);
		MemoryWriteCallback(address + 1);
	}

	buffer[address] = value & 0x00ff;
	buffer[address + 1] = (value >> 8) & 0x0ff;
}

void Memory::WriteBuffer(const uint8_t* buffer, uint16_t startAddress, uint16_t size)
{
	for (uint16_t offset = 0; offset < size; ++offset)
		this->buffer[startAddress + offset] = buffer[offset];
}

uint8_t Memory::ReadByte(uint16_t address) const
{
	return buffer[address];
}

uint16_t Memory::ReadShort(uint16_t address) const
{
	return DECODE_SHORT(buffer + address);
}

void Memory::Read(uint16_t address, uint8_t& value) const
{
	value = ReadByte(address);
}

void Memory::Read(uint16_t address, uint16_t& value) const
{
	value = ReadShort(address);
}