#include "cpu.h"

#include "gameboy.h"

#include "memory.h"
#include "memorypointer.h"

#include "debug.h"

using namespace libdmg;

const uint16_t CPU::INTERRUPT_VECTORS[] = { 0x40, 0x48, 0x50, 0x58, 0x60 };

CPU::CPU(Memory& memory) : memory(memory), interruptEnable(memory, GB_REG_IE), interruptFlags(memory, GB_REG_IF)
{
	nativePointer = (NativePointer*) malloc(sizeof(NativePointer));
	memoryPointer = (MemoryPointer*) malloc(sizeof(MemoryPointer));
}

CPU::~CPU()
{
	free(nativePointer);
	free(memoryPointer);
}

void CPU::Reset()
{
	ticks = 0;
	interruptMasterEnable = true;

	registers.af = 0x01B0;
	registers.bc = 0x0013;
	registers.de = 0x00D8;
	registers.hl = 0x014D;

	registers.sp = 0xFFFE;
	registers.pc = 0x0100;

	memory.WriteByte(0xFF00, 0x0F); // JOYP
	memory.WriteByte(0xFF05, 0x00); // TIMA
	memory.WriteByte(0xFF06, 0x00); // TMA
	memory.WriteByte(0xFF07, 0x00); // TAC
	memory.WriteByte(0xFF10, 0x80); // NR10
	memory.WriteByte(0xFF11, 0xBF); // NR11
	memory.WriteByte(0xFF12, 0xF3); // NR12
	memory.WriteByte(0xFF14, 0xBF); // NR14
	memory.WriteByte(0xFF16, 0x3F); // NR21
	memory.WriteByte(0xFF17, 0x00); // NR22
	memory.WriteByte(0xFF19, 0xBF); // NR24
	memory.WriteByte(0xFF1A, 0x7F); // NR30
	memory.WriteByte(0xFF1B, 0xFF); // NR31
	memory.WriteByte(0xFF1C, 0x9F); // NR32
	memory.WriteByte(0xFF1E, 0xBF); // NR33
	memory.WriteByte(0xFF20, 0xFF); // NR41
	memory.WriteByte(0xFF21, 0x00); // NR42
	memory.WriteByte(0xFF22, 0x00); // NR43
	memory.WriteByte(0xFF23, 0xBF); // NR30
	memory.WriteByte(0xFF24, 0x77); // NR50
	memory.WriteByte(0xFF25, 0xF3); // NR51
	memory.WriteByte(0xFF26, 0xF1); // NR52
	memory.WriteByte(0xFF40, 0x91); // LCDC
	memory.WriteByte(0xFF42, 0x00); // SCY
	memory.WriteByte(0xFF43, 0x00); // SCX
	memory.WriteByte(0xFF45, 0x00); // LYC
	memory.WriteByte(0xFF47, 0xFC); // BGP
	memory.WriteByte(0xFF48, 0xFF); // OBP0
	memory.WriteByte(0xFF49, 0xFF); // OBP1
	memory.WriteByte(0xFF4A, 0x00); // WY
	memory.WriteByte(0xFF4B, 0x00); // WX
	memory.WriteByte(0xFFFF, 0x00); // IE
}

void CPU::Resume()
{
	stopped = false;
}

const CPU::Instruction& CPU::ExecuteNextInstruction()
{
	// Read the opcode the PC points at
	uint8_t opcode;
	memory.Read(registers.pc, opcode);

	// Handle prefixed instructions
	bool prefixedInstruction;
	if (opcode == 0xCB)
	{
		++registers.pc;
		memory.Read(registers.pc, opcode);

		prefixedInstruction = true;
	}
	else
		prefixedInstruction = false;

	// Read the instruction description from the instruction map
	const Instruction& instruction = prefixedInstruction ? PREFIXED_INSTRUCTION_MAP[opcode] : INSTRUCTION_MAP[opcode];

	if (instruction.handler == NULL)
	{
		printf("Missing instruction handler for opcode 0x%s%02X! at 0x%04X\n", prefixedInstruction ? "CB" : "", opcode, registers.pc);
		Debug::Halt();
		return instruction;
	}

	// Read all operands for this instruction into a small buffer
	uint8_t operandBuffer[4];
	memory.ReadBuffer(&operandBuffer[0], registers.pc + 1, instruction.length - 1);

	// Increase the PC to point to the next instruction
	registers.pc += instruction.length;

	// Execute the instruction
	(this->*instruction.handler)(opcode, &operandBuffer[0]);

	// Increase the clock cycle count
	ticks += instruction.duration;

	return instruction;
}


