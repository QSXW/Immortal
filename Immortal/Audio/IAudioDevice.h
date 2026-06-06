/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"
#include "Graphics/Format.h"
#include <functional>

#define REFTIMES_PER_SEC       10000000ll
#define REFTIMES_PER_MILLISEC  10000ll

namespace Immortal
{

enum class AudioDeviceType
{
    Ouput,
    Capture
};

struct AudioFormat
{
	Format   format;
    uint8_t  channels;
	uint8_t  silence;
    uint32_t sampleRate;
};

struct AudioBuffer
{
    uint8_t *data;
    uint32_t size;
};

enum AudioDeviceEvent
{
	AudioDeviceEvent_OnDefaultDeviceChanged,
	AudioDeviceEvent_OnDeviceRemoved,
	AudioDeviceEvent_OnDeviceAdded,
	AudioDeviceEvent_OnDeviceStateChanged,
	AudioDeviceEvent_OnPropertyValueChanged,
	NumAudioDeviceEvent,
};

class AudioStream;
class IAudioDevice
{
public:
	virtual ~IAudioDevice() = default;

    virtual bool OpenDevice() = 0;

    virtual AudioStream *CreateStream() = 0;

    virtual bool RegisterCallback(AudioDeviceEvent type, const std::function<void()> &callback) = 0;

public:
	static IAudioDevice *CreateInstance();
};

}
