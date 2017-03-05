#include "emulator.h"

#include "debug.h"

using namespace libdmg;

Emulator::Emulator(Memory& memory, Cartridge& cartridge) : cpu(memory), memory(memory), cartridge(cartridge)
{

}

void Emulator::Boot()
{
	// Currently, only support ROM only cartridges
	assert(cartridge.header->cartridgeHardware == 0);

	cpu.Reset();

	// Copy lower 16K of rom to main memory
	memory.WriteBuffer(cartridge.rom, 0x0000, 0x4000);

	// Copy upper 16K of rom to main memory
	// TODO: Handle memory bank controllers
	memory.WriteBuffer(cartridge.rom + 0x4000, 0x4000, 0x4000);
}

void Emulator::Step()
{
	cpu.ExecuteNextInstruction();
}