void CPU::TestInterrupts()
{
	if (interruptMasterEnable)
	{
		uint8_t interruptState = *interruptEnable & *interruptFlags;

		for (uint8_t interrupt = 0; interrupt <= INT_JOYPAD; ++interrupt)
		{
			// To execute an interrupt, the following conditions have to be met:
			// 1) The interrupt master enable flag must be set
			// 2) The interrupt must be enabled in the IE register
			// 3) The interrupt must be requested in the IF register
			if (interruptMasterEnable && (interruptState & (1 << interrupt)))
			{
				ExecuteInterrupt((Interrupt) interrupt);
				break;
			}
		}
	}
}

void CPU::RequestInterrupt(Interrupt interrupt)
{
	// Set the flag in the IF register
	interruptFlags = *interruptFlags | (1 << interrupt);
	halted = false;
}

void CPU::ExecuteInterrupt(Interrupt interrupt)
{
	// Clear the flag in the IF register
	interruptFlags = *interruptFlags & ~(1 << interrupt);

	interruptMasterEnable = false;

	// Push the program counter to the stack
	WriteStackShort(registers.pc);

	// Jump to the interrupt vector
	registers.pc = INTERRUPT_VECTORS[interrupt];

	ticks += GB_ISR_DURATION;
}

NativePointer* CPU::CreateNativePointer(uint8_t* ptr)
{
	return new (nativePointer) NativePointer(ptr);
}

MemoryPointer* CPU::CreateMemoryPointer(uint16_t address)
{
	return new (memoryPointer) MemoryPointer(memory, address);
}

uint8_t CPU::ReadStackByte()
{
	return memory.ReadByte(registers.sp++);
}

uint16_t CPU::ReadStackShort()
{
	uint16_t value = memory.ReadShort(registers.sp);
	registers.sp += 2;

	return value;
}

void CPU::WriteStackByte(uint8_t value)
{
	registers.sp -= 1;
	memory.WriteByte(registers.sp, value);
}

void CPU::WriteStackShort(uint16_t value)
{
	registers.sp -= 2;
	memory.WriteShort(registers.sp, value);
}

