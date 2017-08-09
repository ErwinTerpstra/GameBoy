#ifndef _AUDIO_OUTPUT_H_
#define _AUDIO_OUTPUT_H_

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>

namespace WinBoy
{
	class AudioOutput
	{
	private:
		const uint32_t OUTPUT_FREQUENCY = 44100;

		const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
		const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
		const IID IID_IAudioClient = __uuidof(IAudioClient);
		const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

		IMMDeviceEnumerator* enumerator;
		IMMDevice* device;
		IAudioClient* audioClient;
		IAudioRenderClient* renderClient;
		WAVEFORMATEX* pwfx;

		UINT32 bufferFrameCount;

	public:
		AudioOutput();
		~AudioOutput();

		HRESULT Initialize();
		HRESULT Update();
		HRESULT Finalize();

	private:
		HRESULT LoadData(UINT32 numFrames, BYTE* data, DWORD* flags);

	};
}

#endif