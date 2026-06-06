/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "WASAPI.h"
#include "String/IString.h"

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

DeviceChangeListener::DeviceChangeListener(Device *device) :
    device{ device }
{

}

ULONG STDMETHODCALLTYPE DeviceChangeListener::AddRef()
{
	return IObject::AddRef();
}

ULONG STDMETHODCALLTYPE DeviceChangeListener::Release()
{
	return IObject::UnRef();
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::QueryInterface(REFIID riid, VOID **ppvInterface)
{
	if (riid == IID_IUnknown || riid == __uuidof(IMMNotificationClient))
	{
		*ppvInterface = (IMMNotificationClient *) this;
		return S_OK;
	}
	else
	{
		*ppvInterface = NULL;
		return E_NOINTERFACE;
	}
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
	LOG::DEBUG("Device state changed");
	device->OnEvent(AudioDeviceEvent_OnDeviceStateChanged);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
	LOG::DEBUG("Device added");
	device->OnEvent(AudioDeviceEvent_OnDeviceAdded);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
	wchar_t id[64];
	LOG::DEBUG("Device removed: {}", WString2String(pwstrDeviceId));
	if (device->handle->GetId((wchar_t **)&id) == S_OK && lstrcmpW(id, pwstrDeviceId))
	{
		device->OnEvent(AudioDeviceEvent_OnDeviceRemoved);
    }

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
	if (pwstrDefaultDeviceId)
	{
		LOG::DEBUG("Default device changed: {} {} {}", WString2String(pwstrDefaultDeviceId), (int) flow, (int) role);
	}
	if (role == device->role)
	{
		device->OnEvent(AudioDeviceEvent_OnDefaultDeviceChanged);
    }

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
	LOG::DEBUG("Property value changed");
	device->OnEvent(AudioDeviceEvent_OnPropertyValueChanged);
	return S_OK;
}

AudioStream::AudioStream(ComPtr<IAudioClient> &&_audioClient) :
    IAudioStream{},
    audioClient{std::move(_audioClient)},
    renderClient{},
    clock{},
    waveFormat{},
	data{},
    bufferSize{}
{
	Check(audioClient->GetMixFormat(&waveFormat));

	Check(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, REFTIMES_PER_SEC, 0, waveFormat, NULL));

	Check(audioClient->GetService(IID_PPV_ARGS(&renderClient)));

	Check(audioClient->GetService(IID_PPV_ARGS(&clock)));

	Check(audioClient->GetBufferSize(&bufferSize));
}

AudioStream::~AudioStream()
{
	IAudioStream::~AudioStream();
	if (waveFormat)
	{
		CoTaskMemFree(waveFormat);
		waveFormat = nullptr;
	}
}

bool AudioStream::Start()
{
	return audioClient->Start() == S_OK;
}

bool AudioStream::Stop()
{
	return audioClient->Stop() == S_OK;
}

bool AudioStream::Reset()
{
	return audioClient->Reset() == S_OK;
}

bool AudioStream::BeginRender(uint32_t frames)
{
	return renderClient->GetBuffer(frames, &data) == S_OK;
}

void AudioStream::WriteBuffer(const uint8_t *buffer, size_t size)
{
	if (data)
	{
		memcpy(data, buffer, size);
	}
}

bool AudioStream::EndRender(uint32_t frames)
{
	data = nullptr;
	return renderClient->ReleaseBuffer(frames, 0) == S_OK;
}

uint32_t AudioStream::GetAvailableFrameCount()
{
	uint32_t padding = 0;
	if (FAILED(audioClient->GetCurrentPadding(&padding)))
	{
		LOG::ERR("Failed to get current padding!");
		return padding;
	}

	return bufferSize - padding;
}

AudioFormat AudioStream::GetFormat()
{
	AudioFormat format = {
        .format     = Format::VECTOR2,
	    .channels   = (uint8_t)waveFormat->nChannels,
        .silence    = 0,
	    .sampleRate = waveFormat->nSamplesPerSec,
    };

    return format;
}

Device::Device() :
    Super{},
    flow{ eRender },
    role{ eMultimedia },
    callbacks{}
{
	Check(CoInitializeEx(NULL, COINIT_MULTITHREADED));

	Check(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&enumerator)));

	deviceChangeListener = new DeviceChangeListener{this};

	enumerator->RegisterEndpointNotificationCallback(deviceChangeListener.Get());

    OpenDevice();
}

Device::~Device()
{
    Release();
}

bool Device::OpenDevice()
{
    Release();

	return OpenDefaultDevice();
}

bool Device::OpenDefaultDevice()
{
	HRESULT ret;
	ret = enumerator->GetDefaultAudioEndpoint(flow, role, &handle);
	if (FAILED(ret))
	{
		LOG::ERR("Failed to GetDefaultAudioEndpoint!");
		return false;
	}

	return true;
}

IAudioStream *Device::CreateStream()
{
	HRESULT ret;
	ComPtr<IAudioClient> audioClient;
	ret = handle->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **) &audioClient);
	if (FAILED(ret))
	{
		LOG::ERR("Failed to activate audio client!");
		return nullptr;
	}

	return new AudioStream{ std::move(audioClient) };
}

bool Device::RegisterCallback(AudioDeviceEvent type, const std::function<void()> &callback)
{
	if (type >= NumAudioDeviceEvent)
    {
		return false;
    }

    callbacks[type] = callback;
	return true;
}

void Device::Release()
{

}

void Device::OnEvent(AudioDeviceEvent type)
{
	if (callbacks[type])
	{
		callbacks[type]();
	}
}

}
}
