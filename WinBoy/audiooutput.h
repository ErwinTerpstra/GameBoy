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
			WRITE_SBYTE(buffer, (int8_t)(sample * 127));
		}

		template<>
		void WriteSample<16U>(float sample, BYTE* buffer)
		{
			WRITE_SSHORT(buffer, (int16_t)(sample * 32768));
		}

		template<>
		void WriteSample<32U>(float sample, BYTE* buffer)
		{
			WRITE_SLONG(buffer, (int32_t)(sample * 2147483648));
		}

	};
}

#endif