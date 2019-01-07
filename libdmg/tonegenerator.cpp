#include "tonegenerator.h"

#include "util.h"
#include "audio.h"
#include "memory.h"
#include "debug.h"

using namespace libdmg;

const uint8_t WAVEFORMS[] = 
{
	0b00000001,
	0b10000001,
	0b10000111,
	0b01111110
};

ToneGenerator::ToneGenerator(bool hasSweep) :
	hasSweep(hasSweep), enabled(false), sweepEnabled(false), lengthCounterEnabled(false),
	lengthCounter(0),
	sweepTimer(0), sweepPeriod(0), sweepShift(0),
	volume(0), volumeEnvelopeDirection(1), volumeEnvelopeTimer(0), volumeEnvelopePeriod(0),
	wavePatternDuty(2), wavePatternIndex(0),
	frequency(0), shadowFrequency(0), frequencyTimer(0)
{

}

uint8_t ToneGenerator::ReadByte(uint16_t address) const
{
	switch (address)
	{
		case 0x00:
			return sweepRegister;

		case 0x01:
			return (wavePatternDuty << 6);

		case 0x02:
			return volumeEnvelopeRegister;

		case 0x03:
			return 0;

		case 0x04:
			return (lengthCounterEnabled << 6);
	}
}

void ToneGenerator::WriteByte(uint16_t address, uint8_t value)
{
	switch (address)
	{
		case 0x00:
			assert(hasSweep);

			sweepPeriod = (value >> 4) & 0x03;
			sweepDirection = READ_BIT(value, 3);
			sweepShift = (value & 0x07);

			sweepRegister = value;

			RestartSweep();

			break;

		case 0x01:
			wavePatternDuty = (value >> 6) & 0x03;
			lengthCounter = value & 0x3A;
			break;

		case 0x02:
			volume = (value >> 4);
			volumeEnvelopeDirection = READ_BIT(value, 3);
			volumeEnvelopePeriod = (value & 0x07);

			volumeEnvelopeTimer = volumeEnvelopePeriod;

			volumeEnvelopeRegister = value;
			break;

		case 0x03:
			// The lower 8 bits of the 11 bit frequency;
			frequency = (frequency & 0xFF00) | value;
			break;

		case 0x04:
			if (READ_BIT(value, 7))
			{
				enabled = true;

				if (lengthCounter == 0)
					lengthCounter = 64;

				volumeEnvelopeTimer = volumeEnvelopePeriod;

				RestartSweep();

				if (volume == 0)
					enabled = false;
			}

			lengthCounterEnabled = READ_BIT(value, 6);

			// The lowest three bits are the upper three bits of the 11 bit frequency
			frequency = (frequency & 0xFF) | ((value & 0x7) << 8);

			break;

	}
}

void ToneGenerator::StepFrequency()
{
	--frequencyTimer;
	
	if (frequencyTimer == 0)
	{
		wavePatternIndex = (wavePatternIndex + 1) % 8;
		frequencyTimer = (2048 - frequency) << 2;
	}
}

void ToneGenerator::StepSweep()
{
	if (sweepEnabled && sweepPeriod != 0)
	{
		--sweepTimer;

		if (sweepTimer == 0)
		{
			UpdateSweepFrequency(true);
			sweepTimer = sweepPeriod;
		}
	}
}

void ToneGenerator::StepLengthClock()
{
	if (lengthCounterEnabled)
	{
		--lengthCounter;

		if (lengthCounter == 0)
		{
			enabled = false;
			lengthCounterEnabled = false;
		}
	}
}

void ToneGenerator::StepVolumeEnvelope()
{
	if (volumeEnvelopePeriod != 0)
	{
		--volumeEnvelopeTimer;

		if (volumeEnvelopeTimer == 0)
		{
			if (volumeEnvelopeDirection)
			{
				if (volume < 16)
					++volume;
			}
			else
			{
				if (volume > 0)
					--volume;
			}

			volumeEnvelopeTimer = volumeEnvelopePeriod;
		}
	}
}


uint8_t ToneGenerator::GetOutput() const
{
	return READ_BIT(WAVEFORMS[wavePatternDuty], wavePatternIndex);
}

uint8_t ToneGenerator::GetVolume() const
{
	return volume;
}

void ToneGenerator::RestartSweep()
{
	sweepEnabled = sweepPeriod || sweepShift;
	sweepTimer = sweepPeriod;
	shadowFrequency = frequency;

	if (sweepShift > 0)
		UpdateSweepFrequency(false);
}

void ToneGenerator::UpdateSweepFrequency(bool saveFrequency)
{
	uint16_t newFrequency = shadowFrequency;
	
	if (sweepDirection)
		newFrequency -= (shadowFrequency << sweepShift);
	else
		newFrequency += (shadowFrequency << sweepShift);

	if (newFrequency >= 2048)
	{
		enabled = false;
		sweepEnabled = false;
		return;
	}

	if (saveFrequency)
	{
		frequency = newFrequency;
		shadowFrequency = newFrequency;

		UpdateSweepFrequency(false);
	}

}