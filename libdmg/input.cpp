#include "input.h"

#include "gameboy.h"
#include "util.h"

#include "cpu.h"
#include "memory.h"

using namespace libdmg;

Input::Input(CPU& cpu, Memory& memory) : cpu(cpu), memory(memory), buttons(0)
{
	joypadRegister = memory.RetrievePointer(GB_REG_JOYP);
}

void Input::Update()
{
	if (!READ_BIT(*joypadRegister, BUTTONS_BIT))
	{
		*joypadRegister = ~((buttons >> 4) | (1 << BUTTONS_BIT));
	}
	else if (!READ_BIT(*joypadRegister, DPAD_BIT))
	{
		*joypadRegister = ~((buttons & 0x0F) | (1 << DPAD_BIT));
	}
	else
		*joypadRegister = 0x3F;
}

void Input::SetButtonState(Button button, bool state)
{
	uint8_t prevState = buttons;

	buttons = SET_BIT_IF(buttons, button, state);

	if (state && !READ_BIT(buttons, button))
		cpu.RequestInterrupt(CPU::INT_JOYPAD);
}