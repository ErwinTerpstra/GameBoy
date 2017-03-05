#include "cpu.h"

#include "memory.h"

#include "debug.h"

using namespace libdmg;

CPU::CPU(Memory& memory) : memory(memory)
{

}

void CPU::Reset()
{
	registers.af = 0x01B0;
	registers.bc = 0x0013;
	registers.de = 0x00D8;
	registers.hl = 0x014D;

	registers.sp = 0xFFFE;
	registers.pc = 0x0100;

}

void CPU::ExecuteNextInstruction()
{
	// Read the opcode the PC points at
	uint8_t opcode;
	memory.Read(registers.pc, opcode);

	// Read the instruction description from the instruction map
	Instruction& instruction = instructionMap[opcode];

	if (instruction.handler == NULL)
	{
		printf("Missing instruction handler for opcode 0x%02X! at 0x%04X\n", opcode, registers.pc);
		Debug::Halt();
	}

	// Retrieve a pointer to where the operands for this instruction are location
	const uint8_t* operands = memory.RetrievePointer(registers.pc + 1);

	// Increate the PC to point to the next instruction
	registers.pc += instruction.length;

	// Execute the instruction
	(this->*instruction.handler)(opcode, operands);
}

void CPU::SetFlag(Flags flag, bool state)
{
	if (state)
		registers.f |= flag;
	else
		registers.f &= ~flag;
}

bool CPU::GetFlag(Flags flag)
{
	return (registers.f & flag) == flag;
}

void CPU::WriteStackByte(uint8_t value)
{
	memory.WriteByte(registers.sp, value);
	registers.sp -= 1;

}

void CPU::WriteStackShort(uint16_t value)
{
	memory.WriteShort(registers.sp, value);
	registers.sp -= 2;
}

uint8_t CPU::ReadSourceValue(uint8_t opcode)
{
	switch (opcode % 8)
	{
		case 0x0: return registers.b;
		case 0x1: return registers.c;
		case 0x2: return registers.d;
		case 0x3: return registers.e;
		case 0x4: return registers.h;
		case 0x5: return registers.l;
		case 0x7: return registers.a;

		case 0x6: return memory.ReadByte(registers.hl);
	}

}

void CPU::jump(uint8_t opcode, const uint8_t* operands)
{
	registers.pc = DECODE_SHORT(operands);
}

void CPU::call(uint8_t opcode, const uint8_t* operands)
{
	WriteStackShort(registers.pc);
	registers.pc = DECODE_SHORT(operands);
}

void CPU::restart(uint8_t opcode, const uint8_t* operands)
{
	assert(opcode > 0xC0 && opcode <= 0xFF && (opcode % 7 == 0 || opcode % 15 == 0));

	WriteStackShort(registers.pc);
	registers.pc = opcode - 0xC7;
}

void CPU::encode_bcd(uint8_t opcode, const uint8_t* operands)
{
	SetFlag(FLAG_ZERO, registers.a == 0);
	SetFlag(FLAG_HALF_CARRY, false);
	SetFlag(FLAG_CARRY, registers.a > 99);

	uint8_t high = (registers.a / 10) & 0xF;
	uint8_t low = registers.a % 10;

	registers.a = (high << 4) | low;
}

void CPU::alu_add(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode);
	uint16_t result = registers.a + value;
	
	SetFlag(FLAG_ZERO, result == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, (result ^ value ^ registers.a) & 0x10);
	SetFlag(FLAG_CARRY, result > 255);

	registers.a = result;
}

void CPU::alu_adc(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode);
	uint16_t result = registers.a + value + GetFlag(FLAG_CARRY);

	SetFlag(FLAG_ZERO, result == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, (result ^ value ^ registers.a) & 0x10);
	SetFlag(FLAG_CARRY, result > 255);

	registers.a = result;
}

void CPU::alu_sub(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode);
	int16_t result = registers.a - value;

	SetFlag(FLAG_ZERO, result == 0);
	SetFlag(FLAG_SUBTRACT, true);
	SetFlag(FLAG_HALF_CARRY, (result ^ value ^ registers.a) & 0x10);
	SetFlag(FLAG_CARRY, result < 0);

	registers.a = result;
}

void CPU::alu_sbc(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode);
	int16_t result = registers.a - value - GetFlag(FLAG_CARRY);

	SetFlag(FLAG_ZERO, result == 0);
	SetFlag(FLAG_SUBTRACT, true);
	SetFlag(FLAG_HALF_CARRY, (result ^ value ^ registers.a) & 0x10);
	SetFlag(FLAG_CARRY, result < 0);

	registers.a = result;
}

void CPU::alu_and(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode);
	registers.a = registers.a & value;

	SetFlag(FLAG_ZERO, registers.a == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, true);
	SetFlag(FLAG_CARRY, false);
}

void CPU::alu_or(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode);
	registers.a = registers.a & value;

	SetFlag(FLAG_ZERO, registers.a == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, false);
	SetFlag(FLAG_CARRY, false);
}

void CPU::alu_xor(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode);

	registers.a = registers.a & value;

	SetFlag(FLAG_ZERO, registers.a == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, false);
	SetFlag(FLAG_CARRY, false);
}

void CPU::alu_cmp(uint8_t opcode, const uint8_t* operands)
{

}

void CPU::load_constant(uint8_t opcode, const uint8_t* operands)
{
	switch (opcode)
	{
		case 0x06: registers.b = operands[0]; break;
		case 0x0E: registers.c = operands[0]; break;
		case 0x16: registers.d = operands[0]; break;
		case 0x1E: registers.e = operands[0]; break;
		case 0x26: registers.h = operands[0]; break;
		case 0x2E: registers.l = operands[0]; break;
		case 0x3E: registers.a = operands[0]; break;

		case 0x36: memory.WriteByte(registers.hl, operands[0]); break;
	}
}


void CPU::load_memory_to_memory(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode);

	switch (opcode / 8)
	{
		case 8: registers.b = value; break;
		case 9: registers.c = value; break;
		case 10: registers.d = value; break;
		case 11: registers.e = value; break;
		case 12: registers.h = value; break;
		case 13: registers.l = value; break;
		case 15: registers.a = value; break;

		case 14: memory.WriteByte(registers.hl, value); break;

		default: assert(false && "Invalid opcode for handler!");
	}
}

void CPU::load_accumulator_to_memory(uint8_t opcode, const uint8_t* operands)
{
	switch (opcode / 16)
	{
		case 0x0: memory.WriteByte(registers.bc, registers.a); break;
		case 0x1: memory.WriteByte(registers.de, registers.a); break;
		case 0x2: memory.WriteByte(registers.hl, registers.a); ++registers.hl; break;
		case 0x3: memory.WriteByte(registers.hl, registers.a); --registers.hl; break;
	}
}

void CPU::load_memory_to_accumulator(uint8_t opcode, const uint8_t* operands)
{
	switch (opcode / 16)
	{
		case 0x0: memory.Read(registers.bc, registers.a); break;
		case 0x1: memory.Read(registers.de, registers.a); break;
		case 0x2: memory.Read(registers.hl, registers.a); ++registers.hl; break;
		case 0x3: memory.Read(registers.hl, registers.a); --registers.hl; break;
	}

}