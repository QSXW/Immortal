/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Device.h"
#include "AudioSource.h"

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define Check(hres) if (FAILED(hres)) { \
    abort(); \
}

template <class T>
inline void IfNotNullThenRelease(T **ppInterface)
{
    T *pInterface = *ppInterface;
    if (pInterface)
    {
        pInterface->Release();
        *ppInterface = nullptr;
    }
}

namespace Immortal
{

AudioDevice::AudioDevice() :
    pEnumerator{},
    handle{},
    pAudioClient{},
    pRenderClient{},
    bufferFrameCount{},
    stopping{ false }
{
    AudioDevice *device = this;

#ifdef _MSC_VER
    {
        HRESULT hr = S_OK;

        LOG::INFO("Create Audio Device at Thread: {}", (void*)(uint64_t)Thread::Self());

        Check(CoInitialize(NULL));

        Check(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&device->pEnumerator)));

        Check(hr = device->pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device->handle));

        Check(hr = device->handle->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&device->pAudioClient));

        Check(hr = device->pAudioClient->GetMixFormat(&pWaveFormat));

        Check(hr = device->pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, REFTIMES_PER_SEC, 0, device->pWaveFormat, NULL));

        Check(hr = device->pAudioClient->GetService(IID_PPV_ARGS(&device->pRenderClient)));

        Check(hr = device->pAudioClient->GetBufferSize(&device->bufferFrameCount));
    }

    thread = new Thread{ [=] {
        LOG::INFO("Create Audio Rendering at Thread: {}", (void *)(uint64_t)Thread::Self());

        HRESULT hr = S_OK;

        DWORD flags = 0;

        uint8_t* pData = nullptr;
        auto hnsActualDuration = (double)REFTIMES_PER_SEC * device->bufferFrameCount / device->pWaveFormat->nSamplesPerSec;
        Check(hr = device->pAudioClient->Start());
        while (!device->stopping)
        {
            AudioClip audioClip{};
            {
                std::unique_lock<std::mutex> lock{ device->mutex };
                if (device->queue.empty())
                {
                    continue;
                }
                audioClip = device->queue.front();
                device->queue.pop();
            }

            while (audioClip.frames)
            {
                uint32_t numFramesPadding;
                Check(hr = device->pAudioClient->GetCurrentPadding(&numFramesPadding));
                uint32_t frameRequested = std::min(audioClip.frames, device->bufferFrameCount - numFramesPadding);

                Check(hr = device->pRenderClient->GetBuffer(frameRequested, &pData));

                audioClip.Cosume(pData, frameRequested);

                Check(hr = device->pRenderClient->ReleaseBuffer(frameRequested, flags));
            }

            auto hnsActualDuration = (double)REFTIMES_PER_SEC * audioClip.frames / device->pWaveFormat->nSamplesPerSec;
            Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
        }
        Check(device->pAudioClient->Stop());
        Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
    } }; 

    thread->Start();
#endif
}

AudioDevice::~AudioDevice()
{
#ifdef _MSC_VER
    stopping = true;
    thread.Reset();

    IfNotNullThenRelease(&pEnumerator);
    IfNotNullThenRelease(&handle);
    IfNotNullThenRelease(&pAudioClient);

    IfNotNullThenRelease(&pRenderClient);

    if (pWaveFormat)
    {
        CoTaskMemFree(pWaveFormat);
    }
#endif
}

void AudioDevice::PlayAudioStream(AudioSource *pAudioSource)
{
#ifdef _MSC_VER
    DWORD flags = 0;
    HRESULT hr  = S_OK;
    uint32_t numFramesPadding;
    uint32_t numFramesAvailable;
    uint8_t* pData = nullptr;

    Check(hr = pRenderClient->GetBuffer(bufferFrameCount, &pData));
    Check(hr = pAudioSource->LoadData(bufferFrameCount, pData, &flags));
    Check(hr = pRenderClient->ReleaseBuffer(bufferFrameCount, flags));

    auto hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pWaveFormat->nSamplesPerSec;

    Check(hr = pAudioClient->Start());
    while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
        Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

        Check(hr = pAudioClient->GetCurrentPadding(&numFramesPadding));

        numFramesAvailable = bufferFrameCount - numFramesPadding;

        Check(hr = pRenderClient->GetBuffer(numFramesAvailable, &pData));

        Check(hr = pAudioSource->LoadData(numFramesAvailable, pData, &flags));

        Check(hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags));
    }
    Check(hr = pAudioClient->Stop());
    Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
#endif
}

void AudioDevice::PlayClip(AudioClip pAudioClip)
{
    std::unique_lock<std::mutex> lock{ mutex };
    queue.push(pAudioClip);
}

}
