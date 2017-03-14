#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "environment.h"

namespace libdmg
{
	class CPU;
	class Memory;
	class Cartridge;
	class VideoController;

	class Emulator
	{
	public:
		CPU& cpu;
		Memory& memory;
		Cartridge& cartridge;
		VideoController& videoController;

	public:
		Emulator(CPU& cpu, Memory& memory, Cartridge& cartridge, VideoController& videoController);
		
		void Boot();

		void Step();

		void PrintRegisters() const;
		void PrintDisassembly(uint16_t instructionCount) const;
	};

}

#endif