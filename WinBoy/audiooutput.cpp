#include "stdafx.h"
#include "audiooutput.h"
#include "debug.h"

#include "libdmg.h"

using namespace WinBoy;
using namespace libdmg;

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) \
		return hres;

#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL) \
	{ \
		(punk)->Release(); \
		(punk) = NULL; \
	} \

AudioOutput::AudioOutput(Audio& audio) : audio(audio),
	enumerator(NULL), device(NULL), audioClient(NULL), renderClient(NULL),
	volume(NULL), waveFormat(NULL), waveFormatExtensible(NULL)
{

}

AudioOutput::~AudioOutput()
{
	if (waveFormat != NULL)
		CoTaskMemFree(waveFormat);

	SAFE_RELEASE(enumerator);
	SAFE_RELEASE(device);
	SAFE_RELEASE(audioClient);
	SAFE_RELEASE(renderClient);
	SAFE_RELEASE(volume);
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

	hr = audioClient->GetMixFormat(&waveFormat);
	EXIT_ON_ERROR(hr);

	waveFormat = (WAVEFORMATEX*)CoTaskMemAlloc(sizeof(WAVEFORMATEX));

	waveFormat->wFormatTag = WAVE_FORMAT_PCM;
	waveFormat->cbSize = sizeof(WAVEFORMATEX);
	waveFormat->nChannels = 2;
	waveFormat->nSamplesPerSec = 32768;
	waveFormat->wBitsPerSample = 16;

	waveFormat->nBlockAlign = waveFormat->wBitsPerSample / 8 * waveFormat->nChannels;
	waveFormat->nAvgBytesPerSec = waveFormat->wBitsPerSample / 8 * waveFormat->nChannels * waveFormat->nSamplesPerSec;

#ifdef USE_EXCLUSIVE_MODE
	hr = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, waveFormat, NULL);
	EXIT_ON_ERROR(hr);

	hr = audioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, 1024 * 1000 * 10, 0, waveFormat, NULL);
	EXIT_ON_ERROR(hr);
#else
	WAVEFORMATEX* fallbackFormat = NULL;

	hr = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, waveFormat, &fallbackFormat);
	EXIT_ON_ERROR(hr);

	if (hr == S_FALSE)
	{
		CoTaskMemFree(waveFormat);
		waveFormat = fallbackFormat;
	}

	if (waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		waveFormatExtensible = (WAVEFORMATEXTENSIBLE*)waveFormat;
		Debug::Print("[AudioOutput]: Using extended wave format\n");
	}

	hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 1024 * 1000 * 10, 0, waveFormat, NULL);
	EXIT_ON_ERROR(hr);
#endif

	// Set the output frequency of the audio subsystem
	audio.SetOutputFrequency(waveFormat->nSamplesPerSec);

	// Get the actual size of the allocated buffer.
	hr = audioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr);

	hr = audioClient->GetService(IID_IAudioRenderClient, (void**)&renderClient);
	EXIT_ON_ERROR(hr);

	hr = audioClient->GetService(IID_IAudioStreamVolume, (void**)&volume);
	EXIT_ON_ERROR(hr);

	for (uint32_t channel = 0; channel < waveFormat->nChannels; ++channel)
		volume->SetChannelVolume(channel, 1.0f);

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

HRESULT AudioOutput::LoadData(UINT32 numFrames, BYTE* data, DWORD* flags)
{
	//return LoadSine(numFrames, data, flags);

	RingBuffer& outputBuffer = audio.GetOutputBuffer();
	uint8_t bytesPerSample = waveFormat->wBitsPerSample / 8;

	if (outputBuffer.Length() < 2 || numFrames == 0)
	{
		*flags = AUDCLNT_BUFFERFLAGS_SILENT;
		return S_OK;
	}

	while (outputBuffer.Length() >= 2 && numFrames > 0)
	{
		for (uint32_t channel = 0; channel < waveFormat->nChannels; ++channel)
		{
			if (channel < 2)
			{
				float sample = ((((float) outputBuffer.ReadByte()) / 255) - 0.5f) * 2.0f;
				WriteSample(sample, waveFormat->wBitsPerSample, data);
			}

			data += bytesPerSample;
		}

		--numFrames;
	}

	*flags = 0;
	return S_OK;
}

#define PI 3.14159265359f
#define FREQUENCY 440
#define VOLUME 0.1f

HRESULT AudioOutput::LoadSine(UINT32 numFrames, BYTE* data, DWORD* flags)
{
	static uint32_t phase = 0;

	uint8_t bytesPerSample = waveFormat->wBitsPerSample / 8;

	for (uint32_t frameIdx = 0; frameIdx < numFrames; ++frameIdx, ++phase)
	{
		float sample = sinf((phase / (float) waveFormat->nSamplesPerSec) * FREQUENCY * PI * 2) * VOLUME;
		
		for (uint32_t channel = 0; channel < waveFormat->nChannels; ++channel)
		{
			BYTE* buffer = data + (frameIdx * waveFormat->nChannels * bytesPerSample) + (channel * bytesPerSample);
			WriteSample(sample, waveFormat->wBitsPerSample, buffer);
		}

	}

	*flags = numFrames == 0 ? AUDCLNT_BUFFERFLAGS_SILENT : 0;

	return S_OK;
}

void AudioOutput::WriteSample(float sample, WORD bitsPerSample, BYTE* buffer)
{
	switch (bitsPerSample)
	{
		case 8:
			WriteSample<8>(sample, buffer);
			break;
		case 16:
			WriteSample<16>(sample, buffer);
			break;
		case 32:
			if (waveFormatExtensible != NULL && waveFormatExtensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
				*((float*)buffer) = sample;
			else
				WriteSample<32>(sample, buffer);

			break;
		default:
			assert(false);
			break;
	}
}