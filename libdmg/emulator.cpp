#include "emulator.h"

#include "cpu.h"
#include "memory.h"
#include "cartridge.h"
#include "videocontroller.h"

#include "debug.h"

using namespace libdmg;

Emulator::Emulator(CPU& cpu, Memory& memory, Cartridge& cartridge, VideoController& videoController) : 
	cpu(cpu), memory(memory), cartridge(cartridge), videoController(videoController)
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
	videoController.Step();
}