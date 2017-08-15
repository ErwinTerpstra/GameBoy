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

		uint16_t NRx0, NRx1, NRx2, NRx3, NRx4;

	public:
		ToneGenerator(Memory& memory, uint16_t NRx0, uint16_t NRx1, uint16_t NRx2, uint16_t NRx3, uint16_t NRx4);

		void StepLengthClock();
		void StepVolumeEnvelope();

		uint8_t ReadByte(uint16_t address) const;
		void WriteByte(uint16_t address, uint8_t value);

		bool Enabled() const { return enabled; }
	};
}

#endif