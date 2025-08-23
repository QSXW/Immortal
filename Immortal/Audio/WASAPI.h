/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#ifndef WASAPI_CONTEXT_H_
#define WASAPI_CONTEXT_H_

#include "IAudioDevice.h"
#include "AudioStream.h"

#include <mutex>
#include <Audioclient.h>

#ifndef INITGUID
#define INITGUID
#endif

#include <mmdeviceapi.h>
#include <wrl/client.h>

namespace Immortal
{
namespace WASAPI
{

class Device;
class DeviceChangeListener : public IClass, public IMMNotificationClient, public IObject
{
public:
	DeviceChangeListener(Device *device);

	ULONG STDMETHODCALLTYPE AddRef() override;

	ULONG STDMETHODCALLTYPE Release() override;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) override;

	HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;

	HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;

	HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;

	HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;

	HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override;

protected:
	Device *device;
};

using Microsoft::WRL::ComPtr;

class AudioStream : public IClass, public IAudioStream
{
public:
	AudioStream(ComPtr<IMMDevice> &device, ComPtr<IAudioClient> &&audioClient);

    virtual ~AudioStream() override;

	virtual bool Start() override;

	virtual bool Stop() override;

	virtual bool Reset() override;

	virtual bool BeginRender(uint32_t frames) override;

	virtual void WriteBuffer(const uint8_t *buffer, size_t size) override;

	virtual bool EndRender(uint32_t frames) override;

    virtual uint32_t GetAvailableFrameCount() override;

	virtual AudioFormat GetFormat() override;

    virtual bool OnDeviceChanged(IAudioDevice *device) override;

protected:
	bool OpenStream();

    void Release();

protected:
	ComPtr<IMMDevice> device;

    ComPtr<IAudioClient> audioClient;

	ComPtr<IAudioRenderClient> renderClient;

	ComPtr<IAudioClock> clock;

    WAVEFORMATEX *waveFormat;

    uint8_t *data;

    uint32_t bufferSize;
};

class Device : public IClass, public IAudioDevice
{
public:
    using Super = IAudioDevice;

    friend class DeviceChangeListener;

public:
    Device();

    virtual ~Device();

    virtual bool OpenDevice(const AudioDeviceInfo &deviceInfo = {}) override;

    virtual IAudioStream *CreateStream() override;

    virtual AudioFormat GetFormat() override;

    virtual bool SetOnEvent(const std::function<void(Event &)> &callback) override;

    virtual int EnumeratorDevices(AudioDeviceType type, AudioDeviceInfo *devices, uint32_t *numDevice) override;

    bool OpenDefaultDevice();

    void Release();

    void OnEvent(AudioDeviceEvent type, Event &event);

    ComPtr<IAudioClient> CreateAudioClient();

protected:
    ComPtr<IMMDeviceEnumerator> enumerator;

    ComPtr<IMMDevice> handle;

    WAVEFORMATEX *waveFormat;

    ComPtr<DeviceChangeListener> deviceChangeListener;

    EDataFlow flow;

    ERole role;

    std::mutex mutex;

    std::atomic_bool deviceChanged = false;

    std::function<void(Event &)> callback;
};

}
}

#endif
