// WinBoy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "libdmg.h"

#include "Window/window.h"
#include "Window/buffer.h"
#include "Window/gdibufferallocator.h"
#include "Window/color.h"

#include "inputmanager.h"
#include "audiooutput.h"

//#define ROM_FILE "../roms/Tetris (World).gb"
//#define ROM_FILE "../roms/SuperMarioLand.gb"

//#define ROM_FILE "../roms/tests/cpu_instrs/cpu_instrs.gb"
//#define ROM_FILE "../roms/tests/cpu_instrs/individual/01-special.gb"
//#define ROM_FILE "../roms/tests/cpu_instrs/individual/02-interrupts.gb"
#define ROM_FILE "../roms/tests/cpu_instrs/individual/03-op sp,hl.gb"
//#define ROM_FILE "../roms/tests/instr_timing/instr_timing.gb"
//#define ROM_FILE "../roms/tests/interrupt_time/interrupt_time.gb"

#define DISASSEMBLY_LENGTH 10
#define SCALE_FACTOR 2
#define MAX_CATCHUP_TIME 1.0

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
};

uint8_t opcodeBreakpoints[] =
{
	0xF4,

	0xF8,
};

uint16_t memoryBreakpoints[] =
{
	0x00,

	//0xFF80,	// Current joypad state
	//0xFF81,	// Changed joypad bits

	//GB_OAM,
	//GB_REG_DMA
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
Audio* audio;
Input* input;

Emulator* emulator;

Window* window;
AudioOutput* audioOutput;

GDIBufferAllocator* bufferAllocator;
Buffer* frameBuffer;

int main()
{
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}

void DrawFrameBuffer();
void PauseEmulator();

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
			PauseEmulator();
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
			PauseEmulator();
			break;
		}
	}
}

