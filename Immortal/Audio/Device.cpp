/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Device.h"
#include "AudioSource.h"

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
    context{ AudioRenderContext::CreateInstance() },
    pts{ 0 },
    samples{ 0 },
    stopping{ false },
    startpts{ 0 },
    reset{ true },
    status{false}
{
    instance = this;
    thread = new Thread{ [=, this] {
        uint64_t duration = 0;
        context->Begin();
		StereoVector2 buffer[2048] = {};
		StereoVector2 *ptr = buffer;

        static int lastSamples = 1024;
        while (!stopping)
        {
            Picture picture{};
            if (callBack)
            {
                callBack(picture);
            }

            if (picture.Available())
            {
                if (reset)
                {
                    startpts = picture.pts;
                    samples  = picture.desc.samples;
                    reset = false;
                }
                int frameLeft = 0;
                lastSamples = picture.desc.samples;
                if (picture.desc.samples < 1024)
                {
                    size_t bytes = picture.GetWidth() << 3;
                    memcpy(ptr, picture.shared->data[0], bytes);
					ptr += picture.GetWidth();
                    size_t frames = ptr - buffer;
                    if (frames > 1024)
                    {
                        frameLeft = context->PlaySamples(frames, (const uint8_t *)buffer);
                        ptr = buffer;
                    }
                }
                else
                {
                    pts = picture.pts;
                    samples = picture.desc.samples;
                    frameLeft = context->PlaySamples(picture.desc.samples, picture.shared->data[0]);
                }
                duration = Seconds2Nanoseconds(((float)frameLeft / context->GetSampleRate()));
            }
            else
            {
                if (lastSamples >= 512)
                {
                    duration = Seconds2Nanoseconds(((float)1024 / context->GetSampleRate()));
                }
            }

            duration >>= 1;
            std::this_thread::sleep_for(std::chrono::nanoseconds(duration));

            status.wait(true);
        }

        context->End();
    } };

    thread->Start();
    thread->SetDebugDescription("AudioThread");
}

AudioDevice::~AudioDevice()
{
	status = false;
	status.notify_one();

	stopping = true;
	thread->Join();
    thread.Reset();
}

void AudioDevice::PlayAudioStream(AudioSource *pAudioSource)
{
    uint64_t duration = Seconds2Nanoseconds((float)context->GetBufferSize() / context->GetSampleRate());

    context->Begin();
    {
        auto audioClip = pAudioSource->GetAudioClip();
        context->PlaySamples(audioClip.frames, audioClip.pData);

        uint64_t duration = Seconds2Nanoseconds(((float)audioClip.frames / context->GetSampleRate()));
        std::this_thread::sleep_for(std::chrono::nanoseconds(duration >> 1));
    }
    context->End();
    std::this_thread::sleep_for(std::chrono::nanoseconds(duration >> 1));
}

void AudioDevice::PlayClip(AudioClip pAudioClip)
{

}

void AudioDevice::PlayFrame(Picture picture)
{

}

void AudioDevice::OnPauseDown()
{
	status = true;
    context->Pause(true);
}

void AudioDevice::OnPauseRelease()
{
    context->Pause(false);
	status = false;
	status.notify_one();
}

void AudioDevice::Reset()
{
    reset = true;
    context->Reset();
}

double AudioDevice::GetPosition() const
{
	return context->GetPostion();
}

double AudioDevice::Sync(uint64_t videoTimestamp, double framesPerSecond, double delta)
{
    double start = (double)startpts * ((double)samples / context->GetSampleRate());
    double time = (double)videoTimestamp * (1.0 / framesPerSecond) + delta;
    return time - (start + GetPosition());
}

uint64_t AudioDevice::Sync(double framesPerSecond)
{
    double audioTimestamp = (double)startpts * 512.0 / 48000.0; // ((double)samples / context->GetSampleRate());
    audioTimestamp += GetPosition();

    double videoTimestamp = audioTimestamp / (1.0 / framesPerSecond);
    return videoTimestamp;
}

AudioDevice *AudioDevice::instance;

int AudioDevice::GetSampleRate()
{
    return instance ? instance->context->GetSampleRate() : 48000;
}

}
