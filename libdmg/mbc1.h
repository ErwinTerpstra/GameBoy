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

		public:
			ROM(MBC1& mbc) : mbc(mbc)
			{

			}

			const uint8_t* RetrievePointer(uint16_t address) const { return mbc.cartridge.rom + address; }

		};

		class RAM : public MemoryBank
		{
		private:
			MBC1& mbc;

			uint8_t* buffer;

		public:
			RAM(MBC1& mbc) : mbc(mbc)
			{
				buffer = new uint8_t[GB_MBC1_MAX_RAM];
			}

			~RAM()
			{
				if (buffer != NULL)
					delete[] buffer;
			}

			const uint8_t* RetrievePointer(uint16_t address) const { return buffer + address; }
		};

	private:
		Cartridge& cartridge;

	public:
		ROM rom;
		RAM ram;

	public:
		MBC1(Cartridge& cartridge) : cartridge(cartridge), rom(*this), ram(*this)
		{

		}
	};
}

#endif