#include "timer.h"

#include "gameboy.h"

#include "cpu.h"
#include "memory.h"

using namespace libdmg;

const uint16_t Timer::DIV_REGISTER_DIVIDER = 256;
const uint16_t Timer::TIMER_DIVIDERS[] = { 1024, 16, 64, 256 };

Timer::Timer(CPU& cpu, Memory& memory) : cpu(cpu), memory(memory)
{
	divRegister = memory.RetrievePointer(GB_REG_DIV);
	timerCounterRegister = memory.RetrievePointer(GB_REG_TIMA);
}

void Timer::Reset()
{
	ticks = 0;
	divRegisterCycles = 0;
	timerCycles = 0;
}

void Timer::Sync()
{
	const uint64_t& targetTicks = cpu.Ticks();

	while (ticks < targetTicks)
		PerformCycle();
}

void Timer::PerformCycle()
{
	// DIV register
	if (++divRegisterCycles == DIV_REGISTER_DIVIDER)
	{
		++(*divRegister);
		divRegisterCycles = 0;
	}

	// Timer
	uint8_t timerControl = memory.ReadByte(GB_REG_TAC);
	uint16_t timerDivider = TIMER_DIVIDERS[timerControl & 0x3];

	// Check if the timer is enabled
	if (READ_BIT(timerControl, 0x4))
	{
		// Increment the cycle counter and check if we should increase the counter register111
		if (++timerCycles == timerDivider)
		{
			// Increment the counter register and check if it overflowed
			if (++(*timerCounterRegister) == 0)
			{
				// Reset the counter to the timer modulo value
				*timerCounterRegister = memory.ReadByte(GB_REG_TMA); 

				// Request the timer interrupt
				cpu.RequestInterrupt(CPU::INT_TIMER);
			}

			timerCycles = 0;
		}
	}

	++ticks;
}