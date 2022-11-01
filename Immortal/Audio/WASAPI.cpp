/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "WASAPI.h"

namespace Immortal
{

static inline void Check(HRESULT hr)
{
    if (FAILED(hr))
    {
        abort();
    }
}

WASAPIContext::WASAPIContext() :
    Super{},
    waveFormat{}
{
    OpenDevice();
}

WASAPIContext::~WASAPIContext()
{
    Release();
}

void WASAPIContext::OpenDevice()
{
    Release();

    Check(CoInitializeEx(NULL, COINIT_MULTITHREADED));

    Check(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&enumerator)));

    Check(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &handle));

    Check(handle->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&audioClient));

    Check(audioClient->GetMixFormat(&waveFormat));

    Check(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, REFTIMES_PER_SEC, 0, waveFormat, NULL));

    Check(audioClient->GetService(IID_PPV_ARGS(&renderClient)));

    Check(audioClient->GetBufferSize(&bufferFrameCount));

    format.Channels = waveFormat->nChannels;
    format.SampleRate = waveFormat->nSamplesPerSec;
}

void WASAPIContext::Begin()
{
    Check(audioClient->Start());
}

void WASAPIContext::End()
{
    Check(audioClient->Stop());
}

void WASAPIContext::Reset()
{
    Check(audioClient->Reset());
}

void WASAPIContext::PlaySamples(uint32_t numberSamples, const uint8_t *pSamples)
{
    while (numberSamples > 0)
    {
        uint8_t *pData;

        uint32_t numFramesPadding;
        Check(audioClient->GetCurrentPadding(&numFramesPadding));

        uint32_t frameRequested = std::min(numberSamples, bufferFrameCount - numFramesPadding);

        Check(renderClient->GetBuffer(frameRequested, &pData));

        uint32_t bytes = frameRequested << 3;
        memcpy(pData, pSamples, bytes);
        pSamples += bytes;
        numberSamples -= frameRequested;

        Check(renderClient->ReleaseBuffer(frameRequested, 0));
    }
}

void WASAPIContext::Release()
{
    if (waveFormat)
    {
        CoTaskMemFree(waveFormat);
        waveFormat = nullptr;
    }
}

}
