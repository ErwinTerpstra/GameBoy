#ifndef _MBC_H_
#define _MBC_H_

#include "memorybank.h"

#include "cartridge.h"

namespace libdmg
{

	class MBC
	{
		friend class ROM;
		friend class RAM;

	public:
		enum Type
		{
			MBC1,
			MBC3,
			MBC5
		};

		class ROM : public MemoryBank
		{
		private:
			MBC& mbc;

			uint32_t size;

		public:
			ROM(MBC& mbc);

			const uint16_t Banks() const;
			const uint32_t Size() const;

			uint8_t ReadByte(uint16_t address) const;

			void WriteByte(uint16_t address, uint8_t value);

		private:
			const uint8_t* GetBank(uint16_t bank) const { return mbc.cartridge.rom + (0x4000 * bank); }

		};

		class RAM : public MemoryBank
		{
		private:
			MBC& mbc;

			uint32_t size;

		public:
			RAM(MBC& mbc);
			~RAM();

			uint32_t Size() const;

			uint8_t ReadByte(uint16_t address) const;

			void WriteByte(uint16_t address, uint8_t value);

		private:
			uint8_t* GetCurrentBank() { return const_cast<uint8_t*>(static_cast<const RAM*>(this)->GetCurrentBank()); }
			const uint8_t* GetCurrentBank() const { return mbc.cartridge.ram + (mbc.selectedRAMBank << 10); }
		};

	private:
		Type type;
		Cartridge& cartridge;

		bool ramEnabled;
		bool ramBankMode;

		bool ramDirty;

		uint16_t selectedROMBank;

		uint8_t selectedRAMBank;

	public:
		ROM rom;
		RAM ram;

		bool IsRamDirty() const { return ramDirty; }
		void ClearRamDirty() { ramDirty = false; }

	public:
		MBC(Type type, Cartridge& cartridge);

		void WriteRegister(uint16_t address, uint8_t value);
		void WriteRegisterMBC1(uint16_t address, uint8_t value);
		void WriteRegisterMBC3(uint16_t address, uint8_t value);
		void WriteRegisterMBC5(uint16_t address, uint8_t value);
	};
}

#endif