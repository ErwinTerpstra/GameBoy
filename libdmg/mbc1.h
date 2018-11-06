#ifndef _MBC1_H_
#define _MBC1_H_

#include "memorybank.h"

#include "cartridge.h"

namespace libdmg
{

	class MBC1
	{
		friend class ROM;
		friend class RAM;

	public:
		class ROM : public MemoryBank
		{
		private:
			MBC1& mbc;

			uint32_t size;

		public:
			ROM(MBC1& mbc) : mbc(mbc)
			{
			}

			const uint16_t Banks() const
			{
				return mbc.cartridge.header->romSize << 2;
			}

			const uint32_t Size() const
			{
				return 0x8000 << mbc.cartridge.header->romSize;
			}

			uint8_t ReadByte(uint16_t address) const
			{
				const uint8_t* bank = GetBank(address >= 0x4000 ? mbc.selectedROMBank : 0);
				return *(bank + (address % 0x4000));
			}

			void WriteByte(uint16_t address, uint8_t value)
			{
				switch (address & 0xF000)
				{
					case 0x0000:
					case 0x1000:
						// RAM enable
						mbc.ramEnabled = (value & 0xF) == 0xA;
						break;

					case 0x2000:
					case 0x3000:
						// ROM bank select, lower 5 bits only
						value &= 0x1F;

						// Cannot select bank 0x00, select 0x01 instead
						if (value == 0x00)
							value = 0x01;

						// Clear the lower 5 bits of the current bank and set the new bits
						mbc.selectedROMBank = (mbc.selectedROMBank & ~0x1F) | value;
						break;

					case 0x4000:
					case 0x5000:
						// RAM bank/upper ROM bank select, lower 2 bits only
						value &= 0x03;

						if (mbc.ramBankMode)
						{
							mbc.selectedRAMBank = value;
						}
						else
						{
							// Use value as bits 5 & 6 of selected ROM bank
							mbc.selectedROMBank = (mbc.selectedROMBank & ~0x1F) | (value << 5);
						}

						break;

					case 0x6000:
					case 0x7000:
						// ROM/RAM mode switch
						if (value)
						{
							mbc.selectedROMBank &= 0x1F;
							mbc.ramBankMode = true;
						}
						else
						{
							mbc.selectedRAMBank = 0;
							mbc.ramBankMode = false;
						}

						break;

				}
			}

		private:
			const uint8_t* GetBank(uint16_t bank) const { return mbc.cartridge.rom + (0x4000 * bank); }

		};

		class RAM : public MemoryBank
		{
		private:
			MBC1& mbc;

			uint8_t* buffer;
			uint32_t size;

		public:
			RAM(MBC1& mbc) : mbc(mbc)
			{
				assert(mbc.cartridge.header->ramSize < (sizeof(Cartridge::RAM_SIZES) / sizeof(uint8_t)));

				size = Cartridge::RAM_SIZES[mbc.cartridge.header->ramSize];
				size <<= 10; // RAM sizes are stored in KB

				buffer = new uint8_t[size];

				// According to SameBoy source, uninitialized MBC RAM should be 0xFF
				memset(buffer, 0xFF, size);
			}

			~RAM()
			{
				if (buffer != NULL)
					delete[] buffer;
			}

			uint32_t Size() const { return size; }

			uint8_t ReadByte(uint16_t address) const
			{
				if (!mbc.ramEnabled)
					return 0xFF;

				return GetCurrentBank()[address]; 
			}

			void WriteByte(uint16_t address, uint8_t value)
			{
				if (!mbc.ramEnabled)
					return;

				assert(size > 0);

				WRITE_BYTE(GetCurrentBank() + address, value); 
			}

		private:
			uint8_t* GetCurrentBank() { return const_cast<uint8_t*>(static_cast<const RAM*>(this)->GetCurrentBank()); }
			const uint8_t* GetCurrentBank() const { return buffer + (mbc.selectedRAMBank << 10); }
		};

	private:
		Cartridge& cartridge;

		bool ramEnabled;
		bool ramBankMode;

		uint16_t selectedROMBank;

		uint8_t selectedRAMBank;

	public:
		ROM rom;
		RAM ram;

	public:
		MBC1(Cartridge& cartridge) : 
			cartridge(cartridge), rom(*this), ram(*this), 
			ramEnabled(false), ramBankMode(false), selectedROMBank(1), selectedRAMBank(0)
		{

		}
	};
}

#endif