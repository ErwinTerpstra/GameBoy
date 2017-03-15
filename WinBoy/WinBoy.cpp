// WinBoy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "libdmg.h"

#include "Window/window.h"
#include "Window/buffer.h"
#include "Window/gdibufferallocator.h"

#include "inputmanager.h"

#define ROM_FILE "../roms/Tetris (World).gb"
#define DISASSEMBLY_LENGTH 10

using namespace libdmg;
using namespace WinBoy;

int main()
{
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	uint8_t* romBuffer = new uint8_t[GB_MAX_CARTRIDGE_SIZE];
	uint8_t* memoryBuffer = new uint8_t[GB_MAIN_MEM_SIZE];
	uint8_t* videoBuffer = new uint8_t[GB_VIDEO_BUFFER_WIDTH * GB_VIDEO_BUFFER_HEIGHT];

	memset(romBuffer, 0, GB_MAX_CARTRIDGE_SIZE);
	memset(memoryBuffer, 0, GB_MAIN_MEM_SIZE);
	memset(videoBuffer, 0, GB_VIDEO_BUFFER_WIDTH * GB_VIDEO_BUFFER_HEIGHT);

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
			for (uint32_t step = 0; step < (1 << 16); ++step)
			{
				emulator.Step();

				const CPU::Registers& registers = cpu.GetRegisters();
				if (registers.pc == 0x0312)
				{
					paused = true;
					Debug::Print("[WinBoy]: Breakpoint hit\n");
					break;
				}
			}
		}

		Sleep(1);

		window.DrawBuffer(frameBuffer, bufferAllocator);
		window.ProcessMessages();
	}

	delete[] romBuffer;
	delete[] memoryBuffer;

    return 0;
}

