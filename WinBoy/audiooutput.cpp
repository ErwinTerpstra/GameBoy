#include "stdafx.h"
#include "audiooutput.h"

#include "libdmg.h"

using namespace WinBoy;

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) \
		return hres;

#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL) \
	{ \
		(punk)->Release(); \
		(punk) = NULL; \
	} \

AudioOutput::AudioOutput() : enumerator(NULL), device(NULL), audioClient(NULL), renderClient(NULL), pwfx(NULL)
{

}

AudioOutput::~AudioOutput()
{
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(enumerator);
	SAFE_RELEASE(device);
	SAFE_RELEASE(audioClient);
	SAFE_RELEASE(renderClient);
}

HRESULT AudioOutput::Initialize()
{
	HRESULT hr;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enumerator);
	EXIT_ON_ERROR(hr);

	hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
	EXIT_ON_ERROR(hr);

	hr = device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&audioClient);
	EXIT_ON_ERROR(hr);

	WAVEFORMATEX waveFormat = { };
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 2;
	waveFormat.nSamplesPerSec = OUTPUT_FREQUENCY;
	waveFormat.nAvgBytesPerSec = OUTPUT_FREQUENCY * 4;
	waveFormat.nBlockAlign = 4;
	waveFormat.wBitsPerSample = 16;

	hr = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &waveFormat, NULL);
	EXIT_ON_ERROR(hr);

	hr = audioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, 4096 * 100, 0, &waveFormat, NULL);
	EXIT_ON_ERROR(hr);

	// Get the actual size of the allocated buffer.
	hr = audioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr);

	hr = audioClient->GetService(IID_IAudioRenderClient, (void**)&renderClient);
	EXIT_ON_ERROR(hr);
	
	// Start playing.
	hr = audioClient->Start();
	EXIT_ON_ERROR(hr);

	return hr;
}

HRESULT AudioOutput::Update()
{
	BYTE* data;
	DWORD flags = 0;
	UINT32 numFramesPadding;

	HRESULT hr;

	// See how much buffer space is available.
	hr = audioClient->GetCurrentPadding(&numFramesPadding);
	EXIT_ON_ERROR(hr);

	UINT32 numFramesAvailable = bufferFrameCount - numFramesPadding;

	// Grab all the available space in the shared buffer.
	hr = renderClient->GetBuffer(numFramesAvailable, &data);
	EXIT_ON_ERROR(hr);

	// Attempt to fill the buffer
	hr = LoadData(numFramesAvailable, data, &flags);
	EXIT_ON_ERROR(hr);

	hr = renderClient->ReleaseBuffer(numFramesAvailable, flags);
	EXIT_ON_ERROR(hr);

	return hr;
}

HRESULT AudioOutput::Finalize()
{
	// Stop playing.
	HRESULT hr = audioClient->Stop();
	EXIT_ON_ERROR(hr);

	return hr;
}

#define PI 3.14159265359f
#define FREQUENCY 100
#define VOLUME 0.1f

HRESULT AudioOutput::LoadData(UINT32 numFrames, BYTE* data, DWORD* flags)
{
	static uint32_t phase = 0;
	uint16_t* buffer = (uint16_t*)data;

	for (uint32_t frameIdx = 0; frameIdx < numFrames; ++frameIdx, ++phase)
	{
		float x = 0.5f + (sinf((phase / (float) OUTPUT_FREQUENCY) * FREQUENCY * PI * 2) * VOLUME * 0.5f);
		uint16_t sample = (uint16_t) (x * ((1 << 16) - 1));
		
		WRITE_SHORT(buffer + (frameIdx * 2) + 0, sample);
		WRITE_SHORT(buffer + (frameIdx * 2) + 1, sample);
	}

	*flags = numFrames == 0 ? AUDCLNT_BUFFERFLAGS_SILENT : 0;

	return NO_ERROR;
}