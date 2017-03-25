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
	uint8_t joypad = 0xC0;

	if (!READ_BIT(*joypadRegister, BUTTONS_BIT))
		joypad |= ~(buttons >> 4) & 0x0F;
	else if (!READ_BIT(*joypadRegister, DPAD_BIT))
		joypad |= ~(buttons & 0x0F) & 0x0F;
	else
		joypad |= 0x0F;

	joypad |= (*joypadRegister & 0x30);

	*joypadRegister = joypad;
}

void Input::SetButtonState(Button button, bool state)
{
	uint8_t prevState = buttons;

	buttons = SET_BIT_IF(buttons, button, state);

	if (state && !READ_BIT(prevState, button))
		cpu.RequestInterrupt(CPU::INT_JOYPAD);
}