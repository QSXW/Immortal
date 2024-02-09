/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Device.h"

namespace Immortal
{

static inline float Seconds2Nanoseconds(float seconds)
{
    return seconds * 1000000000;
}

struct StereoVector2
{
	float x;
	float y;
};

AudioDevice::AudioDevice() :
    handle{ IAudioDevice::CreateInstance() },
    pts{ 0 },
    samples{ 0 },
    stopping{ false },
    startpts{ 0 },
    reset{ true },
    status{false}
{
    instance = this;
    if (!handle)
    {
        return;
    }

    thread = new Thread{ [=, this] {
        uint64_t duration = 0;
        handle->Begin();
		StereoVector2 buffer[2048] = {};
		StereoVector2 *ptr = buffer;

        AudioFormat format = handle->GetFormat();

        static int lastSamples = 1024;
        while (!stopping)
        {
            Picture picture{};
            if (callBack)
            {
                callBack(picture);
            }

            if (picture)
            {
                if (reset)
                {
                    startpts = picture.GetTimestamp();
                    samples  = picture.GetWidth();
                    reset = false;
                }
                int frameLeft = 0;
				lastSamples = picture.GetWidth();
				if (picture.GetWidth() < 1024)
                {
                    size_t bytes = picture.GetWidth() << 3;
					memcpy(ptr, picture.GetData(), bytes);
					ptr += picture.GetWidth();
                    uint32_t frames = uint32_t(ptr - buffer);
                    if (frames > 1024)
                    {
                        frameLeft = PlaySamples(frames, (const uint8_t *)buffer);
                        ptr = buffer;
                    }
                }
                else
                {
					pts       = picture.GetTimestamp();
                    samples   = picture.GetWidth();
					frameLeft = PlaySamples(samples, picture.GetData());
                }
				duration = Seconds2Nanoseconds(((float) frameLeft / format.sampleRate));
            }
            else
            {
                if (lastSamples >= 512)
                {
					duration = Seconds2Nanoseconds(((float)1024 / format.sampleRate));
                }
            }

            duration >>= 1;
            std::this_thread::sleep_for(std::chrono::nanoseconds(duration));

            status.wait(true);
        }

        handle->End();
    } };

    thread->Start();
    thread->SetDebugDescription("AudioThread");
}

AudioDevice::~AudioDevice()
{
	status = false;
	status.notify_one();

	stopping = true;
    if (thread)
    {
        thread->Join();
        thread.Reset();
    }
}

void AudioDevice::OnPauseDown()
{
	status = true;
    handle->Pause(true);
}

void AudioDevice::OnPauseRelease()
{
    handle->Pause(false);
	status = false;
	status.notify_one();
}

void AudioDevice::Reset()
{
    reset = true;
    handle->Reset();
}

double AudioDevice::GetPosition() const
{
	return handle->GetPostion();
}

int AudioDevice::GetSampleRate() const
{
	AudioFormat format = handle->GetFormat();
	return format.sampleRate;
}

int AudioDevice::PlaySamples(uint32_t numberSamples, const uint8_t *pSamples)
{
    uint32_t frameRequested = 0;
    while (numberSamples > 0)
    {
        uint32_t numFramesPadding = handle->GetAvailableFrameCount();

        frameRequested = std::min(numberSamples, numFramesPadding);

        handle->BeginRender(frameRequested);

        uint32_t bytes = frameRequested << 3;
		handle->WriteBuffer(pSamples, bytes);
        pSamples += bytes;
        numberSamples -= frameRequested;

        handle->EndRender(frameRequested);
    }

    return frameRequested;
}

AudioDevice *AudioDevice::instance;

}
