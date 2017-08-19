#include "audio.h"

#include "gameboy.h"
#include "memory.h"
#include "util.h"
#include "debug.h"

using namespace libdmg;

Audio::Audio(Memory& memory) : memory(memory),
	sound1(memory), sound2(memory),
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
	if ((((uint32_t) ticks) % (GB_CLOCK_FREQUENCY / GB_SOUND_CLOCK_PERIOD)) == 0)
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

	if ((frameSequencerTicks % 8) == 7)
	{
		sound1.StepVolumeEnvelope();
		sound2.StepVolumeEnvelope();
	}

	if ((frameSequencerTicks % 4) == 2)
		sound1.StepSweep();
	
	// Update the state register
	uint8_t state = memory.ReadByte(GB_REG_NR52) & 0xF0;
	state = SET_BIT_IF(state, 0, sound1.Enabled());
	state = SET_BIT_IF(state, 1, sound2.Enabled());

	memory.WriteByte(GB_REG_NR52, state);

	// Increase the sequencer tick count
	++frameSequencerTicks;
}