/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"
#include "Vision/Audio/WAV.h"

namespace Immortal
{

struct AudioClip;
class AudioSource
{
public:
    AudioSource(Ref<Vision::Interface::Codec> &codec);

#ifdef _MSC_VER
    HRESULT LoadData(int numFramesAvaiable, void* pData, DWORD* flags);
#endif 

    bool InProgress() const;

    AudioClip GetAudioClip(int frames);

protected:
    Ref<Vision::Interface::Codec> codec;

    int pts;
};

struct AudioClip
{
    AudioClip() :
        bytes{},
        pData{},
        frames{}
    {}

    int Cosume(void *dst, uint32_t frameRequested)
    {
        size_t samples = frameRequested * 2 * sizeof(float);
        memcpy(dst, pData, samples);
        pData  += samples;
        bytes  -= samples;
        frames -= frameRequested;

        return bytes;
    }

    size_t bytes;

    const uint8_t *pData;

    uint32_t frames;
};

}
