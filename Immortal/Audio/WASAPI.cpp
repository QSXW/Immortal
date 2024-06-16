/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "WASAPI.h"

namespace Immortal
{
namespace WASAPI
{

static inline void Check(HRESULT hr)
{
    if (FAILED(hr))
    {
        abort();
    }
}

Device::Device() :
    Super{},
    waveFormat{},
    bufferSize{}
{
    OpenDevice();
}

Device::~Device()
{
    Release();
}

void Device::OpenDevice()
{
    Release();

    Check(CoInitializeEx(NULL, COINIT_MULTITHREADED));

    Check(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&enumerator)));

    Check(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &handle));

    Check(handle->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&audioClient));

    Check(audioClient->GetMixFormat(&waveFormat));

    Check(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, REFTIMES_PER_SEC, 0, waveFormat, NULL));

    Check(audioClient->GetService(IID_PPV_ARGS(&renderClient)));

    Check(audioClient->GetService(IID_PPV_ARGS(&clock)));

    Check(audioClient->GetBufferSize(&bufferSize));
}

void Device::Begin()
{
    Check(audioClient->Start());
}

void Device::End()
{
    Check(audioClient->Stop());
}

void Device::Reset()
{
    Check(audioClient->Reset());
}

void Device::Pause(bool enable)
{
    if (enable)
    {
        Check(audioClient->Stop());
    }
    else
    {
        Check(audioClient->Start());
    }
}

double Device::GetPostion()
{
    uint64_t position;
    Check(clock->GetPosition(&position, nullptr));

    uint64_t frequency;
    Check(clock->GetFrequency(&frequency));

    return (double)position / (double)frequency;
}

void Device::BeginRender(uint32_t frames)
{
	Check(renderClient->GetBuffer(frames, &data));
}

void Device::WriteBuffer(const uint8_t *buffer, size_t size)
{
    if (data)
    {
		memcpy(data, buffer, size);
    }
}

void Device::EndRender(uint32_t frames)
{
	Check(renderClient->ReleaseBuffer(frames, 0));
	data = nullptr;
}

uint32_t Device::GetAvailableFrameCount()
{
	uint32_t padding = 0;
	Check(audioClient->GetCurrentPadding(&padding));

    return bufferSize - padding;
}

AudioFormat Device::GetFormat()
{
    AudioFormat format = {
        .format     = Format::VECTOR2,
	    .channels   = (uint8_t)waveFormat->nChannels,
        .silence    = 0,
	    .sampleRate = waveFormat->nSamplesPerSec,
    };

    return format;
}

void Device::Release()
{
    if (waveFormat)
    {
        CoTaskMemFree(waveFormat);
        waveFormat = nullptr;
    }
}

}
}
