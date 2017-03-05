#include "cartridge.h"

using namespace libdmg;


Cartridge::Cartridge(const uint8_t* buffer) : rom(buffer)
{
	header = reinterpret_cast<const Header*>(rom + HEADER_OFFSET);
}