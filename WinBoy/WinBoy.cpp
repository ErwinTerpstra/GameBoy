// WinBoy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "libdmg.h"

#include "Window/window.h"
#include "Window/buffer.h"
#include "Window/gdibufferallocator.h"
#include "Window/color.h"

#include "inputmanager.h"

#define ROM_FILE "../roms/Tetris (World).gb"
#define DISASSEMBLY_LENGTH 10

using namespace libdmg;
using namespace WinBoy;

const Color COLORS[] = 
{
	{ 1.0f, 1.0f, 1.0f, 1.0f },
	{ 0.6f, 0.6f, 0.6f, 1.0f },
	{ 0.3f, 0.3f, 0.3f, 1.0f },
	{ 0.0f, 0.0f, 0.0f, 1.0f },
};

int main()
{
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	uint8_t* romBuffer = new uint8_t[GB_MAX_CARTRIDGE_SIZE];
	uint8_t* memoryBuffer = new uint8_t[GB_MAIN_MEM_SIZE];

	uint16_t videoBufferSize = GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT / 4;
	uint8_t* videoBuffer = new uint8_t[videoBufferSize];

	memset(romBuffer, 0, GB_MAX_CARTRIDGE_SIZE);
	memset(memoryBuffer, 0, GB_MAIN_MEM_SIZE);
	memset(videoBuffer, 0, videoBufferSize);

	FILE* handle;
	errno_t error = fopen_s(&handle, ROM_FILE, "rb");

	if (error != 0)
	{
		Debug::Print("Failed to read ROM file: %s!\n", ROM_FILE);
		return 1;
	}

	size_t romSize = fread(romBuffer, 1, GB_MAX_CARTRIDGE_SIZE, handle);

	fclose(handle);

	Debug::Print("[WinBoy]: Rom file read with %u bytes.\n", romSize);
	
	Memory memory(memoryBuffer);
	CPU cpu(memory);
	Cartridge cartridge(romBuffer);
	VideoController videoController(cpu, memory, videoBuffer);

	Emulator emulator(cpu, memory, cartridge, videoController);
	emulator.Boot();

	InputManager& inputManager = InputManager::Instance();

	Window window(hInstance, "WinBoyWindow");
	window.Create("WinBoy", GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);

	GDIBufferAllocator bufferAllocator(window.handle);
	Buffer frameBuffer(&bufferAllocator);
	frameBuffer.Allocate(GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, Buffer::BGR24);

	window.Show(nCmdShow);

	bool paused = false;
	bool step = false;

	VideoController::Mode prevVideoControllerMode = videoController.CurrentMode();

	uint16_t breakpoints[] =
	{
		0x0040,
	};

	while (true)
	{
		if (inputManager.GetKeyDown('B'))
		{
			paused = true;
			Debug::Print("[WinBoy]: Execution paused.\n");
			emulator.PrintDisassembly(DISASSEMBLY_LENGTH);
			emulator.PrintRegisters();
		}

		if (paused)
		{
			if (inputManager.GetKeyDown('R'))
			{
				Debug::Print("[WinBoy]: Execution resumed.\n");
				paused = false;
			}

			if (inputManager.GetKeyDown('S'))
			{
				Debug::Print("[WinBoy]: Executing next instruction...\n");
				emulator.Step();
				emulator.PrintDisassembly(DISASSEMBLY_LENGTH);
				emulator.PrintRegisters();
			}

			if (inputManager.GetKeyDown('P'))
			{
				emulator.PrintDisassembly(DISASSEMBLY_LENGTH);
				emulator.PrintRegisters();
			}
		}
		else
		{
			for (uint32_t step = 0; step < (1 << 16) && !paused; ++step)
			{
				emulator.Step();

				const CPU::Registers& registers = cpu.GetRegisters();
				//if (registers.pc == 0x0040)
				//if (registers.pc == 0x2882)
				//if (registers.pc == 0xFFB6)
				//if (FALSE)
				for (uint8_t breakpointIdx = 0; breakpointIdx < sizeof(breakpoints) / sizeof(uint16_t); ++breakpointIdx)
				{
					if (registers.pc == breakpoints[breakpointIdx])
					{
						paused = true;
						Debug::Print("[WinBoy]: Breakpoint hit at 0x%04X\n", registers.pc);
					}
				}
			}
		}

		if (videoController.CurrentMode() == VideoController::MODE_VBLANK && prevVideoControllerMode != VideoController::MODE_VBLANK)
		{
			for (uint8_t y = 0; y < GB_SCREEN_HEIGHT; ++y)
			{
				for (uint8_t x = 0; x < GB_SCREEN_WIDTH; ++x)
				{
					uint16_t pixel = (y * GB_SCREEN_WIDTH) + x;
					uint8_t videoByte = videoBuffer[pixel / 4];
					uint8_t videoBit = (pixel % 4) << 1;
					uint8_t color = (videoByte >> videoBit) & 0x03;

					frameBuffer.SetPixel(x, y, COLORS[color]);
				}
			}

			window.DrawBuffer(frameBuffer, bufferAllocator);
		}

		prevVideoControllerMode = videoController.CurrentMode();

		window.ProcessMessages();

		Sleep(10);
	}

	delete[] romBuffer;
	delete[] memoryBuffer;

    return 0;
}

