#include "mbc.h"

#include "debug.h"
#include "util.h"

using namespace libdmg;

MBC::MBC(MBC::Type type, Cartridge& cartridge) :
	type(type), cartridge(cartridge), 
	rom(*this), ram(*this),
	ramEnabled(false), ramBankMode(false), 
	selectedROMBank(1), selectedRAMBank(0)
{

}

MBC::ROM::ROM(MBC& mbc) : mbc(mbc)
{

}

void MBC::WriteRegister(uint16_t address, uint8_t value)
{
	// TODO: replace by function pointer table
	switch (type)
	{
	case MBC1:
		WriteRegisterMBC1(address, value);
		break;

	case MBC3:
		WriteRegisterMBC3(address, value);
		break;

	case MBC5:
		WriteRegisterMBC5(address, value);
		break;
	}
}

void MBC::WriteRegisterMBC1(uint16_t address, uint8_t value)
{
	switch (address & 0xF000)
	{
	case 0x0000:
	case 0x1000:
		// RAM enable
		ramEnabled = (value & 0xF) == 0xA;
		break;

	case 0x2000:
	case 0x3000:
		// ROM bank select, lower 5 bits only
		value &= 0x1F;

		// Cannot select bank 0x00, select 0x01 instead
		if (value == 0x00)
			value = 0x01;

		// Clear the lower 5 bits of the current bank and set the new bits
		selectedROMBank = (selectedROMBank & ~0x1F) | value;
		break;

	case 0x4000:
	case 0x5000:
		// RAM bank/upper ROM bank select, lower 2 bits only
		value &= 0x03;

		if (ramBankMode)
		{
			selectedRAMBank = value;
		}
		else
		{
			// Use value as bits 5 & 6 of selected ROM bank
			selectedROMBank = (selectedROMBank & ~0x1F) | (value << 5);
		}

		break;

	case 0x6000:
	case 0x7000:
		// ROM/RAM mode switch
		if (value)
		{
			selectedROMBank &= 0x1F;
			ramBankMode = true;
		}
		else
		{
			selectedRAMBank = 0;
			ramBankMode = false;
		}

		break;

	}
}

void MBC::WriteRegisterMBC3(uint16_t address, uint8_t value)
{
	switch (address & 0xF000)
	{
	case 0x0000:
	case 0x1000:
		// RAM enable
		ramEnabled = (value & 0xF) == 0xA;

		// TODO: enable/disable RTC timer
		break;

	case 0x2000:
	case 0x3000:
		// ROM bank select, lower 7 bits only
		value &= 0x7F;

		// Cannot select bank 0x00, select 0x01 instead
		if (value == 0x00)
			value = 0x01;

		selectedROMBank = value;
		break;

	case 0x4000:
	case 0x5000:
		if (value > 0x7)
		{
			// TODO: support RTC reading
			assert(false);
		}
		else
		{
			// RAM bank number
			selectedRAMBank = value;
		}

		break;

	case 0x6000:
	case 0x7000:
		// TODO: Support RTC
		break;

	}
}

void MBC::WriteRegisterMBC5(uint16_t address, uint8_t value)
{
	switch (address & 0xF000)
	{
	case 0x0000:
	case 0x1000:
		// RAM enable
		ramEnabled = (value & 0xF) == 0xA;
		break;

	case 0x2000:
		// Lower 8 bits of the ROM bank

		// Clear the lower 8 bits of the current bank and set the new bits
		selectedROMBank = (selectedROMBank & 0xFF00) | value;
		break;

	case 0x3000:
		// 9th bit of the ROM bank
		value &= 0x01;

		// Clear the uper 8 bits of the current bank and set the new bits
		selectedROMBank = (selectedROMBank & 0x00FF) | (value << 8);
		break;

	case 0x4000:
	case 0x5000:
		// RAM bank select, maximum of 16 banks
		selectedRAMBank = value & 0xF;
		break;
	}
}

const uint16_t MBC::ROM::Banks() const
{
	return mbc.cartridge.header->romSize << 2;
}

const uint32_t MBC::ROM::Size() const
{
	return 0x8000 << mbc.cartridge.header->romSize;
}

uint8_t MBC::ROM::ReadByte(uint16_t address) const
{
	const uint8_t* bank = GetBank(address >= 0x4000 ? mbc.selectedROMBank : 0);
	return *(bank + (address % 0x4000));
}

void MBC::ROM::WriteByte(uint16_t address, uint8_t value)
{
	mbc.WriteRegister(address, value);
}


MBC::RAM::RAM(MBC& mbc) : mbc(mbc)
{
	assert(mbc.cartridge.header->ramSize < (sizeof(Cartridge::RAM_SIZES) / sizeof(uint8_t)));

	size = Cartridge::RAM_SIZES[mbc.cartridge.header->ramSize];
	size <<= 10; // RAM sizes are stored in KB

	buffer = new uint8_t[size];

	// According to SameBoy source, uninitialized MBC RAM should be 0xFF
	memset(buffer, 0xFF, size);
}

MBC::RAM::~RAM()
{
	if (buffer != NULL)
		delete[] buffer;
}

uint32_t MBC::RAM::Size() const { return size; }

uint8_t MBC::RAM::ReadByte(uint16_t address) const
{
	if (!mbc.ramEnabled)
		return 0xFF;

	return GetCurrentBank()[address];
}

void MBC::RAM::WriteByte(uint16_t address, uint8_t value)
{
	if (!mbc.ramEnabled)
		return;

	assert(size > 0);

	WRITE_BYTE(GetCurrentBank() + address, value);
}