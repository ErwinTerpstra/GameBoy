#include "emulator.h"

#include "cpu.h"
#include "memory.h"
#include "cartridge.h"
#include "video.h"
#include "audio.h"
#include "input.h"

#include "gameboy.h"

#include "debug.h"

using namespace libdmg;

Emulator::Emulator(CPU& cpu, Memory& memory, Cartridge& cartridge, Video& video, Audio& audio, Input& input) : 
	cpu(cpu), memory(memory), cartridge(cartridge), video(video), audio(audio), input(input),
	timer(cpu, memory),
	ticks(0), ticksUntilNextInstruction(0), 
	historyIdx(0), historyLength(0)
{
	memory.BindIO(&input, audio.Sound1(), audio.Sound2());
}

void Emulator::Boot()
{
	ticks = 0;
	ticksUntilNextInstruction = 0;

	// Reset CPU statistics
	historyIdx = 0;
	historyLength = 0;

	for (uint16_t opcode = 0; opcode < 256; ++opcode)
		instructionCount[opcode] = 0;

	// Map cartridge rom and ram to memory
	memory.BindCartridge(cartridge);

	// Reset IO subsystems
	cpu.Reset();
	timer.Reset();
	video.Reset();
	audio.Reset();
}

void Emulator::Step()
{
	uint64_t cpuTicks = cpu.Ticks();

	do
	{
		Tick();
	} while (cpu.Ticks() == cpuTicks);
}

void Emulator::Tick()
{
	++ticks;

	if (!cpu.Stopped())
	{
		uint64_t previousTicks = cpu.Ticks();

		if (!cpu.Halted() && ticksUntilNextInstruction == 0)
		{
			// If the CPU is not halted, test interrupts after executing an instruction
			ExecuteNextInstruction();
			cpu.TestInterrupts();
		}

		if (ticksUntilNextInstruction > 0)
			--ticksUntilNextInstruction;

		// Delay next CPU instruction until we've caught up 
		ticksUntilNextInstruction += (uint32_t)(cpu.Ticks() - previousTicks);
	}

	// Update IO subsystems
	timer.Sync(ticks);
	video.Sync(ticks);
	audio.Sync(ticks);
}

void Emulator::ExecuteNextInstruction()
{
	// Save the PC in the instruction history
	const CPU::Registers& registers = cpu.GetRegisters();
	historyIdx = historyIdx < MAX_HISTORY_LENGTH - 1 ? historyIdx + 1 : 0;
	historyLength = std::min(historyLength + 1U, (unsigned int)MAX_HISTORY_LENGTH);
	executionHistory[historyIdx] = registers.pc;

	// Execute the next CPU instruction
	const CPU::Instruction& instruction = cpu.ExecuteNextInstruction();

	++instructionCount[instruction.opcode];
}

void Emulator::PrintRegisters() const
{
	const CPU::Registers& registers = cpu.GetRegisters();

	Debug::Print("Registers:\n");
	
	Debug::Print("a: 0x%02X; f: 0x%02X; af: 0x%04X\n", registers.a, registers.f, registers.af);
	Debug::Print("b: 0x%02X; c: 0x%02X; bc: 0x%04X\n", registers.b, registers.c, registers.bc);
	Debug::Print("d: 0x%02X; e: 0x%02X; de: 0x%04X\n", registers.d, registers.e, registers.de);
	Debug::Print("h: 0x%02X; l: 0x%02X; hl: 0x%04X\n", registers.h, registers.l, registers.hl);
	
	Debug::Print("z: %d; n: %d; h: %d, c: %d\n",
		READ_MASK(registers.f, CPU::FLAG_ZERO),
		READ_MASK(registers.f, CPU::FLAG_SUBTRACT),
		READ_MASK(registers.f, CPU::FLAG_HALF_CARRY),
		READ_MASK(registers.f, CPU::FLAG_CARRY));

	Debug::Print("pc: 0x%04X; sp: 0x%04X\n", registers.pc, registers.sp);
	Debug::Print("ime: %d, if: 0x%02X; ie: 0x%02X\n", cpu.InterruptMasterEnable() ? 1 : 0, memory.ReadByte(GB_REG_IF), memory.ReadByte(GB_REG_IE));
	Debug::Print("DIV: 0x%02X; TIMA: 0x%02X; TMA: 0x%02X; TAC: 0x%02X\n", memory.ReadByte(GB_REG_DIV), memory.ReadByte(GB_REG_TIMA), memory.ReadByte(GB_REG_TMA), memory.ReadByte(GB_REG_TAC));
	Debug::Print("\n");
}

