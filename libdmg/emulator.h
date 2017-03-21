#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "environment.h"

#include "cpu.h"

namespace libdmg
{
	class Memory;
	class Cartridge;
	class Video;
	class Input;

	class Emulator
	{
	public:
		CPU& cpu;
		Memory& memory;
		Cartridge& cartridge;
		Video& video;
		Input& input;

	private:
		static const uint8_t MAX_HISTORY_LENGTH = 10;

		uint16_t executionHistory[MAX_HISTORY_LENGTH];
		uint16_t historyIdx;
		uint16_t historyLength;

	public:
		Emulator(CPU& cpu, Memory& memory, Cartridge& cartridge, Video& video, Input& input);
		
		void Boot();

		void Step();

		void PrintRegisters() const;
		void PrintDisassembly(uint16_t instructionCount) const;
	
	private:
		const CPU::Instruction& PrintInstruction(uint16_t address, bool prefixed) const;
	};

}

#endif