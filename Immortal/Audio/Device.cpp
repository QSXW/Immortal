/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Device.h"

#ifdef _MSC_VER
#include <immintrin.h>
#endif

namespace Immortal
{

static inline void MixAudioSamples(float *o, float *a, float *b, size_t size)
{
#if _MSC_VER
    for (size_t i = 0; i < size; i += 8)
    {
		auto _a = _mm256_load_ps(&a[i]);
		auto _b = _mm256_load_ps(&b[i]);
		auto _c = _mm256_add_ps(_a, _b);
		_c = _mm256_min_ps(_c, _mm256_set1_ps( 1.0f));
		_c = _mm256_max_ps(_c, _mm256_set1_ps(-1.0f));
		_mm256_store_ps(&o[i], _c);
    }
#else
    for (size_t i = 0; i < size; i++)
    {
		o[i] = a[i] + b[i];
    }
#endif
}

AudioDevice::AudioDevice() :
    handle{ IAudioDevice::CreateInstance() },
    pts{ 0 },
    samples{ 0 },
    stopping{ false },
    startpts{ 0 },
    reset{ true },
    status{false},
    defaultDeviceChanged{}
{
    instance = this;
    if (!handle)
    {
        return;
    }

    handle->SetOnEvent([=, this](Event &event) {
        if (event.GetType() == Event::Type::AudioDefaultDeviceChanged)
        {
			handle->OpenDevice();
        }

        if (onEvent)
		{
			onEvent(event);
		}
	});
}

AudioDevice::~AudioDevice()
{
	if (instance == this)
	{
		instance = nullptr;
	}

	status = false;
	status.notify_one();

	stopping = true;
    if (thread)
    {
        thread->Join();
        thread.Reset();
    }

	if (handle)
	{
		handle->SetOnEvent({});
	}
	onEvent = {};

	std::vector<URef<IAudioStream>> closingStreams;
	{
		std::lock_guard lock{ mutex };
		closingStreams.swap(streams);
	}
	for (auto &stream : closingStreams)
	{
		if (stream)
		{
			stream->Stop();
		}
	}
	closingStreams.clear();

	handle.Reset();
}

void AudioDevice::OnPauseDown()
{
	status = true;
}

void AudioDevice::OnPauseRelease()
{
	status = false;
	status.notify_one();
}

void AudioDevice::Reset()
{
    reset = true;
}

AudioFormat AudioDevice::GetFormat() const
{
	return handle->GetFormat();
}

bool AudioDevice::SetOnEvent(const std::function<void(Event &)> &callback)
{
	onEvent = callback;
	return true;
}

IAudioStream *AudioDevice::CreateAudioStream(const PFN_AudioStreamPlayCallback &callback)
{
	IAudioStream *stream = handle->CreateStream();
    if (stream)
    {
        {
            std::lock_guard lock{ mutex };
            streams.emplace_back(stream);
        }
		stream->Start(callback);
    }

    return stream;
}

void AudioDevice::DestroyAudioStream(IAudioStream **ppStream)
{
    if (!ppStream || !*ppStream)
    {
        return;
    }

	IAudioStream *stream = *ppStream;
    *ppStream = nullptr;

    URef<IAudioStream> removed;
    {
        std::lock_guard lock{ mutex };
        for (auto it = streams.begin(); it != streams.end(); it++)
        {
            if (stream == it->Get())
            {
                removed.Swap(*it);
                streams.erase(it);
                break;
            }
        }
    }

	if (removed)
	{
		removed->Stop();
		removed.Reset();
	}
}

int AudioDevice::EnumeratorDevices(AudioDeviceType type, AudioDeviceInfo *devices, uint32_t *numDevice)
{
	return handle->EnumeratorDevices(type, devices, numDevice);
}

AudioDevice *AudioDevice::GetInstance()
{
	return instance;
}

AudioDevice *AudioDevice::instance;

}
