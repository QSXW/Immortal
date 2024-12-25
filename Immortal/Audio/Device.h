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

    double GetPosition() const;
    
    int GetSampleRate() const;

    int PlaySamples(uint32_t numberSamples, const uint8_t *pSamples);

    IAudioStream *CreateAudioStream(const PFN_AudioStreamPlayCallback &callback);

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

    std::vector<URef<IAudioStream *>> streams;

    uint64_t pts;

    double startpts;

    int samples;

    bool stopping;

    bool reset;
};

}
