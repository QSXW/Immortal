/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "IAudioDevice.h"
#include "Shared/IObject.h"
#include "Shared/Async.h"

#include <thread>
#include <mutex>

namespace Immortal
{

using PFN_AudioStreamPlayCallback = std::function<uint32_t(void *, uint32_t)>;

class AudioStream : public IObject
{
public:
	AudioStream();

	virtual ~AudioStream();

	virtual bool Start() = 0;

	virtual bool Stop() = 0;

	virtual bool Reset() = 0;

	virtual bool BeginRender(uint32_t frames) = 0;

	virtual void WriteBuffer(const uint8_t *buffer, size_t size) = 0;

	virtual bool EndRender(uint32_t frames) = 0;

	virtual uint32_t GetAvailableFrameCount() = 0;

	virtual AudioFormat GetFormat() = 0;

	virtual bool OnDeviceChanged(IAudioDevice *device) = 0;

	/// Default: worker thread + PlaySamples (WASAPI/ALSA/CoreAudio). SDL overrides to use SDL_AudioCallback only.
	virtual void Start(const PFN_AudioStreamPlayCallback &callback);

	int PlaySamples(uint32_t numberSamples, const uint8_t *pSamples);

	void SetDebugName(const std::string &name);

	/// One SDL-sized hardware period in bytes (ffplay `audio_hw_buf_size`); 0 if unknown.
	virtual uint32_t FfplayAudioHwBufferBytes() const { return 0; }

protected:
	void Destroy();

protected:
	Thread thread;

	PFN_AudioStreamPlayCallback callback;

	std::mutex mutex;

	std::atomic_bool exited;

	std::string name;

	uint32_t bytePerSample;
};

using IAudioStream = AudioStream;

}
