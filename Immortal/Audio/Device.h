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

public:
    URef<Thread> thread;

    URef<AudioRenderContext> context;

    std::mutex mutex;

    std::queue<AudioClip> queue;

    bool stopping;
};

}
