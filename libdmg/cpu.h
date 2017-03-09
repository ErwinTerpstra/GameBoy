#ifndef _CPU_H_
#define _CPU_H_

#include "environment.h"
#include "util.h"

namespace libdmg
{
	class Memory;

	class CPU
	{
	private:
		struct Registers
		{
			union
			{
				struct { uint8_t f; uint8_t a; };
				uint16_t af;
			};

			union
			{
				struct { uint8_t c; uint8_t b; };
				uint16_t bc;
			};

			union
			{
				struct { uint8_t e; uint8_t d; };
				uint16_t de;
			};


			union
			{
				struct { uint8_t l; uint8_t h; };
				uint16_t hl;
			};

			uint16_t sp;
			uint16_t pc;
		};

		struct Instruction
		{
			char* disassemblyFormat;
			uint8_t length;
			uint8_t duration;
			void (CPU::*handler)(uint8_t opcode, const uint8_t* operands);
		};

		enum Flags
		{
			FLAG_ZERO = 128,
			FLAG_SUBTRACT = 64,
			FLAG_HALF_CARRY = 32,
			FLAG_CARRY = 16
		};

		static Instruction instructionMap[256];

		Registers registers;

		Memory& memory;

		bool ime;

	public:
		CPU(Memory& memory);
		
		void Reset();
		void ExecuteNextInstruction();

	private:

		DMG_INLINE void CPU::SetFlag(Flags flag, bool state)
		{
			if (state)
				registers.f |= flag;
			else
				registers.f &= ~flag;
		}

		DMG_INLINE bool CPU::GetFlag(Flags flag)
		{
			return (registers.f & flag) == flag;
		}

		void WriteStackByte(uint8_t value);
		void WriteStackShort(uint16_t value);

		uint8_t ReadSourceValue(uint8_t opcode);
		
		/* CPU control */
		void nop(uint8_t opcode, const uint8_t* operands) { }
		void enable_interupts(uint8_t opcode, const uint8_t* operands) { ime = true; }
		void disable_interrupts(uint8_t opcode, const uint8_t* operands) { ime = false; }

		void jump(uint8_t opcode, const uint8_t* operands);
		void jump_to_offset(uint8_t opcode, const uint8_t* operands);
		void jump_to_offset_conditional(uint8_t opcode, const uint8_t* operands);

		void call(uint8_t opcode, const uint8_t* operands);
		void restart(uint8_t opcode, const uint8_t* operands);

		/* 8-bit ALU */
		void alu_add(uint8_t opcode, const uint8_t* operands);
		void alu_adc(uint8_t opcode, const uint8_t* operands);
		void alu_sub(uint8_t opcode, const uint8_t* operands);
		void alu_sbc(uint8_t opcode, const uint8_t* operands);
		void alu_and(uint8_t opcode, const uint8_t* operands);
		void alu_or(uint8_t opcode, const uint8_t* operands);
		void alu_xor(uint8_t opcode, const uint8_t* operands);
		void alu_cmp(uint8_t opcode, const uint8_t* operands);
		void alu_inc(uint8_t opcode, const uint8_t* operands);
		void alu_dec(uint8_t opcode, const uint8_t* operands);
		void alu_swap(uint8_t opcode, const uint8_t* operands);
		void alu_complement(uint8_t opcode, const uint8_t* operands);

		void encode_bcd(uint8_t opcode, const uint8_t* operands);

		/* 8-bit loads */
		void load_constant(uint8_t opcode, const uint8_t* operands);
		void load_memory_to_memory(uint8_t opcode, const uint8_t* operands);
		void load_accumulator_to_memory(uint8_t opcode, const uint8_t* operands);
		void load_memory_to_accumulator(uint8_t opcode, const uint8_t* operands);
		void load_accumulator_to_io_register(uint8_t opcode, const uint8_t* operands);
		void load_io_register_to_accumulator(uint8_t opcode, const uint8_t* operands);

		/* 16-bit loads */
		void load_constant_16bit(uint8_t opcode, const uint8_t* operands);
		void load_hl_to_sp(uint8_t opcode, const uint8_t* operands);
		void load_sp_plus_constant_to_hl(uint8_t opcode, const uint8_t* operands);
		void load_sp_to_memory(uint8_t opcode, const uint8_t* operands);
	};
}

#endif