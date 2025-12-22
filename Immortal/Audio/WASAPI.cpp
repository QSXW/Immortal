/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "WASAPI.h"
#include "String/IString.h"

#include <functiondiscoverykeys_devpkey.h>

namespace Immortal
{
namespace WASAPI
{

static inline bool Check(HRESULT hr)
{
    if (FAILED(hr))
    {
		abort();
    }
}

#define WASAPI_CHECK(x) if (x) return false;

DeviceChangeListener::DeviceChangeListener(Device *device) :
    ICLASS,
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

	AudioDeviceStateChangedEvent event;
	device->OnEvent(AudioDeviceEvent_OnDeviceStateChanged, event);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
	CLOG_INFO("Device added");

	AudioDeviceAddedEvent event;
	device->OnEvent(AudioDeviceEvent_OnDeviceAdded, event);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
	wchar_t id[64];
	CLOG_INFO("Device removed: {}", WString2String(pwstrDeviceId));

	AudioDeviceRemovedEvent event;
	if (device->handle->GetId((wchar_t **)&id) == S_OK && lstrcmpW(id, pwstrDeviceId))
	{
		device->OnEvent(AudioDeviceEvent_OnDeviceRemoved, event);
    }

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR defaultDeviceId)
{
	if (role == device->role)
	{
		CLOG_INFO("Default device changed from {} {} {}", defaultDeviceId ? WString2String(defaultDeviceId) : "", (int) flow, (int) role);
		AudioDefaultDeviceChangedEvent event = {};
		device->OnEvent(AudioDeviceEvent_OnDefaultDeviceChanged, event);
    }

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DeviceChangeListener::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
	CLOG_INFO("Property value changed");

	AudioDevicePropertyValueChangedEvent event;
	device->OnEvent(AudioDeviceEvent_OnPropertyValueChanged, event);
	return S_OK;
}

AudioStream::AudioStream(ComPtr<IMMDevice> &device, ComPtr<IAudioClient> &&_audioClient) :
    IClass{"WASPIAudioStream"},
    IAudioStream{},
    device{ device },
    audioClient{std::move(_audioClient)},
    renderClient{},
    clock{},
    waveFormat{},
    data{},
    bufferSize{}
{
	OpenStream();
}

AudioStream::~AudioStream()
{
	Release();
}

void AudioStream::Release()
{
	Destroy();
	if (waveFormat)
	{
		CoTaskMemFree(waveFormat);
		waveFormat = nullptr;
	}
}

bool AudioStream::OpenStream()
{
	WASAPI_CHECK(audioClient->GetMixFormat(&waveFormat));

	WASAPI_CHECK(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, REFTIMES_PER_SEC, 0, waveFormat, NULL));

	WASAPI_CHECK(audioClient->GetService(IID_PPV_ARGS(&renderClient)));

	WASAPI_CHECK(audioClient->GetService(IID_PPV_ARGS(&clock)));

	WASAPI_CHECK(audioClient->GetBufferSize(&bufferSize));

	bytePerSample = (waveFormat->wBitsPerSample >> 3) * waveFormat->nChannels;

	return true;
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
        .format     = Format::FLOAT,
	    .channels   = (int)waveFormat->nChannels,
        .silence    = 0,
	    .sampleRate = (int)waveFormat->nSamplesPerSec,
    };

    return format;
}

bool AudioStream::OnDeviceChanged(IAudioDevice *_device)
{
	std::lock_guard lock{ mutex };

	Device *device = InterpretAs<Device>(_device);
	audioClient = device->CreateAudioClient();
	if (!audioClient)
	{
		CLOG_ERROR("Error when creating audio client");
		return false;
	}

	return OpenStream();
}

Device::Device() :
    IClass{"WASAPI"},
    Super{},
    flow{ eRender },
    role{ eMultimedia },
    callback{},
    waveFormat{}
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

bool Device::OpenDevice(const AudioDeviceInfo &deviceInfo)
{
    Release();

	if (deviceInfo.uuid.empty())
	{
		return OpenDefaultDevice();
	}

	if (FAILED(enumerator->GetDevice(String2WString(deviceInfo.uuid).c_str(), &handle)))
	{
		CLOG_ERROR("Error when opening {}", deviceInfo.name);
		return false;
	}

	(void) CreateAudioClient();

	return true;
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

	(void)CreateAudioClient();

	return true;
}

IAudioStream *Device::CreateStream()
{
	HRESULT ret;
	ComPtr<IAudioClient> audioClient = CreateAudioClient();
	return new AudioStream{ handle,  std::move(audioClient) };
}

