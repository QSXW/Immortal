/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Device.h"
#include "AudioSource.h"

namespace Immortal
{

AudioDevice::AudioDevice() :
    context{ AudioRenderContext::CreateInstance() },
    stopping{ false }
{
    thread = new Thread{ [=, this] {
        auto hnsActualDuration = (double)REFTIMES_PER_SEC * context->GetBufferSize() / context->GetSampleRate();

        context->Begin();
        while (!stopping)
        {
            AudioClip audioClip{};
            {
                std::unique_lock<std::mutex> lock{ mutex };
                if (queue.empty())
                {
                    continue;
                }
                audioClip = queue.front();
                queue.pop();
            }

            context->PlaySamples(audioClip.frames, audioClip.pData);

            auto hnsActualDuration = (double)REFTIMES_PER_SEC * audioClip.frames / context->GetSampleRate();
            Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
        }
        context->End();
        Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
    } };

    thread->Start();
}

AudioDevice::~AudioDevice()
{
    stopping = true;
    thread.Reset();
}

void AudioDevice::PlayAudioStream(AudioSource *pAudioSource)
{
    auto hnsActualDuration = (double)REFTIMES_PER_SEC * context->GetBufferSize() / context->GetSampleRate();

    context->Begin();
    {
        auto audioClip = pAudioSource->GetAudioClip();
        context->PlaySamples(audioClip.frames, audioClip.pData);

        auto hnsActualDuration = (double)REFTIMES_PER_SEC * audioClip.frames / context->GetSampleRate();
        Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
    }
    context->End();
    Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
}

void AudioDevice::PlayClip(AudioClip pAudioClip)
{
    std::unique_lock<std::mutex> lock{ mutex };
    queue.push(pAudioClip);
}

void AudioDevice::PlayFrame(Picture picture)
{
    AudioClip clip{ picture };
    std::unique_lock<std::mutex> lock{ mutex };
    queue.push(clip);
}

}
