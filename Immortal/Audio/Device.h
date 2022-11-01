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
#include "AudioRenderContext.h"
#include "Sync/Semaphore.h"

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

    void PlayFrame(Picture picture);

    void Reset();

    template <class T>
    void SetCallBack(T &&task)
    {
        callBack = std::move(task);
    }

    void Flush()
    {
        flush = true;
    }

    void OnPauseDown()
    {
        pause = true;
    }

    void OnPauseRelease()
    {
        pause = false;
        semaphore.Signal();
    }

public:
    URef<Thread> thread;

    URef<AudioRenderContext> context;

    std::mutex mutex;

    Semaphore semaphore = { "Audio Device Sync Primitive" };

    std::function<void(Picture &)> callBack;

    bool stopping;

    bool flush;

    bool pause = false;
};

}