uint8_t CPU::ReadSourceValue(uint8_t opcode, const uint8_t* operands) const
{
	if (opcode > 0xC0 && (opcode % 8) == 0x6)
		return operands[0];

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

Pointer* CPU::GetSourcePointer(uint8_t opcode)
{
	switch (opcode % 8)
	{
		case 0x0: return CreateNativePointer(&registers.b);
		case 0x1: return CreateNativePointer(&registers.c);
		case 0x2: return CreateNativePointer(&registers.d);
		case 0x3: return CreateNativePointer(&registers.e);
		case 0x4: return CreateNativePointer(&registers.h);
		case 0x5: return CreateNativePointer(&registers.l);
		case 0x7: return CreateNativePointer(&registers.a);

		case 0x6: return CreateMemoryPointer(registers.hl);
	}
}

void CPU::enable_interupts(uint8_t opcode, const uint8_t* operands)
{
	interruptMasterEnable = true;
}

void CPU::disable_interrupts(uint8_t opcode, const uint8_t* operands)
{
	interruptMasterEnable = false;
}

void CPU::stop(uint8_t opcode, const uint8_t* operands)
{
	stopped = true;
}

void CPU::halt(uint8_t opcode, const uint8_t* operands)
{
	halted = true;
}

void CPU::jump(uint8_t opcode, const uint8_t* operands)
{
	registers.pc = DECODE_SHORT(operands);
}

void CPU::jump_conditional(uint8_t opcode, const uint8_t* operands)
{
	bool conditional;

	switch (opcode)
	{
		case 0xC2: conditional = !GetFlag(FLAG_ZERO); break;
		case 0xCA: conditional = GetFlag(FLAG_ZERO); break;
		case 0xD2: conditional = !GetFlag(FLAG_CARRY); break;
		case 0xDA: conditional = GetFlag(FLAG_CARRY); break;

		default: assert(false && "Invalid opcode for handler!");
	}

	if (conditional)
		registers.pc = DECODE_SHORT(operands);
}

void CPU::jump_to_hl(uint8_t opcode, const uint8_t* operands)
{
	registers.pc = registers.hl;
}

void CPU::jump_to_offset(uint8_t opcode, const uint8_t* operands)
{
	registers.pc += DECODE_SIGNED_BYTE(operands);
}

void CPU::jump_to_offset_conditional(uint8_t opcode, const uint8_t* operands)
{
	bool conditional;

	switch (opcode)
	{
		case 0x20: conditional = !GetFlag(FLAG_ZERO); break;
		case 0x28: conditional = GetFlag(FLAG_ZERO); break;
		case 0x30: conditional = !GetFlag(FLAG_CARRY); break;
		case 0x38: conditional = GetFlag(FLAG_CARRY); break;

		default: assert(false && "Invalid opcode for handler!");
	}

	if (conditional)
		registers.pc += DECODE_SIGNED_BYTE(operands);
}

void CPU::call(uint8_t opcode, const uint8_t* operands)
{
	WriteStackShort(registers.pc);
	registers.pc = DECODE_SHORT(operands);
}

void CPU::call_conditional(uint8_t opcode, const uint8_t* operands)
{
	bool conditional;

	switch (opcode)
	{
		case 0xC4: conditional = !GetFlag(FLAG_ZERO); break;
		case 0xCC: conditional = GetFlag(FLAG_ZERO); break;
		case 0xD4: conditional = !GetFlag(FLAG_CARRY); break;
		case 0xDC: conditional = GetFlag(FLAG_CARRY); break;

		default: assert(false && "Invalid opcode for handler!");
	}
	if (conditional)
	{
		WriteStackShort(registers.pc);
		registers.pc = DECODE_SHORT(operands);
	}
}

void CPU::restart(uint8_t opcode, const uint8_t* operands)
{
	assert(opcode > 0xC0 && opcode <= 0xFF && (opcode % 16 == 7 || opcode % 16 == 15));

	WriteStackShort(registers.pc);
	registers.pc = opcode - 0xC7;
}

void CPU::return_default(uint8_t opcode, const uint8_t* operands)
{
	registers.pc = ReadStackShort();
}

void CPU::return_conditional(uint8_t opcode, const uint8_t* operands)
{
	bool conditional;

	switch (opcode)
	{
		case 0xC0: conditional = !GetFlag(FLAG_ZERO); break;
		case 0xC8: conditional = GetFlag(FLAG_ZERO); break;
		case 0xD0: conditional = !GetFlag(FLAG_CARRY); break;
		case 0xD8: conditional = GetFlag(FLAG_CARRY); break;

		default: assert(false && "Invalid opcode for handler!");
	}

	if (conditional)
		registers.pc = ReadStackShort();

}

void CPU::return_enable_interrupts(uint8_t opcode, const uint8_t* operands)
{
	registers.pc = ReadStackShort();

	enable_interupts(opcode, operands);
}

void CPU::adjust_bcd(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = registers.a;

	if (GetFlag(FLAG_SUBTRACT))
	{
		if (GetFlag(FLAG_CARRY))
			value -= 0x60;

		if (GetFlag(FLAG_HALF_CARRY))
			value -= 0x06;
	}
	else
	{
		if (value > 0x99 || GetFlag(FLAG_CARRY))
		{
			value += 0x60;
			SetFlag(FLAG_CARRY, true);
		}

		if ((value & 0x0F) > 0x09 || GetFlag(FLAG_HALF_CARRY))
			value += 0x06;

	}

	SetFlag(FLAG_ZERO, value == 0);
	SetFlag(FLAG_HALF_CARRY, false);

	registers.a = value;
}

void CPU::alu_add(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode, operands);
	uint8_t result = registers.a + value;
	
	SetFlag(FLAG_ZERO, result == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, CARRY_BIT_4(registers.a, value, result));
	SetFlag(FLAG_CARRY, OVERFLOW_8(registers.a, value, result));

	registers.a = result;
}

void CPU::alu_adc(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode, operands) + GetFlag(FLAG_CARRY);
	uint8_t result = registers.a + value;

	SetFlag(FLAG_ZERO, result == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, CARRY_BIT_4(registers.a, value, result));
	SetFlag(FLAG_CARRY, OVERFLOW_8(registers.a, value, result));

	registers.a = result;
}

void CPU::alu_sub(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode, operands);
	uint8_t result = registers.a - value;

	SetFlag(FLAG_ZERO, result == 0);
	SetFlag(FLAG_SUBTRACT, true);
	SetFlag(FLAG_HALF_CARRY, CARRY_BIT_4(registers.a, value, result));
	SetFlag(FLAG_CARRY, UNDERFLOW_8(registers.a, value, result));

	registers.a = result;
}

