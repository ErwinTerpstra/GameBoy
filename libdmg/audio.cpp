#include "audio.h"

#include "gameboy.h"
#include "memory.h"
#include "util.h"
#include "debug.h"

using namespace libdmg;

Audio::Audio(Memory& memory) : memory(memory),
	sound1(memory, GB_REG_NR10, GB_REG_NR11, GB_REG_NR12, GB_REG_NR13, GB_REG_NR14),
	sound2(memory,           0, GB_REG_NR21, GB_REG_NR22, GB_REG_NR23, GB_REG_NR24),
	ticks(0), frameSequencerTicks(0)
{

}

void Audio::Reset()
{
	ticks = 0;
	frameSequencerTicks = 0;
}

void Audio::Sync(const uint64_t& targetTicks)
{
	while (ticks < targetTicks)
		Step();
}

void Audio::Step()
{
	if ((((uint32_t) ticks) % (GB_CLOCK_FREQUENCY / GB_SOUND_CLOCK_FREQUENCY)) == 0)
		StepFrameSequencer();

	++ticks;
}

void Audio::StepFrameSequencer()
{
	if ((frameSequencerTicks % 2) == 0)
	{
		sound1.StepLengthClock();
		sound2.StepLengthClock();
	}
	
	// Update the state register
	uint8_t state = memory.ReadByte(GB_REG_NR52) & 0xF0;
	state = SET_BIT_IF(state, 0, sound1.Enabled());
	state = SET_BIT_IF(state, 1, sound2.Enabled());

	memory.WriteByte(GB_REG_NR52, state);

	// Increase the sequencer tick count
	++frameSequencerTicks;
}