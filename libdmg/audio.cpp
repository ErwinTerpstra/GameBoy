#include "audio.h"

#include "gameboy.h"
#include "memory.h"
#include "util.h"
#include "debug.h"

using namespace libdmg;

Audio::Audio(Memory& memory) : memory(memory),
	outputBuffer(BUFFER_SIZE),
	sound1(true), sound2(false),
	samplePeriod(128), sampleTimer(0.0f),
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

void Audio::SetOutputFrequency(uint32_t frequency)
{
	samplePeriod = GB_CLOCK_FREQUENCY / (float) frequency;
}

uint32_t Audio::GetOutputFrequency() const 
{
	return GB_CLOCK_FREQUENCY / samplePeriod; 
}

void Audio::Step()
{
	if ((((uint32_t) ticks) % (GB_CLOCK_FREQUENCY / GB_FRAME_SEQUENCER_PERIOD)) == 0)
		StepFrameSequencer();

	sound1.StepFrequency();
	sound2.StepFrequency();
	
	++sampleTimer;
	if (sampleTimer > samplePeriod)
	{
		SampleOutput();
		sampleTimer -= samplePeriod;
	}

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

void Audio::SampleOutput()
{
	uint8_t NR50 = memory.ReadByte(GB_REG_NR50);
	uint8_t NR51 = memory.ReadByte(GB_REG_NR51);
	uint8_t NR52 = memory.ReadByte(GB_REG_NR52);

	// Check if the audio chip is enabled
	if (READ_BIT(NR52, 7))
	{
		uint8_t S01 = 127;
		uint8_t S02 = 127;

		// Extract volumes from NR50 register
		uint8_t S01Volume = NR50 & 0x07;
		uint8_t S02Volume = (NR50 >> 4) & 0x07;
		
		// Route sound channel outputs to S01 and S02
		if (sound1.Enabled() && READ_BIT(NR51, 0))
		{
			if (sound1.GetOutput())
				S01 += sound1.GetVolume() * S01Volume;
			else
				S01 -= sound1.GetVolume() * S01Volume;
		}

		if (sound1.Enabled() && READ_BIT(NR51, 4))
		{
			if (sound1.GetOutput())
				S02 += sound1.GetVolume() * S02Volume;
			else
				S02 -= sound1.GetVolume() * S02Volume;
		}

		if (sound2.Enabled() && READ_BIT(NR51, 1))
		{
			if (sound2.GetOutput())
				S01 += sound2.GetVolume() * S01Volume;
			else
				S01 -= sound2.GetVolume() * S01Volume;
		}

		if (sound2.Enabled() && READ_BIT(NR51, 5))
		{
			if (sound2.GetOutput())
				S02 += sound2.GetVolume() * S02Volume;
			else
				S02 -= sound2.GetVolume() * S02Volume;
		}

		outputBuffer.WriteByte(S01);
		outputBuffer.WriteByte(S02);

		//assert(!outputBuffer.Full());
	}
}