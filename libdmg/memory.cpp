#include "memory.h"

#include "util.h"
#include "debug.h"

#include "gameboy.h"

#include "memorybuffer.h"
#include "mbc1.h"

#include "cartridge.h"

using namespace libdmg;

uint16_t null;

Memory::Memory() : MemoryWriteCallback(NULL)
{
	MemoryBuffer* vramBuffer	= new MemoryBuffer(0x2000);
	MemoryBuffer* wramBuffer	= new MemoryBuffer(0x2000);

	MemoryBuffer* oamBuffer		= new MemoryBuffer(0x9F);
	MemoryBuffer* hramBuffer	= new MemoryBuffer(0xF1);

	MemoryBuffer* naBuffer		= new MemoryBuffer(0x5F);

	banks = new MemoryRange[MEMORY_BANK_COUNT];

	uint8_t currentBank = 0;
	banks[currentBank++] = { 0x0000, 0x7FFF, NULL };
	banks[currentBank++] = { 0x8000, 0x9FFF, vramBuffer };
	banks[currentBank++] = { 0xA000, 0xBFFF, NULL };
	banks[currentBank++] = { 0xC000, 0xDFFF, wramBuffer };
	banks[currentBank++] = { 0xE000, 0xFDFF, wramBuffer };
	banks[currentBank++] = { 0xFE00, 0xFE9F, oamBuffer };
	banks[currentBank++] = { 0xFEA0, 0xFEFF, naBuffer };
	banks[currentBank++] = { 0xFF00, 0xFF00, NULL };
	banks[currentBank++] = { 0xFF01, 0xFF0F, new MemoryBuffer(0x09) };
	banks[currentBank++] = { 0xFF10, 0xFF14, NULL };
	banks[currentBank++] = { 0xFF15, 0xFF19, NULL };
	banks[currentBank++] = { 0xFF1A, 0xFF7F, new MemoryBuffer(0x65) };
	banks[currentBank++] = { 0xFF80, 0xFFFE, hramBuffer };
	banks[currentBank++] = { 0xFFFF, 0xFFFF, new MemoryBuffer(0x01) };

	assert(currentBank == MEMORY_BANK_COUNT);
}

Memory::~Memory()
{
	if (banks != NULL)
	{
		delete[] banks;
		banks = NULL;
	}
}

void Memory::BindIO(MemoryBank* input, MemoryBank* sound1, MemoryBank* sound2)
{
	FindMemoryRange(GB_REG_JOYP)->bank = input;
	FindMemoryRange(GB_REG_NR10)->bank = sound1;
	FindMemoryRange(GB_REG_NR21)->bank = sound2;
}

void Memory::BindCatridge(Cartridge& cartridge)
{
	MemoryRange* romRange = FindMemoryRange(GB_ROM);
	MemoryRange* ramRange = FindMemoryRange(GB_CRAM);

	// TODO: make sure memory from previous cartridge is free'd

	switch (cartridge.header->cartridgeHardware)
	{
		case 0:
		{
			printf("[Memory]: 32KB ROM, no RAM\n");

			romRange->bank = new MemoryBuffer(&cartridge.rom[0]);
			ramRange->bank = NULL;
			break;
		}

		// MBC1
		case 1: 
		case 2: 
		case 3:
		{
			printf("[Memory]: MBC1: 2MB ROM 32KB RAM\n");
			MBC1* mbc = new MBC1(cartridge);
			romRange->bank = &mbc->rom;
			ramRange->bank = &mbc->ram;
			break;
		}

		default:
			assert(false && "Unsupported cartridge hardware");
			break;
	}
}

const Memory::MemoryRange* Memory::FindMemoryRange(uint16_t address) const
{
	uint8_t bankIdx = 0;
	while (banks[bankIdx].end < address)
		++bankIdx;

	const MemoryRange* range = &banks[bankIdx];

	// Make sure the address is in an allocated range
	assert(address >= range->start);

	// Make sure the range has a registered memory bank
	//assert(range->bank != NULL);

	return range;
}

void Memory::WriteByte(uint16_t address, uint8_t value)
{
	if (MemoryWriteCallback != NULL)
		MemoryWriteCallback(address);

	if (address == GB_REG_DMA)
	{
		Copy(value << 8, GB_OAM, 0x9F);
	}
	else
	{
		MemoryRange* range = FindMemoryRange(address);
		range->bank->WriteByte(address - range->start, value);
	}
}

void Memory::WriteShort(uint16_t address, uint16_t value)
{
	if (MemoryWriteCallback != NULL)
	{
		MemoryWriteCallback(address);
		MemoryWriteCallback(address + 1);
	}

	MemoryRange* range = FindMemoryRange(address);
	range->bank->WriteByte(address - range->start + 0, value & 0xFF);
	range->bank->WriteByte(address - range->start + 1, value >> 8);
}

void Memory::WriteBuffer(const uint8_t* srcBuffer, uint16_t startAddress, uint16_t size)
{
	for (uint16_t offset = 0; offset < size; ++offset)
		WriteByte(startAddress + offset, srcBuffer[offset]);
}

void Memory::Copy(uint16_t srcAddress, uint16_t dstAddress, uint16_t size)
{
	for (uint16_t offset = 0; offset < size; ++offset)
		WriteByte(dstAddress + offset, ReadByte(srcAddress + offset));
}

uint8_t Memory::ReadByte(uint16_t address) const
{
	if (MemoryReadCallback != NULL)
		MemoryReadCallback(address);

	const MemoryRange* range = FindMemoryRange(address);
	return range->bank->ReadByte(address - range->start);
}

uint16_t Memory::ReadShort(uint16_t address) const
{
	if (MemoryReadCallback != NULL)
	{
		MemoryReadCallback(address);
		MemoryReadCallback(address + 1);
	}

	const MemoryRange* range = FindMemoryRange(address);
	uint16_t result;
	result = range->bank->ReadByte(address - range->start + 0);
	result |= range->bank->ReadByte(address - range->start + 1) << 8;

	return result;
}

void Memory::Read(uint16_t address, uint8_t& value) const
{
	value = ReadByte(address);
}

void Memory::Read(uint16_t address, uint16_t& value) const
{
	value = ReadShort(address);
}

void Memory::ReadBuffer(uint8_t* buffer, uint16_t address, uint16_t length) const
{
	for (uint8_t offset = 0; offset < length; ++offset)
		buffer[offset] = ReadByte(address + offset);
}