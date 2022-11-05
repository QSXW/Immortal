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

AudioDevice::AudioDevice() :
    context{ AudioRenderContext::CreateInstance() },
    stopping{ false },
    flush{ false }
{
    thread = new Thread{ [=, this] {

        uint64_t duration = 0;
        context->Begin();
        while (!stopping)
        {
            Picture picture;
            if (callBack)
            {
                callBack(picture);
            }

            if (picture.Available())
            {
                AudioClip audioClip{ picture };
                context->PlaySamples(audioClip.frames, audioClip.pData);
                uint64_t duration = Seconds2Nanoseconds(((float)audioClip.frames / context->GetSampleRate()));
                duration >>= 1;
                if (picture.desc.width <= 512)
                {
                    duration = 0;
                }
                std::this_thread::sleep_for(std::chrono::nanoseconds(duration));
            }

            if (pause)
            {
                context->End();
                semaphore.Wait();
                semaphore.Reset();
                context->Begin();
            }

            if (flush)
            {
                Reset();
                flush = false;
            }
        }
        context->End();
    } };

    thread->Start();
}

AudioDevice::~AudioDevice()
{
    Flush();
    stopping = true;
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

void AudioDevice::Reset()
{
    context->End();
    context->Reset();
    context->Begin();
}

}