void VBlankCallback()
{
	//DrawFrameBuffer();
	
	HRESULT result = audioOutput->Update();

	if (result != S_OK)
		Debug::Print("[WinBoy]: Error updating audio output: 0x%04x\n", result);
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

void PauseEmulator()
{
	emulator->PrintDisassembly(DISASSEMBLY_LENGTH);
	emulator->PrintRegisters();

	paused = true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	romBuffer = new uint8_t[GB_MAX_CARTRIDGE_SIZE];

	videoBufferSize = GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT / 4;
	videoBuffer = new uint8_t[videoBufferSize];

	memset(romBuffer, 0, GB_MAX_CARTRIDGE_SIZE);
	memset(videoBuffer, 0, videoBufferSize);

	FILE* handle;
	errno_t error = fopen_s(&handle, ROM_FILE, "rb");

	if (error != 0)
	{
		Debug::Print("[WinBoy]: Failed to read ROM file: %s!\n", ROM_FILE);
		return 1;
	}

	size_t romSize = fread(romBuffer, 1, GB_MAX_CARTRIDGE_SIZE, handle);

	fclose(handle);

	Debug::Print("[WinBoy]: Rom file read with %u bytes.\n", romSize);

	memory = new Memory();
	cpu = new CPU(*memory);
	cartridge = new Cartridge(romBuffer);
	video = new Video(*cpu, *memory, videoBuffer);
	audio = new Audio(*memory);
	input = new Input(*cpu);

	memory->MemoryWriteCallback = MemoryWriteCallback;
	memory->MemoryReadCallback = MemoryReadCallback;
	video->VBlankCallback = VBlankCallback;

	emulator = new Emulator(*cpu, *memory, *cartridge, *video, *audio, *input);
	emulator->Boot();

	InputManager& inputManager = InputManager::Instance();

	window = new Window(hInstance, "WinBoyWindow");
	window->Create("WinBoy", GB_SCREEN_WIDTH * SCALE_FACTOR, GB_SCREEN_HEIGHT * SCALE_FACTOR);

	bufferAllocator = new GDIBufferAllocator(window->handle);

	frameBuffer = new Buffer(bufferAllocator);
	frameBuffer->Allocate(GB_SCREEN_WIDTH * SCALE_FACTOR, GB_SCREEN_HEIGHT * SCALE_FACTOR, Buffer::BGR24);

	window->Show(nCmdShow);

	audioOutput = new AudioOutput(*audio);
	HRESULT result = audioOutput->Initialize();
	if (result != S_OK)
	{
		Debug::Print("[WinBoy]: Error initializing audio output: 0x%04x\n", result);
		return 1;
	}

	Video::Mode prevVideoMode = video->CurrentMode();
	
	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency(&timerFrequency);
	const double secondsPerTick = 1.0 / timerFrequency.QuadPart;

	LARGE_INTEGER previousFrameTicks;
	QueryPerformanceCounter(&previousFrameTicks);

	double realTime = 0.0;
	double timeScale = 1.0;

	while (true)
	{
		if (inputManager.GetKeyDown('B'))
		{
			paused = true;
			Debug::Print("[WinBoy]: Execution paused.\n");
			emulator->PrintDisassembly(DISASSEMBLY_LENGTH);
			emulator->PrintRegisters();
		}

		if (inputManager.GetKeyDown('Y'))
		{
			Debug::Print("[WinBoy]: Emulator reset.\n");
			emulator->Boot();
			realTime = 0.0;
		}

		if (inputManager.GetKey('L'))
		{
			if (inputManager.GetKeyDown('1'))
			{
				video->SetLayerState(Video::LAYER_BACKGROUND, !video->GetLayerState(Video::LAYER_BACKGROUND));
				Debug::Print("[WinBoy]: Background layer %s\n", video->GetLayerState(Video::LAYER_BACKGROUND) ? "enabled" : "disabled");
			}

			if (inputManager.GetKeyDown('2'))
			{
				video->SetLayerState(Video::LAYER_WINDOW, !video->GetLayerState(Video::LAYER_WINDOW));
				Debug::Print("[WinBoy]: Window layer %s\n", video->GetLayerState(Video::LAYER_WINDOW) ? "enabled" : "disabled");
			}

			if (inputManager.GetKeyDown('3'))
			{
				video->SetLayerState(Video::LAYER_SPRITES, !video->GetLayerState(Video::LAYER_SPRITES));
				Debug::Print("[WinBoy]: Sprite layer %s\n", video->GetLayerState(Video::LAYER_SPRITES) ? "enabled" : "disabled");
			}
		}

		timeScale = inputManager.GetKey('P') ? 4.0 : 1.0;

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

			if (inputManager.GetKeyDown('T'))
			{
				Debug::Print("[WinBoy]: Executing one tick...\n");
				emulator->Tick();
			}

			if (inputManager.GetKeyDown('P'))
			{
				emulator->PrintDisassembly(DISASSEMBLY_LENGTH);
				emulator->PrintRegisters();
			}

			if (inputManager.GetKey('V'))
			{
				uint16_t startAddress = 0, endAddress = 0;

				if (inputManager.GetKeyDown('1'))
				{
					startAddress = GB_BG_MAP_0;
					endAddress = GB_BG_MAP_0 + 0x3FF;
				}
				else if (inputManager.GetKeyDown('2'))
				{
					startAddress = GB_BG_MAP_1;
					endAddress = GB_BG_MAP_1 + 0x3FF;
				}
				
				if (startAddress != 0)
				{
					for (uint16_t address = startAddress; address <= endAddress; ++address)
					{
						if (address % 16 == 0)
							Debug::Print("0x%04X  ", address);

						uint8_t byte = memory->ReadByte(address);
						Debug::Print("0x%02X ", byte);

						if (address % 16 == 15)
							Debug::Print("\n");
					}
				}
			}

			if (inputManager.GetKeyDown('F'))
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

			if (inputManager.GetKeyDown('C'))
			{
				emulator->PrintInstructionCount();
			}

			QueryPerformanceCounter(&previousFrameTicks);
		}
		else
		{
			double emulatorTime;
			LARGE_INTEGER currentTicks;
			QueryPerformanceCounter(&currentTicks);
			realTime += (currentTicks.QuadPart - previousFrameTicks.QuadPart) * secondsPerTick * timeScale;
			previousFrameTicks = currentTicks;
			
			do
			{
				uint64_t cpuTicks = cpu->Ticks();
				emulator->Tick();
				
				// Calculate the time since boot for the CPU
				emulatorTime = emulator->Ticks() / (double) GB_CLOCK_FREQUENCY;

				double delta = realTime - emulatorTime;
				if (delta > MAX_CATCHUP_TIME)
				{
					realTime = emulatorTime;
					printf("[WinBoy]: Warning! Emulator was %.2fs behind. Skipping to catch up...\n", delta);
				}

				if (cpu->Ticks() != cpuTicks && breakpointsEnabled)
				{
					// Test for PC breakpoints
					const CPU::Registers& registers = cpu->GetRegisters();
					for (uint8_t breakpointIdx = 0; breakpointIdx < sizeof(breakpoints) / sizeof(uint16_t); ++breakpointIdx)
					{
						if (registers.pc == breakpoints[breakpointIdx])
						{
							Debug::Print("[WinBoy]: Breakpoint hit at 0x%04X\n", registers.pc);
							PauseEmulator();
							break;
						}
					}

					uint8_t nextOpcode = memory->ReadByte(registers.pc);
					for (uint8_t breakpointIdx = 0; breakpointIdx < sizeof(opcodeBreakpoints) / sizeof(uint8_t); ++breakpointIdx)
					{
						if (nextOpcode == opcodeBreakpoints[breakpointIdx])
						{
							Debug::Print("[WinBoy]: Opcode breakpoint hit for 0x%02X at 0x%04X\n", nextOpcode, registers.pc);
							PauseEmulator();
							break;
						}
					}
				}
			} while (emulatorTime < realTime && !paused);
		}

		DrawFrameBuffer();
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

	audioOutput->Finalize();

	delete[] romBuffer;
	delete[] memoryBuffer;

	return 0;
}

