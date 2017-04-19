#include "emulator.h"

#include "cpu.h"
#include "memory.h"
#include "cartridge.h"
#include "video.h"
#include "input.h"

#include "gameboy.h"

#include "debug.h"

using namespace libdmg;

Emulator::Emulator(CPU& cpu, Memory& memory, Cartridge& cartridge, Video& video, Input& input) : 
	cpu(cpu), memory(memory), cartridge(cartridge), video(video), input(input),
	historyIdx(0), historyLength(0)
{

}

void Emulator::Boot()
{
	// Currently, only support ROM only cartridges
	assert(cartridge.header->cartridgeHardware == 0);

	cpu.Reset();
	video.Reset();

	// Copy lower 16K of rom to main memory
	memory.WriteBuffer(cartridge.rom, 0x0000, 0x4000);

	// Copy upper 16K of rom to main memory
	// TODO: Handle memory bank controllers
	memory.WriteBuffer(cartridge.rom + 0x4000, 0x4000, 0x4000);
}

void Emulator::Step()
{
	// Save the PC in the instruction history
	const CPU::Registers& registers = cpu.GetRegisters();
	historyIdx = historyIdx < MAX_HISTORY_LENGTH - 1 ? historyIdx + 1 : 0;
	historyLength = std::min(historyLength + 1U, (unsigned int)MAX_HISTORY_LENGTH);
	executionHistory[historyIdx] = registers.pc;

	cpu.ExecuteNextInstruction();
	video.Sync();
	input.Update();
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
	Debug::Print("\n");
}

void Emulator::PrintDisassembly(uint16_t instructionCount) const
{
	const CPU::Registers& registers = cpu.GetRegisters();

	bool prefixNextInstruction = false;
	
	Debug::Print("Disassembly:\n");

	for (uint16_t historyEntryIdx = historyLength; historyEntryIdx > 0; --historyEntryIdx)
	{
		uint16_t historyIdx = (MAX_HISTORY_LENGTH + this->historyIdx - historyEntryIdx + 1) % MAX_HISTORY_LENGTH;

		const CPU::Instruction& instruction = PrintInstruction(executionHistory[historyIdx], prefixNextInstruction);
		prefixNextInstruction = instruction.opcode == 0xCB;
	}

	Debug::Print("===========>\n");

	uint16_t address = registers.pc;
	for (uint16_t instructionIdx = 0; instructionIdx < instructionCount; ++instructionIdx)
	{
		const CPU::Instruction& instruction = PrintInstruction(address, prefixNextInstruction);
		address += instruction.length;

		prefixNextInstruction = instruction.opcode == 0xCB;
	}
	
	Debug::Print("\n");
}

const CPU::Instruction& Emulator::PrintInstruction(uint16_t address, bool prefixed) const
{
	static char disassemblyBuffer[256];

	uint8_t opcode = memory.ReadByte(address);
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