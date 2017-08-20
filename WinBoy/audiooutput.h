#ifndef _AUDIO_OUTPUT_H_
#define _AUDIO_OUTPUT_H_

#include "libdmg.h"

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>

//#define USE_EXCLUSIVE_MODE

using namespace libdmg;

namespace WinBoy
{
	class AudioOutput
	{
	private:
		const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
		const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
		const IID IID_IAudioClient = __uuidof(IAudioClient);
		const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
		const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);
		const IID IID_IAudioStreamVolume = __uuidof(IAudioStreamVolume);

		IMMDeviceEnumerator* enumerator;
		IMMDevice* device;
		IAudioClient* audioClient;
		IAudioRenderClient* renderClient;
		IAudioStreamVolume* volume;
		ISimpleAudioVolume* simpleVolume;
		WAVEFORMATEX* waveFormat;
		WAVEFORMATEXTENSIBLE* waveFormatExtensible;

		UINT32 bufferFrameCount;

		Audio& audio;

	public:
		AudioOutput(Audio& audio);
		~AudioOutput();

		HRESULT Initialize();
		HRESULT Update();
		HRESULT Finalize();

	private:
		HRESULT LoadData(UINT32 numFrames, BYTE* data, DWORD* flags);
		HRESULT LoadSine(UINT32 numFrames, BYTE* data, DWORD* flags);

		void WriteSample(float sample, WORD bitsPerSample, BYTE* buffer);

		template<uint32_t bitsPerSample>
		void WriteSample(float sample, BYTE* buffer);

		template<>
		void WriteSample<8U>(float sample, BYTE* buffer)
		{
			WRITE_BYTE(buffer, (uint8_t)(sample * ((1 << 8) - 1)));
		}

		template<>
		void WriteSample<16U>(float sample, BYTE* buffer)
		{
			WRITE_SHORT(buffer, (uint16_t)(sample * ((1 << 16) - 1)));
		}

		template<>
		void WriteSample<32U>(float sample, BYTE* buffer)
		{
			WRITE_LONG(buffer, (uint32_t)(sample * (((uint64_t) 1 << 32) - 1)));
		}

	};
}

#endif