void Emulator::PrintDisassembly(uint16_t instructionCount) const
{
	bool prefixed;
	const CPU::Registers& registers = cpu.GetRegisters();

	Debug::Print("Disassembly:\n");

	for (uint16_t historyEntryIdx = historyLength; historyEntryIdx > 0; --historyEntryIdx)
	{
		uint16_t historyIdx = (MAX_HISTORY_LENGTH + this->historyIdx - historyEntryIdx + 1) % MAX_HISTORY_LENGTH;

		PrintInstruction(executionHistory[historyIdx], prefixed);
	}

	Debug::Print("===========>\n");

	uint16_t address = registers.pc;
	for (uint16_t instructionIdx = 0; instructionIdx < instructionCount; ++instructionIdx)
	{
		const CPU::Instruction& instruction = PrintInstruction(address, prefixed);
		address += instruction.length;

		if (prefixed)
			++address;
	}
	
	Debug::Print("\n");
}

const CPU::Instruction& Emulator::PrintInstruction(uint16_t address, bool& prefixed) const
{
	static char disassemblyBuffer[256];

	uint8_t opcode = memory.ReadByte(address);

	if (opcode == 0xCB)
	{
		prefixed = true;
		opcode = memory.ReadByte(address + 1);
	}
	else
		prefixed = false;

	const CPU::Instruction& instruction = prefixed ? CPU::PREFIXED_INSTRUCTION_MAP[opcode] : CPU::INSTRUCTION_MAP[opcode];

	switch (instruction.length)
	{
		case 0:
		case 1:
			sprintf_s(disassemblyBuffer, instruction.disassemblyFormat);
			break;

		case 2:
			sprintf_s(disassemblyBuffer, instruction.disassemblyFormat, memory.ReadByte(address + 1));
			break;

		case 3:
			sprintf_s(disassemblyBuffer, instruction.disassemblyFormat, memory.ReadShort(address + 1));
			break;

		default:
			assert(false && "Not implemented");
			break;
	}

	Debug::Print("0x%04X\t0x%02X\t%s\n", address, opcode, disassemblyBuffer);

	return instruction;
}

void Emulator::PrintInstructionCount() const
{
	static char suffixes[] = { 'K', 'M', 'G', 'T' };

	// Print header
	Debug::Print("\t");

	for (uint8_t lowerNibble = 0; lowerNibble < 16; ++lowerNibble)
		Debug::Print("    x%X\t", lowerNibble);

	Debug::Print("\n");

	// Print rows
	char numberBuffer[16];
	for (uint8_t upperNibble = 0; upperNibble < 16; ++upperNibble)
	{
		Debug::Print("%Xx\t", upperNibble);

		for (uint8_t lowerNibble = 0; lowerNibble < 16; ++lowerNibble)
		{
			uint8_t opcode = (upperNibble << 4) | lowerNibble;

			uint32_t count = instructionCount[opcode];
			int8_t suffixIdx = -1;

			while (count > 1000)
			{
				count /= 1000;
				++suffixIdx;
			}

			sprintf_s(numberBuffer, 16, "%d%c", count, suffixIdx >= 0 ? suffixes[suffixIdx] : '\0');

			Debug::Print("% 6s\t", numberBuffer);
		}

		Debug::Print("\n");
	}
}