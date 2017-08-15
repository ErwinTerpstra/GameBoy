#include "input.h"

#include "gameboy.h"
#include "util.h"

#include "cpu.h"
#include "memory.h"

using namespace libdmg;

Input::Input(CPU& cpu) : cpu(cpu), buttons(0)
{

}

void Input::SetButtonState(Button button, bool state)
{
	uint8_t prevState = buttons;

	buttons = SET_BIT_IF(buttons, button, state);

	if (state && !READ_BIT(prevState, button))
		cpu.RequestInterrupt(CPU::INT_JOYPAD);
}

uint8_t Input::ReadByte(uint16_t address) const
{
	return joypadRegister;
}

void Input::WriteByte(uint16_t address, uint8_t value)
{
	joypadRegister = 0xC0;

	if (!READ_BIT(value, BUTTONS_BIT))
		joypadRegister |= ~(buttons >> 4) & 0x0F;
	else if (!READ_BIT(value, DPAD_BIT))
		joypadRegister |= ~(buttons & 0x0F) & 0x0F;
	else
		joypadRegister |= 0x0F;

	joypadRegister |= (value & 0x30);
}