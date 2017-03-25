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
#define SCALE_FACTOR 2

using namespace libdmg;
using namespace WinBoy;

const Color COLORS[] =
{
	{ 1.0f, 1.0f, 1.0f, 1.0f },
	{ 0.6f, 0.6f, 0.6f, 1.0f },
	{ 0.3f, 0.3f, 0.3f, 1.0f },
	{ 0.0f, 0.0f, 0.0f, 1.0f },
};

uint16_t breakpoints[] =
{
	0x0000,
	//0x034C,
	//0x029C,
	//0x030F,
	//0x2A08
};

uint16_t memoryBreakpoints[] =
{
	0x0000,
	//0xFF80,	// Current joypad state
	0xFF81, // Changed joypad bits
	0xFFC5, // Changed joypad bits (copy)
};

bool paused = false;
bool breakpointsEnabled = true;

uint8_t* romBuffer;
uint8_t* memoryBuffer;

uint16_t videoBufferSize;
uint8_t* videoBuffer;

Memory* memory;
CPU* cpu;
Cartridge* cartridge;
Video* video;
Input* input;

Emulator* emulator;

Window* window;

GDIBufferAllocator* bufferAllocator;
Buffer* frameBuffer;

int main()
{
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}

void DrawFrameBuffer();


void MemoryReadCallback(uint16_t address)
{
	if (!breakpointsEnabled)
		return;

	const CPU::Registers& registers = cpu->GetRegisters();

	for (uint8_t breakpointIdx = 0; breakpointIdx < sizeof(memoryBreakpoints) / sizeof(uint16_t); ++breakpointIdx)
	{
		if (address == memoryBreakpoints[breakpointIdx])
		{
			Debug::Print("[WinBoy]: Memory read breakpoint for 0x%04X hit at 0x%04X\n", address, registers.pc);
			paused = true;
			break;
		}
	}
}

void MemoryWriteCallback(uint16_t address)
{
	if (!breakpointsEnabled)
		return;

	const CPU::Registers& registers = cpu->GetRegisters();

	for (uint8_t breakpointIdx = 0; breakpointIdx < sizeof(memoryBreakpoints) / sizeof(uint16_t); ++breakpointIdx)
	{
		if (address == memoryBreakpoints[breakpointIdx])
		{
			Debug::Print("[WinBoy]: Memory write breakpoint for 0x%04X hit at 0x%04X\n", address, registers.pc);
			paused = true;
			break;
		}
	}
}

void VBlankCallback()
{
	DrawFrameBuffer();
}

