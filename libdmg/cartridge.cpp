#include "cartridge.h"

using namespace libdmg;

// Available RAM sizes, in KB
const uint8_t Cartridge::RAM_SIZES[] = { 0, 2, 8, 32, 128, 64 };

Cartridge::Cartridge(uint8_t* buffer) : rom(buffer)
{
	header = reinterpret_cast<const Header*>(rom + HEADER_OFFSET);
}