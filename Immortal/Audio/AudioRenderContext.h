/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"

#define REFTIMES_PER_SEC       10000000
#define REFTIMES_PER_MILLISEC  10000

namespace Immortal
{

struct WaveFormat
{
    uint32_t Channels;
    uint32_t SampleRate;
};

class AudioRenderContext
{
public:
    AudioRenderContext() :
        bufferFrameCount{},
        format{}
    {

    }

    virtual ~AudioRenderContext() { }

    virtual void OpenDevice() = 0;

    virtual void Begin() = 0;

    virtual void End() = 0;

    virtual void PlaySamples(uint32_t numberSamples, const uint8_t *pData) = 0;

public:
    uint32_t GetBufferSize() const
    {
        return bufferFrameCount;
    }

    uint32_t GetSampleRate() const
    {
        return format.SampleRate;
    }

public:
    static AudioRenderContext *CreateInstance();

public:
    uint32_t bufferFrameCount;

    WaveFormat format;
};

}
