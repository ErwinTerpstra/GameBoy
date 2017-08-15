#ifndef _INPUT_H_
#define _INPUT_H_

#include "environment.h"

#include "memorypointer.h"
#include "memorybank.h"

namespace libdmg
{
	class CPU;
	class Memory;

	class Input : public MemoryBank
	{
	public:

		enum Button
		{
			BUTTON_DPAD_RIGHT	= 0,
			BUTTON_DPAD_LEFT	= 1,
			BUTTON_DPAD_UP		= 2,
			BUTTON_DPAD_DOWN	= 3,

			BUTTON_A			= 4,
			BUTTON_B			= 5,
			BUTTON_SELECT		= 6,
			BUTTON_START		= 7,
		};

	private:
		static const uint8_t DPAD_BIT = 4;
		static const uint8_t BUTTONS_BIT = 5;


		CPU& cpu;

		uint8_t buttons;

		uint8_t joypadRegister;
	public:
		Input(CPU& cpu);

		void SetButtonState(Button button, bool state);

		uint8_t ReadByte(uint16_t address) const;
		void WriteByte(uint16_t address, uint8_t value);
	};
}

#endif