#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "cpu.h"
#include "cartridge.h"
#include "memory.h"

namespace libdmg
{

	class Emulator
	{
	public:
		CPU cpu;

		Cartridge& cartridge;
		Memory& memory;

	public:
		Emulator(Memory& memory, Cartridge& cartridge);
		
		void Boot();

		void Step();
	};

}

#endif