void CPU::alu_sbc(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode, operands) + GetFlag(FLAG_CARRY);
	uint8_t result = registers.a - value;

	SetFlag(FLAG_ZERO, result == 0);
	SetFlag(FLAG_SUBTRACT, true);
	SetFlag(FLAG_HALF_CARRY, CARRY_BIT_4(registers.a, value, result));
	SetFlag(FLAG_CARRY, UNDERFLOW_8(registers.a, value, result));

	registers.a = result;
}

void CPU::alu_and(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode, operands);
	registers.a = registers.a & value;

	SetFlag(FLAG_ZERO, registers.a == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, true);
	SetFlag(FLAG_CARRY, false);
}

void CPU::alu_or(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode, operands);
	registers.a = registers.a | value;

	SetFlag(FLAG_ZERO, registers.a == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, false);
	SetFlag(FLAG_CARRY, false);
}

void CPU::alu_xor(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode, operands);

	registers.a = registers.a ^ value;

	SetFlag(FLAG_ZERO, registers.a == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, false);
	SetFlag(FLAG_CARRY, false);
}

void CPU::alu_cmp(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode, operands);
	uint8_t result = registers.a - value;

	SetFlag(FLAG_ZERO, result == 0);
	SetFlag(FLAG_SUBTRACT, true);
	SetFlag(FLAG_HALF_CARRY, CARRY_BIT_4(registers.a, value, result));
	SetFlag(FLAG_CARRY, result > registers.a);
}

void CPU::alu_inc(uint8_t opcode, const uint8_t* operands)
{
	Pointer* target = NULL;

	switch (opcode)
	{
		case 0x04: target = CreateNativePointer(&registers.b); break;
		case 0x0C: target = CreateNativePointer(&registers.c); break;
		case 0x14: target = CreateNativePointer(&registers.d); break;
		case 0x1C: target = CreateNativePointer(&registers.e); break;
		case 0x24: target = CreateNativePointer(&registers.h); break;
		case 0x2C: target = CreateNativePointer(&registers.l); break;
		case 0x3C: target = CreateNativePointer(&registers.a); break;

		case 0x34: target = CreateMemoryPointer(registers.hl); break;
	}

	uint8_t value = **target;
	*target = value + 1;

	SetFlag(FLAG_ZERO, **target == 0);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, (**target & 0x10) == 0x10);
}

void CPU::alu_dec(uint8_t opcode, const uint8_t* operands)
{
	Pointer* target = NULL;

	switch (opcode)
	{
		case 0x05: target = CreateNativePointer(&registers.b); break;
		case 0x0D: target = CreateNativePointer(&registers.c); break;
		case 0x15: target = CreateNativePointer(&registers.d); break;
		case 0x1D: target = CreateNativePointer(&registers.e); break;
		case 0x25: target = CreateNativePointer(&registers.h); break;
		case 0x2D: target = CreateNativePointer(&registers.l); break;
		case 0x3D: target = CreateNativePointer(&registers.a); break;

		case 0x35: target = CreateMemoryPointer(registers.hl); break;
	}

	uint8_t value = **target;

	*target = value - 1;

	SetFlag(FLAG_ZERO, **target == 0);
	SetFlag(FLAG_SUBTRACT, true);
	SetFlag(FLAG_HALF_CARRY, (**target & 0x10) == 0x10);
}

void CPU::alu_complement(uint8_t opcode, const uint8_t* operands)
{
	registers.a = ~registers.a;

	SetFlag(FLAG_SUBTRACT, true);
	SetFlag(FLAG_HALF_CARRY, true);
}

void CPU::alu_complement_carry(uint8_t opcode, const uint8_t* operands)
{
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, false);
	SetFlag(FLAG_CARRY, !GetFlag(FLAG_CARRY));
}

void CPU::alu_set_carry(uint8_t opcode, const uint8_t* operands)
{
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, false);
	SetFlag(FLAG_CARRY, true);
}

void CPU::alu_inc_16bit(uint8_t opcode, const uint8_t* operands)
{
	switch (opcode >> 4)
	{
		case 0x0: ++registers.bc; break;
		case 0x1: ++registers.de; break;
		case 0x2: ++registers.hl; break;
		case 0x3: ++registers.sp; break;

		default: assert(false && "Invalid opcode for handler!");
	}
}

void CPU::alu_dec_16bit(uint8_t opcode, const uint8_t* operands)
{
	switch (opcode >> 4)
	{
		case 0x0: --registers.bc; break;
		case 0x1: --registers.de; break;
		case 0x2: --registers.hl; break;
		case 0x3: --registers.sp; break;

		default: assert(false && "Invalid opcode for handler!");
	}

}