static Format CAST(const GUID &guid, WORD bitPerSample)
{
	if (IsEqualGUID(guid, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
	{
		return Format::FLOAT;
	}
	else if (IsEqualGUID(guid, KSDATAFORMAT_SUBTYPE_PCM))
	{
		switch (bitPerSample)
		{
			case 8:
				return Format::R8_SINT;
				break;
			case 16:
				return Format::R16_SINT;
			case 24:
			case 32:
				return Format::R32_SINT;
			default:
				break;
		}
	}

	return Format::None;
}

static Format CAST(WORD format, WORD bitPerSample)
{
	if (format == WAVE_FORMAT_IEEE_FLOAT)
	{
		return Format::FLOAT;
	}
	else if (format == WAVE_FORMAT_PCM)
	{
		switch (bitPerSample)
		{
			case 8:
				return Format::R8_SINT;
			case 16:
				return Format::R16_SINT;
			case 24:
			case 32:
				return Format::R32_SINT;
			default:
				break;
		}
	}

	return Format::None;
}

AudioFormat Device::GetFormat()
{
	if (!waveFormat)
	{
		return {};
	}

	AudioFormat format{
	    .format     = Format::None,
		.channels   = (int)waveFormat->nChannels,
		.sampleRate = (int)waveFormat->nSamplesPerSec,
		.mask       = 0,
	};

	const WAVEFORMATEXTENSIBLE *ext = NULL;
	if (waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
	    waveFormat->cbSize >= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
	{
		ext = (const WAVEFORMATEXTENSIBLE *) waveFormat;
		format.mask = ext->dwChannelMask;
		format.format = CAST(ext->SubFormat, waveFormat->wBitsPerSample);
	}
	else
	{
		format.format = CAST(waveFormat->wFormatTag, waveFormat->wBitsPerSample);
		for (int i = 0; i < waveFormat->nChannels; i++)
		{
			format.mask |= (1llu << i);
		}
	}

	return format;
}

bool Device::SetOnEvent(const std::function<void(Event &)> &_callback)
{
    callback = _callback;
	return true;
}

int Device::EnumeratorDevices(AudioDeviceType type, AudioDeviceInfo *devices, uint32_t *numDevice)
{
	ComPtr<IMMDeviceCollection> deviceCollection;
	HRESULT hr = enumerator->EnumAudioEndpoints(type == AudioDeviceType::Capture ? eCapture : eRender, DEVICE_STATE_ACTIVE, &deviceCollection);
	if (FAILED(hr))
	{
		*numDevice = 0;
		CLOG_ERROR("Error when calling EnumAudioEndpoints");
		return -1;
	}

	deviceCollection->GetCount(numDevice);
	if (!devices)
	{
		return 0;
	}

	for (UINT i = 0; i < *numDevice; i++)
	{
		auto &info = devices[i];
		info.type = type;

		ComPtr<IMMDevice> device;
		hr = deviceCollection->Item(i, &device);

		if (FAILED(hr))
		{
			continue;
		}

		LPWSTR deviceId = NULL;
		hr = device->GetId(&deviceId);
		if (FAILED(hr))
		{
			return -1;
		}

		info.uuid = WString2String(deviceId);
		CoTaskMemFree(deviceId);

		ComPtr<IPropertyStore> pProps;
		hr = device->OpenPropertyStore(STGM_READ, &pProps);
		if (SUCCEEDED(hr))
		{
			PROPVARIANT prop;
			PropVariantInit(&prop);
			hr = pProps->GetValue(PKEY_Device_FriendlyName, &prop);

			if (SUCCEEDED(hr) && prop.vt == VT_LPWSTR)
			{
				info.name = WString2String(prop.pwszVal);
			}

			PropVariantClear(&prop);
		}
	}

	return 0;
}

void Device::Release()
{
	if (waveFormat)
	{
		CoTaskMemFree(waveFormat);
		waveFormat = nullptr;
	}
	handle.Reset();
}

void Device::OnEvent(AudioDeviceEvent type, Event &event)
{
	if (callback)
	{
		callback(event);
	}
}

ComPtr<IAudioClient> Device::CreateAudioClient()
{
	ComPtr<IAudioClient> audioClient;
	HRESULT ret = handle->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **)&audioClient);
	if (FAILED(ret))
	{
		CLOG_ERROR("Failed to activate audio client!");
		return nullptr;
	}

	if (!waveFormat)
	{
		if (FAILED(audioClient->GetMixFormat(&waveFormat)))
		{
			CLOG_ERROR("Error when getting mix format!");
			return nullptr;
		}
	}

	return audioClient;
}

}
}
