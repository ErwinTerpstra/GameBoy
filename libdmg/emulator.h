#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "environment.h"

#include "cpu.h"

#include "timer.h"

namespace libdmg
{
	class Memory;
	class Cartridge;
	class Video;
	class Audio;
	class Input;

	class Emulator
	{
	public:
		CPU& cpu;
		Memory& memory;
		Cartridge& cartridge;
		Video& video;
		Audio& audio;
		Input& input;

	private:
		static const uint8_t MAX_HISTORY_LENGTH = 10;

		Timer timer;

		uint16_t executionHistory[MAX_HISTORY_LENGTH];
		uint16_t historyIdx;
		uint16_t historyLength;

		uint32_t instructionCount[256];

		uint64_t ticks;
		uint32_t ticksUntilNextInstruction;
	public:
		Emulator(CPU& cpu, Memory& memory, Cartridge& cartridge, Video& video, Audio& audio, Input& input);
		
		void Boot();

		void Tick();
		void Step();

		void PrintRegisters() const;
		void PrintDisassembly(uint16_t instructionCount) const;
		void PrintInstructionCount() const;

		const uint64_t& Ticks() const { return ticks; }
	
	private:
		void ExecuteNextInstruction();

		const CPU::Instruction& PrintInstruction(uint16_t address, bool& prefixed) const;
	};

}

#endif