#ifndef _TIMER_H_
#define _TIMER_H_

#include "environment.h"

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

		uint8_t* divRegister;
		uint8_t* timerCounterRegister;

		uint16_t divRegisterCycles;
		uint16_t timerCycles;

	public:
		Timer(CPU& cpu, Memory& memory);

		void Reset();
		void Sync();
		void PerformCycle();

	};
}

#endif