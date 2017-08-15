#include "memorypointer.h"

#include "memory.h"
#include "memorybank.h"

using namespace libdmg;

NativePointer::NativePointer(uint8_t* ptr) : ptr(ptr)
{

}

uint8_t NativePointer::Read()
{
	return *ptr; 
}

void NativePointer::Write(uint8_t value)
{
	*ptr = value; 
}

MemoryPointer::MemoryPointer(Memory& memory, uint16_t address)
{
	Memory::MemoryRange* range = memory.FindMemoryRange(address);
	memoryBank = range->bank;

	localAddress = address - range->start;
}

uint8_t MemoryPointer::Read()
{
	return memoryBank->ReadByte(localAddress);
}

void MemoryPointer::Write(uint8_t value)
{
	memoryBank->WriteByte(localAddress, value);
}