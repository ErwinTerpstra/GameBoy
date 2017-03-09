// WinBoy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "libdmg.h"

#include "Window/window.h"
#include "Window/buffer.h"
#include "Window/gdibufferallocator.h"

#define ROM_FILE "../roms/Tetris (World).gb"

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

	FILE* handle;
	errno_t error = fopen_s(&handle, ROM_FILE, "rb");

	if (error != 0)
	{
		printf("Failed to read ROM file: %s!\n", ROM_FILE);
		return 1;
	}

	size_t romSize = fread(romBuffer, 1, GB_MAX_CARTRIDGE_SIZE, handle);

	fclose(handle);

	printf("[WinBoy]: Rom file read with %u bytes.\n", romSize);
	
	Memory memory(memoryBuffer);
	Cartridge cartridge(romBuffer);

	Emulator emulator(memory, cartridge);
	emulator.Boot();

	Window window(hInstance, "WinBoyWindow");
	window.Create("WinBoy", GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);

	GDIBufferAllocator bufferAllocator(window.handle);
	Buffer frameBuffer(&bufferAllocator);
	frameBuffer.Allocate(GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, Buffer::BGR24);

	window.Show(nCmdShow);

	while (true)
	{
		for (uint32_t step = 0; step < (1 << 16); ++step)
			emulator.Step();

		Sleep(1);

		window.DrawBuffer(frameBuffer, bufferAllocator);
		window.ProcessMessages();
	}

	delete[] romBuffer;
	delete[] memoryBuffer;

    return 0;
}

