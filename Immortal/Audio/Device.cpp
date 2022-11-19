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
        Vector2 buffer[2048] = {};
        Vector2 *ptr = buffer;
        while (!stopping)
        {
            Picture picture;
            if (callBack)
            {
                callBack(picture);
            }

            if (picture.Available())
            {
                int frameLeft = 0;
                if (picture.desc.width < 1024)
                {
                    size_t bytes = picture.desc.width << 3;
                    memcpy(ptr, picture.shared->data[0], bytes);
                    ptr += picture.desc.width;
                    size_t frames = ptr - buffer;
                    if (frames > 1024)
                    {
                        frameLeft = context->PlaySamples(frames, (const uint8_t *)buffer);
                        ptr = buffer;
                    }
                }
                else
                {
                    frameLeft = context->PlaySamples(picture.desc.samples, picture.shared->data[0]);
                }
                uint64_t duration = Seconds2Nanoseconds(((float)frameLeft / context->GetSampleRate()));
                duration >>= 1;
                std::this_thread::sleep_for(std::chrono::nanoseconds(duration));
            }

            if (pause)
            {
                context->Pause(true);
                semaphore.Wait();
                semaphore.Reset();
                context->Pause(false);
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
