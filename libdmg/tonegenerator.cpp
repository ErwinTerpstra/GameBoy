#include "tonegenerator.h"

#include "util.h"
#include "audio.h"
#include "memory.h"

using namespace libdmg;

ToneGenerator::ToneGenerator(Memory& memory, uint16_t NRx0, uint16_t NRx1, uint16_t NRx2, uint16_t NRx3, uint16_t NRx4) :
	enabled(true), memory(memory), 
	NRx0(NRx0), NRx1(NRx1), NRx2(NRx2), NRx3(NRx3), NRx4(NRx4)
{

}

uint8_t ToneGenerator::ReadByte(uint16_t address) const
{
	return 0;
}

void ToneGenerator::WriteByte(uint16_t address, uint8_t value)
{

}

void ToneGenerator::StepLengthClock()
{
	if (READ_BIT(memory.ReadByte(NRx4), 6))
	{
		uint8_t lengthReg = memory.ReadByte(NRx1);
		
		uint8_t length = lengthReg & 0x3F;
		++length;

		enabled = length < 64;

		memory.WriteByte(NRx1, (lengthReg & 0xE0) | (length & 0x3F));
	}
}

void ToneGenerator::StepVolumeEnvelope()
{
	
}