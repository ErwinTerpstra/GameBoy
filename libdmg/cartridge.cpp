#include "cartridge.h"

using namespace libdmg;

// Available RAM sizes, in KB
const uint8_t Cartridge::RAM_SIZES[] = { 0, 2, 8, 32, 128, 64 };

Cartridge::Cartridge(uint8_t* rom, uint8_t* ram) : rom(rom), ram(ram)
{
	header = reinterpret_cast<const Header*>(rom + HEADER_OFFSET);
}