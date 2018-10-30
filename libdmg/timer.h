#ifndef _TIMER_H_
#define _TIMER_H_

#include "environment.h"

#include "memorypointer.h"

namespace libdmg
{
	class CPU;
	class Memory;

	class Timer
	{
	private:
		static const uint16_t DIV_REGISTER_DIVIDER;
		static const uint16_t TIMER_DIVIDERS[4];

		CPU& cpu;
		Memory& memory;

		uint64_t ticks;

		MemoryPointer divRegister;
		MemoryPointer timerCounterRegister;
		MemoryPointer timerControlRegister;
		MemoryPointer timerModuloRegister;

		uint16_t divRegisterCycles;
		uint16_t timerCycles;

	public:
		Timer(CPU& cpu, Memory& memory);

		void Reset();
		void Sync(const uint64_t& targetTicks);
		void PerformCycle();

	};
}

#endif