void DrawFrameBuffer()
{
	for (uint16_t y = 0; y < GB_SCREEN_HEIGHT; ++y)
	{
		for (uint16_t x = 0; x < GB_SCREEN_WIDTH; ++x)
		{
			uint16_t pixel = ((GB_SCREEN_HEIGHT - 1 - y) * GB_SCREEN_WIDTH) + x;
			uint8_t videoByte = videoBuffer[pixel / 4];
			uint8_t videoBit = (pixel % 4) << 1;
			uint8_t color = (videoByte >> videoBit) & 0x03;

			for (uint8_t yy = 0; yy < SCALE_FACTOR; ++yy)
				for (uint8_t xx = 0; xx < SCALE_FACTOR; ++xx)
					frameBuffer->SetPixel(x * SCALE_FACTOR + xx, y * SCALE_FACTOR + yy, COLORS[color]);

		}
	}

	window->DrawBuffer(*frameBuffer, *bufferAllocator);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	romBuffer = new uint8_t[GB_MAX_CARTRIDGE_SIZE];
	memoryBuffer = new uint8_t[GB_MAIN_MEM_SIZE];

	videoBufferSize = GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT / 4;
	videoBuffer = new uint8_t[videoBufferSize];

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

	memory = new Memory(memoryBuffer);
	cpu = new CPU(*memory);
	cartridge = new Cartridge(romBuffer);
	video = new Video(*cpu, *memory, videoBuffer);
	input = new Input(*cpu, *memory);

	memory->MemoryWriteCallback = MemoryWriteCallback;
	memory->MemoryReadCallback = MemoryReadCallback;
	video->VBlankCallback = VBlankCallback;

	emulator = new Emulator(*cpu, *memory, *cartridge, *video, *input);
	emulator->Boot();

	InputManager& inputManager = InputManager::Instance();

	window = new Window(hInstance, "WinBoyWindow");
	window->Create("WinBoy", GB_SCREEN_WIDTH * SCALE_FACTOR, GB_SCREEN_HEIGHT * SCALE_FACTOR);

	bufferAllocator = new GDIBufferAllocator(window->handle);

	frameBuffer = new Buffer(bufferAllocator);
	frameBuffer->Allocate(GB_SCREEN_WIDTH * SCALE_FACTOR, GB_SCREEN_HEIGHT * SCALE_FACTOR, Buffer::BGR24);

	window->Show(nCmdShow);

	Video::Mode prevVideoMode = video->CurrentMode();
	
	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency(&timerFrequency);
	const double secondsPerTick = 1.0 / timerFrequency.QuadPart;

	LARGE_INTEGER previousFrameTicks;
	QueryPerformanceCounter(&previousFrameTicks);

	double realTime = 0.0;

	while (true)
	{
		if (inputManager.GetKeyDown('B'))
		{
			paused = true;
			Debug::Print("[WinBoy]: Execution paused.\n");
			emulator->PrintDisassembly(DISASSEMBLY_LENGTH);
			emulator->PrintRegisters();
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
				emulator->Step();
				emulator->PrintDisassembly(DISASSEMBLY_LENGTH);
				emulator->PrintRegisters();
			}

			if (inputManager.GetKeyDown('P'))
			{
				emulator->PrintDisassembly(DISASSEMBLY_LENGTH);
				emulator->PrintRegisters();
			}

			if (inputManager.GetKeyDown('V'))
			{
				for (uint16_t address = GB_TILE_DATA_0; address < GB_TILE_DATA_1; ++address)
				{
					uint8_t byte = memory->ReadByte(address);
					Debug::Print("0x%02X ", byte);

					if (address % 16 == 15)
						Debug::Print("\n");
				}
			}

			if (inputManager.GetKeyDown('T'))
			{
				Debug::Print("[WinBoy]: Drawing tileset to framebuffer... ");

				video->DrawTileset();
				DrawFrameBuffer();

				Debug::Print("done\n");
			}

			if (inputManager.GetKeyDown('E'))
			{
				breakpointsEnabled = true;
				Debug::Print("[WinBoy]: Breakpoints enabled\n");
			}

			if (inputManager.GetKeyDown('D'))
			{
				breakpointsEnabled = false;
				Debug::Print("[WinBoy]: Breakpoints disabled\n");
			}

			QueryPerformanceCounter(&previousFrameTicks);
		}
		else
		{
			double cpuTime;
			LARGE_INTEGER currentTicks;
			QueryPerformanceCounter(&currentTicks);
			realTime += (currentTicks.QuadPart - previousFrameTicks.QuadPart) * secondsPerTick;
			previousFrameTicks = currentTicks;
			
			do
			{
				emulator->Step();
				
				// Calculate the time since boot for the CPU
				cpuTime = cpu->Ticks() / (double) GB_CLOCK_FREQUENCY;

				// Test for PC breakpoints
				if (breakpointsEnabled)
				{
					const CPU::Registers& registers = cpu->GetRegisters();
					for (uint8_t breakpointIdx = 0; breakpointIdx < sizeof(breakpoints) / sizeof(uint16_t); ++breakpointIdx)
					{
						if (registers.pc == breakpoints[breakpointIdx])
						{
							Debug::Print("[WinBoy]: Breakpoint hit at 0x%04X\n", registers.pc);
							paused = true;
							break;
						}
					}
				}

			} while (cpuTime < realTime && !paused);
		}

		window->ProcessMessages();

		input->SetButtonState(Input::BUTTON_A, inputManager.GetKey('Z'));
		input->SetButtonState(Input::BUTTON_B, inputManager.GetKey('X'));
		input->SetButtonState(Input::BUTTON_START, inputManager.GetKey(VK_RETURN));
		input->SetButtonState(Input::BUTTON_SELECT, inputManager.GetKey(VK_BACK));

		input->SetButtonState(Input::BUTTON_DPAD_DOWN, inputManager.GetKey(VK_DOWN));
		input->SetButtonState(Input::BUTTON_DPAD_LEFT, inputManager.GetKey(VK_LEFT));
		input->SetButtonState(Input::BUTTON_DPAD_RIGHT, inputManager.GetKey(VK_RIGHT));
		input->SetButtonState(Input::BUTTON_DPAD_UP, inputManager.GetKey(VK_UP));

		Sleep(1);
	}

	delete[] romBuffer;
	delete[] memoryBuffer;

	return 0;
}