void CPU::alu_add_hl_16bit(uint8_t opcode, const uint8_t* operands)
{
	uint16_t value;

	switch (opcode)
	{
		case 0x09: value = registers.bc; break;
		case 0x19: value = registers.de; break;
		case 0x29: value = registers.hl; break;
		case 0x39: value = registers.sp; break;

		default: assert(false && "Invalid opcode for handler!");
	}

	uint16_t result = registers.hl + value;

	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = SET_MASK_IF(registers.f, FLAG_HALF_CARRY, CARRY_BIT_12(registers.hl, value, result));
	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, OVERFLOW_16(registers.hl, value, result));

	registers.hl = result;
}

void CPU::alu_add_sp_constant(uint8_t opcode, const uint8_t* operands)
{
	int8_t operand = REINTERPRET(operands[0], int8_t);
	uint16_t result = registers.sp + operand;

	SetFlag(FLAG_ZERO, false);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, CARRY_BIT_4(registers.sp, operand, result));
	SetFlag(FLAG_CARRY, CARRY_BIT_8(registers.sp, operand, result));

	registers.sp = result;
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

		default: assert(false && "Invalid opcode for handler!");
	}
}


void CPU::load_memory_to_memory(uint8_t opcode, const uint8_t* operands)
{
	uint8_t value = ReadSourceValue(opcode, operands);

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

		default: assert(false && "Invalid opcode for handler!");
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

		default: assert(false && "Invalid opcode for handler!");
	}

}

void CPU::load_accumulator_to_constant_io_register(uint8_t opcode, const uint8_t* operands)
{
	memory.WriteByte(GB_IO_REGISTERS + operands[0], registers.a);
}

void CPU::load_constant_io_register_to_accumulator(uint8_t opcode, const uint8_t* operands)
{
	registers.a = memory.ReadByte(GB_IO_REGISTERS + operands[0]);
}

void CPU::load_accumulator_to_c_plus_io_register(uint8_t opcode, const uint8_t* operands)
{
	memory.WriteByte(GB_IO_REGISTERS + registers.c, registers.a);
}

void CPU::load_c_plus_io_register_to_accumulator(uint8_t opcode, const uint8_t* operands)
{
	registers.a = memory.ReadByte(GB_IO_REGISTERS + registers.c);
}

void CPU::load_constant_16bit(uint8_t opcode, const uint8_t* operands)
{
	switch (opcode)
	{
		case 0x01: registers.bc = DECODE_SHORT(operands); break;
		case 0x11: registers.de = DECODE_SHORT(operands); break;
		case 0x21: registers.hl = DECODE_SHORT(operands); break;
		case 0x31: registers.sp = DECODE_SHORT(operands); break;

		default: assert(false && "Invalid opcode for handler!");
	}
}

void CPU::load_hl_to_sp(uint8_t opcode, const uint8_t* operands)
{
	registers.sp = registers.hl;
}

void CPU::load_sp_plus_constant_to_hl(uint8_t opcode, const uint8_t* operands)
{
	int8_t operand = REINTERPRET(operands[0], int8_t);
	uint16_t result = registers.sp + operand;

	SetFlag(FLAG_ZERO, false);
	SetFlag(FLAG_SUBTRACT, false);
	SetFlag(FLAG_HALF_CARRY, CARRY_BIT_4(registers.sp, operand, result));
	SetFlag(FLAG_CARRY, CARRY_BIT_8(registers.sp, operand, result));

	registers.hl = result;
}

void CPU::load_sp_to_memory(uint8_t opcode, const uint8_t* operands)
{
	memory.WriteShort(DECODE_SHORT(operands), registers.sp);
}

void CPU::load_accumulator_to_memory_16bit(uint8_t opcode, const uint8_t* operands)
{
	uint16_t address = DECODE_SHORT(operands);
	memory.WriteByte(address, registers.a);
}

void CPU::load_memory_to_accumulator_16bit(uint8_t opcode, const uint8_t* operands)
{
	uint16_t address = DECODE_SHORT(operands);
	registers.a = memory.ReadByte(address);

}

void CPU::push_stack_16bit(uint8_t opcode, const uint8_t* operands)
{
	switch (opcode)
	{
		case 0xC5: WriteStackShort(registers.bc); break;
		case 0xD5: WriteStackShort(registers.de); break;
		case 0xE5: WriteStackShort(registers.hl); break;
		case 0xF5: WriteStackShort(registers.af); break;

		default: assert(false && "Invalid opcode for handler!");
	}
}

