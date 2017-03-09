#include "memory.h"

#include "util.h"

using namespace libdmg;

Memory::Memory(uint8_t* buffer) : buffer(buffer)
{
	WriteByte(0xFFFFF, (uint8_t) 0);

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
	buffer[address] = value;
}

void Memory::WriteShort(uint16_t address, uint16_t value)
{
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