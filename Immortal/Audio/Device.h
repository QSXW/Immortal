/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"
#include "Shared/Async.h"
#include "Shared/IObject.h"
#include "IAudioDevice.h"
#include "AudioStream.h"

namespace Immortal
{

class IMMORTAL_API AudioDevice : public IObject
{
public:
    AudioDevice();

    ~AudioDevice();

    void Reset();

    void OnPauseDown();

    void OnPauseRelease();
   
    AudioFormat GetFormat() const;

    IAudioStream *CreateAudioStream(const PFN_AudioStreamPlayCallback &callback);

    void DestroyAudioStream(IAudioStream **ppStream);

    bool SetOnEvent(const std::function<void(Event &)> &callback);

    void OnEvent();

    int EnumeratorDevices(AudioDeviceType type, AudioDeviceInfo *devices, uint32_t *numDevice);

public:
	static AudioDevice *GetInstance();

protected:
    static AudioDevice *instance;

protected:
    URef<Thread> thread;

    URef<IAudioDevice> handle;

    std::mutex mutex;

    std::atomic_bool status;

    std::atomic_bool defaultDeviceChanged;

    std::vector<URef<IAudioStream>> streams;

    std::function<void(Event &)> onEvent;

    uint64_t pts;

    double startpts;

    int samples;

    bool stopping;

    bool reset;
};

}