void CPU::pop_stack_16bit(uint8_t opcode, const uint8_t* operands)
{
	switch (opcode)
	{
		case 0xC1: registers.bc = ReadStackShort(); break;
		case 0xD1: registers.de = ReadStackShort(); break;
		case 0xE1: registers.hl = ReadStackShort(); break;
		
		case 0xF1: 
			// POP AF clears the lower nibble of the F register, which seems to be undocumented behaviour
			registers.af = ReadStackShort() & 0xFFF0; 
			break;

		default: assert(false && "Invalid opcode for handler!");
	}
}

void CPU::rotate_accumulator_left(uint8_t opcode, const uint8_t* operands)
{
	uint8_t bit7 = READ_BIT(registers.a, 7);
	registers.a = ((registers.a << 1) | READ_MASK(registers.f, FLAG_CARRY));
	
	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit7);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::rotate_accumulator_right(uint8_t opcode, const uint8_t* operands)
{
	uint8_t bit0 = READ_BIT(registers.a, 0);
	registers.a = ((registers.a >> 1) | (READ_MASK(registers.f, FLAG_CARRY) << 7));

	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit0);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::rotate_accumulator_left_circular(uint8_t opcode, const uint8_t* operands)
{
	uint8_t bit7 = READ_BIT(registers.a, 7);
	registers.a = ((registers.a << 1) | bit7);

	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit7);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::rotate_accumulator_right_circular(uint8_t opcode, const uint8_t* operands)
{
	uint8_t bit0 = READ_BIT(registers.a, 0);
	registers.a = ((registers.a >> 1) | (bit0 << 7));

	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit0);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::rotate_left(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);
	uint8_t bit7 = READ_BIT(**value, 7);
	*value = ((**value << 1) | READ_MASK(registers.f, FLAG_CARRY));

	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit7);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::rotate_right(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);
	uint8_t bit0 = READ_BIT(**value, 0);
	*value = ((**value >> 1) | (READ_MASK(registers.f, FLAG_CARRY) << 7));

	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit0);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::rotate_left_circular(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);
	uint8_t bit7 = READ_BIT(**value, 7);
	*value = ((**value << 1) | bit7);

	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit7);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::rotate_right_circular(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);
	uint8_t bit0 = READ_BIT(**value, 0);
	*value = ((**value >> 1) | (bit0 << 7));

	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit0);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::shift_left_arithmetically(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);
	uint8_t bit7 = READ_BIT(**value, 7);
	*value = **value << 1;

	registers.f = SET_MASK_IF(registers.f, FLAG_ZERO, **value == 0);
	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit7);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::shift_right_arithmetically(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);
	uint8_t bit7 = READ_BIT(**value, 7);
	uint8_t bit0 = READ_BIT(**value, 0);
	*value = SET_BIT_IF(**value >> 1, 7, bit7);

	registers.f = SET_MASK_IF(registers.f, FLAG_ZERO, **value == 0);
	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit0);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::shift_right_logically(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);
	uint8_t bit0 = READ_BIT(**value, 0);
	*value = **value >> 1;

	registers.f = SET_MASK_IF(registers.f, FLAG_ZERO, **value == 0);
	registers.f = SET_MASK_IF(registers.f, FLAG_CARRY, bit0);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::swap(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);
	*value = (**value << 4) | (**value >> 4);

	registers.f = SET_MASK_IF(registers.f, FLAG_ZERO, **value == 0);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = UNSET_MASK(registers.f, FLAG_HALF_CARRY);
	registers.f = UNSET_MASK(registers.f, FLAG_CARRY);
}

void CPU::test_bit(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);

	uint8_t bit = (opcode - 0x40) / 8;
	uint8_t result = READ_BIT(**value, bit);

	registers.f = SET_MASK_IF(registers.f, FLAG_ZERO, result == 0);
	registers.f = UNSET_MASK(registers.f, FLAG_SUBTRACT);
	registers.f = SET_MASK(registers.f, FLAG_HALF_CARRY);
}

void CPU::reset_bit(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);

	uint8_t bit = (opcode - 0x80) / 8;
	*value = UNSET_BIT(**value, bit);
}

void CPU::set_bit(uint8_t opcode, const uint8_t* operands)
{
	Pointer* value = GetSourcePointer(opcode);

	uint8_t bit = (opcode - 0xC0) / 8;
	*value = SET_BIT(**value, bit);
}