#ifndef _CPU_H_
#define _CPU_H_

#include "environment.h"
#include "util.h"

#include "memorypointer.h"

namespace libdmg
{
	class Memory;

	class Pointer;
	class NativePointer;

	class CPU
	{
	public:
		enum Interrupt
		{
			INT_VBLANK = 0,
			INT_LCD_STAT = 1,
			INT_TIMER = 2,
			INT_SERIAL = 3,
			INT_JOYPAD = 4
		};

	public:
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
			uint8_t opcode;
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

		static const Instruction INSTRUCTION_MAP[256];
		static const Instruction PREFIXED_INSTRUCTION_MAP[256];
	
	private:
		static const uint16_t INTERRUPT_VECTORS[];
		
		static const uint8_t GB_ISR_DURATION = 5;

		Memory& memory;

		Registers registers;

		uint64_t ticks;

		bool interruptMasterEnable;
		bool halted;
		bool stopped;

		NativePointer* nativePointer;
		MemoryPointer* memoryPointer;

		MemoryPointer interruptEnable;
		MemoryPointer interruptFlags;

	public:
		CPU(Memory& memory);
		~CPU();
		
		void Reset();
		void Resume();
		
		const Instruction& ExecuteNextInstruction();
		void TestInterrupts();

		void RequestInterrupt(Interrupt interrupt);

		bool InterruptMasterEnable() const { return interruptMasterEnable; }
		bool Halted() const { return halted; }
		bool Stopped() const { return stopped; }

		const Registers& GetRegisters() const { return registers; }
		const uint64_t& Ticks() const { return ticks; }
	
	private:
		NativePointer* CreateNativePointer(uint8_t* ptr);
		MemoryPointer* CreateMemoryPointer(uint16_t address);

		void ExecuteInterrupt(Interrupt interrupt);

		// Flag register manipulation
		DMG_INLINE void SetFlag(Flags flag, bool state) { registers.f = SET_MASK_IF(registers.f, flag, state); }
		DMG_INLINE bool GetFlag(Flags flag) { return READ_MASK(registers.f, flag); }

		/* Stack utilities*/
		uint8_t ReadStackByte();
		uint16_t ReadStackShort();

		void WriteStackByte(uint8_t value);
		void WriteStackShort(uint16_t value);

		/* Memory read/writing */
		uint8_t ReadSourceValue(uint8_t opcode, const uint8_t* operands) const;
		Pointer* GetSourcePointer(uint8_t opcode);
		
		/* ALU utilities */

		/* CPU control */
		void nop(uint8_t opcode, const uint8_t* operands) { }
		void stop(uint8_t opcode, const uint8_t* operands);
		void halt(uint8_t opcode, const uint8_t* operands);
		void enable_interupts(uint8_t opcode, const uint8_t* operands);
		void disable_interrupts(uint8_t opcode, const uint8_t* operands);

		void jump(uint8_t opcode, const uint8_t* operands);
		void jump_conditional(uint8_t opcode, const uint8_t* operands);
		void jump_to_hl(uint8_t opcode, const uint8_t* operands);
		void jump_to_offset(uint8_t opcode, const uint8_t* operands);
		void jump_to_offset_conditional(uint8_t opcode, const uint8_t* operands);

		void call(uint8_t opcode, const uint8_t* operands);
		void call_conditional(uint8_t opcode, const uint8_t* operands);
		void restart(uint8_t opcode, const uint8_t* operands);

		void return_default(uint8_t opcode, const uint8_t* operands);
		void return_conditional(uint8_t opcode, const uint8_t* operands);
		void return_enable_interrupts(uint8_t opcode, const uint8_t* operands);

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
		void alu_complement(uint8_t opcode, const uint8_t* operands);
		void alu_complement_carry(uint8_t opcode, const uint8_t* operands);
		void alu_set_carry(uint8_t opcode, const uint8_t* operands);

		void adjust_bcd(uint8_t opcode, const uint8_t* operands);

		/* 16-bit alu */
		void alu_inc_16bit(uint8_t opcode, const uint8_t* operands);
		void alu_dec_16bit(uint8_t opcode, const uint8_t* operands);
		void alu_add_hl_16bit(uint8_t opcode, const uint8_t* operands);
		void alu_add_sp_constant(uint8_t opcode, const uint8_t* operands);

		/* 8-bit loads */
		void load_constant(uint8_t opcode, const uint8_t* operands);
		void load_memory_to_memory(uint8_t opcode, const uint8_t* operands);
		void load_accumulator_to_memory(uint8_t opcode, const uint8_t* operands);
		void load_memory_to_accumulator(uint8_t opcode, const uint8_t* operands);
		void load_accumulator_to_constant_io_register(uint8_t opcode, const uint8_t* operands);
		void load_constant_io_register_to_accumulator(uint8_t opcode, const uint8_t* operands);
		void load_accumulator_to_c_plus_io_register(uint8_t opcode, const uint8_t* operands);
		void load_c_plus_io_register_to_accumulator(uint8_t opcode, const uint8_t* operands);

		/* 16-bit loads */
		void load_constant_16bit(uint8_t opcode, const uint8_t* operands);
		void load_hl_to_sp(uint8_t opcode, const uint8_t* operands);
		void load_sp_plus_constant_to_hl(uint8_t opcode, const uint8_t* operands);
		void load_sp_to_memory(uint8_t opcode, const uint8_t* operands);
		void load_accumulator_to_memory_16bit(uint8_t opcode, const uint8_t* operands);
		void load_memory_to_accumulator_16bit(uint8_t opcode, const uint8_t* operands);
		void push_stack_16bit(uint8_t opcode, const uint8_t* operands);
		void pop_stack_16bit(uint8_t opcode, const uint8_t* operands);

		/* Rotate & shift */
		void rotate_accumulator_left(uint8_t opcode, const uint8_t* operands);
		void rotate_accumulator_right(uint8_t opcode, const uint8_t* operands);
		void rotate_accumulator_left_circular(uint8_t opcode, const uint8_t* operands);
		void rotate_accumulator_right_circular(uint8_t opcode, const uint8_t* operands);

		/* Prefixed instructions */
		void rotate_left(uint8_t opcode, const uint8_t* operands);
		void rotate_right(uint8_t opcode, const uint8_t* operands);
		void rotate_left_circular(uint8_t opcode, const uint8_t* operands);
		void rotate_right_circular(uint8_t opcode, const uint8_t* operands);

		void shift_left_arithmetically(uint8_t opcode, const uint8_t* operands);
		void shift_right_arithmetically(uint8_t opcode, const uint8_t* operands);
		void shift_right_logically(uint8_t opcode, const uint8_t* operands);
		void swap(uint8_t opcode, const uint8_t* operands);
		void test_bit(uint8_t opcode, const uint8_t* operands);
		void reset_bit(uint8_t opcode, const uint8_t* operands);
		void set_bit(uint8_t opcode, const uint8_t* operands);
	};
}

#endif