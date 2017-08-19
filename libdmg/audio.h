#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "tonegenerator.h"

namespace libdmg
{
	class Memory;

	class Audio
	{

	private:
		Memory& memory;

		bool enabled;

		uint64_t ticks;
		uint32_t frameSequencerTicks;

		ToneGenerator sound1, sound2;

	public:
		Audio(Memory& memory);

		void Reset();
		void Sync(const uint64_t& targetTicks);

		ToneGenerator* Sound1() { return &sound1; }
		ToneGenerator* Sound2() { return &sound2; }

	private:
		void Step();
		void StepFrameSequencer();
	};
}

#endif