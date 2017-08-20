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
		bool hasSweep;
		bool enabled;
		bool sweepEnabled;
		bool lengthCounterEnabled;


		uint8_t wavePatternDuty;
		uint8_t wavePatternIndex;

		uint8_t lengthCounter;

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
		uint16_t frequencyTimer;

	public:
		ToneGenerator(bool hasSweep);

		uint8_t ReadByte(uint16_t address) const;
		void WriteByte(uint16_t address, uint8_t value);

		void StepFrequency();

		void StepSweep();
		void StepLengthClock();
		void StepVolumeEnvelope();

		uint8_t GetOutput() const;
		uint8_t GetVolume() const;

		bool Enabled() const { return enabled; }

	private:
		void RestartSweep();
		void UpdateSweepFrequency(bool saveFrequency);
	};
}

#endif