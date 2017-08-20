#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "tonegenerator.h"
#include "ringbuffer.h"

namespace libdmg
{
	class Memory;

	class Audio
	{

	private:
		const uint16_t BUFFER_SIZE = 1024 * 32;

		Memory& memory;

		bool enabled;

		uint64_t ticks;
		uint32_t frameSequencerTicks;
		
		uint32_t sampleTimer;
		uint32_t samplePeriod;

		ToneGenerator sound1, sound2;

		RingBuffer outputBuffer;

	public:
		Audio(Memory& memory);

		void Reset();
		void Sync(const uint64_t& targetTicks);

		void SetOutputFrequency(uint32_t frequency);
		uint32_t GetOutputFrequency() const;

		ToneGenerator* Sound1() { return &sound1; }
		ToneGenerator* Sound2() { return &sound2; }

		RingBuffer& GetOutputBuffer() { return outputBuffer; }

	private:
		void Step();
		void StepFrameSequencer();
		void SampleOutput();
	};
}

#endif