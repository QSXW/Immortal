/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#ifndef WASAPI_CONTEXT_H_
#define WASAPI_CONTEXT_H_

#include "IAudioDevice.h"

#include <mutex>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <wrl/client.h>

namespace Immortal
{
namespace WASAPI
{

using Microsoft::WRL::ComPtr;
class Device : public IAudioDevice
{
public:
    using Super = IAudioDevice;

public:
    Device();

    virtual ~Device();

    virtual void OpenDevice() override;

    virtual void Begin() override;

    virtual void End() override;

    virtual void Reset() override;

    virtual void Pause(bool enable) override;

    virtual double GetPostion() override;

    virtual void BeginRender(uint32_t frames) override;

    virtual void WriteBuffer(const uint8_t *buffer, size_t size) override;

	virtual void EndRender(uint32_t frames) override;

	virtual uint32_t GetAvailableFrameCount() override;

    virtual AudioFormat GetFormat() override;

    void Release();

protected:
    ComPtr<IMMDeviceEnumerator> enumerator;

    ComPtr<IMMDevice> handle;

    ComPtr<IAudioClient> audioClient;

    ComPtr<IAudioRenderClient> renderClient;

    ComPtr<IAudioClock> clock;

    uint32_t bufferSize;

    std::mutex mutex;

    WAVEFORMATEX *waveFormat;

    uint8_t *data;
};

}
}

#endif
