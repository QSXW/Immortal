/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"
#include "Graphics/Format.h"

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

class IAudioDevice
{
public:
	virtual ~IAudioDevice() = default;

    virtual void OpenDevice() = 0;

    virtual void Begin() = 0;

    virtual void End() = 0;

    virtual void Reset() = 0;

    virtual void Pause(bool enable) = 0;

    virtual double GetPostion() = 0;
    
    virtual void BeginRender(uint32_t frames) = 0;
    
    virtual void WriteBuffer(const uint8_t *buffer, size_t size) = 0;

    virtual void EndRender(uint32_t frames) = 0;

    virtual uint32_t GetAvailableFrameCount() = 0;
    
    virtual AudioFormat GetFormat() = 0;

public:
	static IAudioDevice *CreateInstance();
};

}
