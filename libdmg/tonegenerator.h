#ifndef _TONE_GENERATOR_H_
#define _TONE_GENERATOR_H_

#include "environment.h"
#include "memorybank.h"

namespace libdmg
{
	class Audio;
	class Memory;

	class ToneGenerator : public MemoryBank
	{
	private:
		Memory& memory;

		bool enabled;
		bool sweepEnabled;
		bool lengthCounterEnabled;

		uint8_t lengthCounter;
		uint8_t wavePatternDuty;

		uint8_t sweepTimer;
		uint8_t sweepPeriod;
		uint8_t sweepDirection;
		uint8_t sweepShift;
		uint8_t sweepRegister;

		uint8_t volume;
		uint8_t volumeEnvelopeDirection;
		uint8_t volumeEnvelopeTimer;
		uint8_t volumeEnvelopePeriod;
		uint8_t volumeEnvelopeRegister;

		uint16_t frequency;
		uint16_t shadowFrequency;

	public:
		ToneGenerator(Memory& memory);

		void StepSweep();
		void StepLengthClock();
		void StepVolumeEnvelope();

		void RestartSweep();
		void UpdateSweepFrequency(bool saveFrequency);

		uint8_t ReadByte(uint16_t address) const;
		void WriteByte(uint16_t address, uint8_t value);

		bool Enabled() const { return enabled; }
	};
}

#endif