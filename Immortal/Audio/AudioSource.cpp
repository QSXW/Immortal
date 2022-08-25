/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "AudioSource.h"
#include "Utils/PlatformUtils.h"

#ifdef _MSC_VER
#include <audioclient.h>
#endif

namespace Immortal
{

AudioSource::AudioSource(Ref<Vision::Interface::Codec> &codec) :
    codec{ codec },
    pts{}
{

}

#ifdef _MSC_VER
HRESULT AudioSource::LoadData(int numFramesAvaiable, void *pData, DWORD *flags)
{
    auto picture = codec->GetPicture();
    float* wave = (float*)&picture.Data()[pts];

    auto samples = std::min(picture.desc.width * sizeof(float) - pts, numFramesAvaiable * 2 * sizeof(float));

    if (!samples)
    {
        *flags |= AUDCLNT_BUFFERFLAGS_SILENT;
        return S_OK;
    }
    memcpy(pData, wave, samples);
    pts += samples;

    return S_OK;
}
#endif

bool AudioSource::InProgress() const
{
    auto picture = codec->GetPicture();
    return pts < picture.desc.width * sizeof(float);
}

AudioClip AudioSource::GetAudioClip(int frames)
{
    auto picture = codec->GetPicture();

    AudioClip clip{};
    clip.pData  = (const uint8_t *)&picture.Data()[pts];
    clip.bytes  = frames /* sample rate per frame */ * 2 /* channels */ * sizeof(float) /* format */;
    clip.frames = frames;

    pts += clip.bytes;
    return clip;
}

}
