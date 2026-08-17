/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"
#include "Graphics/Format.h"
#include "Graphics/Event/Event.h"
#include <functional>

#define REFTIMES_PER_SEC       10000000ll
#define REFTIMES_PER_MILLISEC  10000ll

namespace Immortal
{

enum class AudioDeviceType
{
    Render,
    Capture
};

struct AudioFormat
{
	Format   format;
    int      channels;
	uint8_t  silence;
    int      sampleRate;
	uint64_t mask;
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

class AudioDefaultDeviceChangedEvent : public Event
{
public:
	AudioDefaultDeviceChangedEvent() = default;

	DEFINE_EVENT_TYPE(AudioDefaultDeviceChanged);
	DEFINE_EVENT_CATEGORY(Category::Audio);
};

class AudioDeviceRemovedEvent : public Event
{
public:
	AudioDeviceRemovedEvent() = default;
	DEFINE_EVENT_TYPE(AudioDeviceRemoved);
	DEFINE_EVENT_CATEGORY(Category::Audio);
};

class AudioDeviceAddedEvent : public Event
{
public:
	AudioDeviceAddedEvent() = default;

	DEFINE_EVENT_TYPE(AudioDeviceAdded);
	DEFINE_EVENT_CATEGORY(Category::Audio);
};

class AudioDeviceStateChangedEvent : public Event
{
public:
	AudioDeviceStateChangedEvent() = default;

	DEFINE_EVENT_TYPE(AudioDeviceStateChanged);
	DEFINE_EVENT_CATEGORY(Category::Audio);
};

class AudioDevicePropertyValueChangedEvent : public Event
{
public:
	AudioDevicePropertyValueChangedEvent() = default;

	DEFINE_EVENT_TYPE(AudioDevicePropertyValueChanged);
	DEFINE_EVENT_CATEGORY(Category::Audio);
};

struct AudioDeviceInfo
{
	std::string name;
	AudioDeviceType type;
	std::string uuid;
};

class AudioStream;
class IAudioDevice
{
public:
	virtual ~IAudioDevice() = default;

    virtual bool OpenDevice(const AudioDeviceInfo &deviceInfo = {}) = 0;

    virtual AudioStream *CreateStream() = 0;

	virtual AudioFormat GetFormat() = 0;

    virtual bool SetOnEvent(const std::function<void(Event &)> &callback) = 0;

	virtual int EnumeratorDevices(AudioDeviceType type, AudioDeviceInfo *devices, uint32_t *numDevice) = 0;

public:
	static IAudioDevice *CreateInstance();
};

}
