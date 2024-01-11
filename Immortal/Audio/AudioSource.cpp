/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "AudioSource.h"
#include "Helper/Platform.h"

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
    float *wave = (float*)&picture.GetData()[pts];

    auto samples = std::min(picture.GetWidth() * sizeof(float) - pts, numFramesAvaiable * 2 * sizeof(float));

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
	return pts < picture.GetWidth() * sizeof(float);
}

AudioClip AudioSource::GetAudioClip(int frames)
{
    auto picture = codec->GetPicture();

    if (!frames)
    {
		frames = picture.GetWidth();
    }

    AudioClip clip{ picture };
    clip.pData  = (const uint8_t *)&picture.GetData()[pts];
    clip.bytes  = frames /* sample rate per frame */ * 2 /* channels */ * sizeof(float) /* format */;
    clip.frames = frames;

    pts += clip.bytes;
    return clip;
}

}
