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
	MemoryBuffer* vramBuffer	= new MemoryBuffer(new uint8_t[0x2000]);
	MemoryBuffer* wramBuffer	= new MemoryBuffer(new uint8_t[0x2000]);

	MemoryBuffer* oamBuffer		= new MemoryBuffer(new uint8_t[0x9F]);
	MemoryBuffer* ioBuffer		= new MemoryBuffer(new uint8_t[0x7F]);
	MemoryBuffer* hramBuffer	= new MemoryBuffer(new uint8_t[0xF1]);
	MemoryBuffer* ieBuffer		= new MemoryBuffer(new uint8_t[0x01]);

	MemoryBuffer* naBuffer		= new MemoryBuffer(new uint8_t[0x5F]);

	banks[BANK_ROM]		= { 0x0000, 0x7FFF, NULL };
	banks[BANK_VRAM]	= { 0x8000, 0x9FFF, vramBuffer };
	banks[BANK_CRAM]	= { 0xA000, 0xBFFF, NULL };
	banks[BANK_WRAM]	= { 0xC000, 0xDFFF, wramBuffer };
	banks[BANK_ECHO]	= { 0xE000, 0xFDFF, wramBuffer };
	banks[BANK_OAM]		= { 0xFE00, 0xFE9F, oamBuffer };
	banks[BANK_NA]		= { 0xFEA0, 0xFEFF, naBuffer };
	banks[BANK_IO]		= { 0xFF00, 0xFF7F, ioBuffer };
	banks[BANK_HRAM]	= { 0xFF80, 0xFFFE, hramBuffer };
	banks[BANK_IE]		= { 0xFFFF, 0xFFFF, ieBuffer };
}

void Memory::BindCatridge(Cartridge& cartridge)
{
	MemoryRange& romRange = banks[BANK_ROM];
	MemoryRange& ramRange = banks[BANK_CRAM];

	if (romRange.bank != NULL)
		delete romRange.bank;

	if (ramRange.bank != NULL)
		delete ramRange.bank;

	switch (cartridge.header->cartridgeHardware)
	{
		case 0:	// 32KB ROM, no RAM
		{
			romRange.bank = new MemoryBuffer(&cartridge.rom[0]);
			ramRange.bank = NULL;
			break;
		}

		case 1: // MBC1
		{
			MBC1* mbc = new MBC1(cartridge);
			romRange.bank = &mbc->rom;
			ramRange.bank = &mbc->ram;
			break;
		}

		default:
			assert(false && "Unsupported cartridge hardware");
			break;
	}
}

uint8_t* Memory::RetrievePointer(uint16_t address)
{
	return const_cast<uint8_t*>(static_cast<const Memory*>(this)->RetrievePointer(address));
}

const uint8_t* Memory::RetrievePointer(uint16_t address) const
{
	uint8_t bankIdx =
		(address >= banks[BANK_VRAM].start) +
		(address >= banks[BANK_CRAM].start) +
		(address >= banks[BANK_WRAM].start) +
		(address >= banks[BANK_ECHO].start) +
		(address >= banks[BANK_OAM].start) +
		(address >= banks[BANK_NA].start) +
		(address >= banks[BANK_IO].start) +
		(address >= banks[BANK_HRAM].start) +
		(address >= banks[BANK_IE].start);

	const MemoryRange& range = banks[bankIdx];

	// If the address is in an unallocated range, return a reference to the null address
	assert(address <= range.end);

	// Make sure the range has a registered memory bank
	assert(range.bank != NULL);
	
	return range.bank->RetrievePointer(address - range.start);
}

void Memory::WriteByte(uint16_t address, uint8_t value)
{
	if (address == GB_REG_DMA)
	{
		uint8_t* srcBuffer = RetrievePointer(value << 8);
		WriteBuffer(srcBuffer, GB_OAM, 0x9F);
	}
	else
		*RetrievePointer(address) = value;

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

	uint8_t* buffer = RetrievePointer(address);

	buffer[0] = value & 0x00ff;
	buffer[1] = (value >> 8) & 0x0ff;
}

void Memory::WriteBuffer(const uint8_t* srcBuffer, uint16_t startAddress, uint16_t size)
{
	uint8_t* dstBuffer = RetrievePointer(startAddress);

	for (uint16_t offset = 0; offset < size; ++offset)
		dstBuffer[offset] = srcBuffer[offset];
}

uint8_t Memory::ReadByte(uint16_t address) const
{
	if (MemoryReadCallback != NULL)
		MemoryReadCallback(address);

	return *RetrievePointer(address);
}

uint16_t Memory::ReadShort(uint16_t address) const
{
	if (MemoryReadCallback != NULL)
	{
		MemoryReadCallback(address);
		MemoryReadCallback(address + 1);
	}

	return DECODE_SHORT(RetrievePointer(address));
}

void Memory::Read(uint16_t address, uint8_t& value) const
{
	value = ReadByte(address);
}

void Memory::Read(uint16_t address, uint16_t& value) const
{
	value = ReadShort(address);
}