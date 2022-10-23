/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Audio/AudioSource.h"
#include "Core.h"
#include "Framework/Async.h"
#include "Interface/IObject.h"

#ifdef _MSC_VER
#include <audioclient.h>
#include <mmdeviceapi.h>
#endif

namespace Immortal
{

class AudioClip;
class AudioSource;
class AudioDevice : public IObject
{
public:
    AudioDevice();

    ~AudioDevice();

    void PlayAudioStream(AudioSource *pAudioSource);

    void PlayClip(AudioClip pAudioClip);

public:
    URef<Thread> thread;

    std::mutex mutex;

    std::queue<AudioClip> queue;

#ifdef _MSC_VER
    IMMDeviceEnumerator *pEnumerator;

    IMMDevice *handle;

    IAudioClient *pAudioClient;

    IAudioRenderClient *pRenderClient;

    WAVEFORMATEX *pWaveFormat;
#endif

    uint32_t bufferFrameCount;

    bool stopping;
};

}
