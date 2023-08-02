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

    void OnPauseDown();

    void OnPauseRelease();

    double GetPosition() const;

    double Sync(uint64_t videoTimestamp, double framesPerSecond, double delta);

    uint64_t Sync(double framesPerSecond);

public:
	template <class T>
	void SetCallBack(T &&task)
	{
		callBack = std::move(task);
	}

    void DisableCallBack()
    {
		callBack = {};
    }

public:
    static int GetSampleRate();

protected:
    static AudioDevice *instance;

protected:
    URef<Thread> thread;

    URef<AudioRenderContext> context;

    std::mutex mutex;

    std::atomic_bool status;

    std::function<void(Picture &)> callBack;

    uint64_t pts;

    double startpts;

    int samples;

    bool stopping;

    bool reset;
};

}
