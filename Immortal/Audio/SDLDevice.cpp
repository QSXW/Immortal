/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 */

#include "SDLDevice.h"

#include "Shared/Log.h"

#include <atomic>
#include <cstdlib>
#include <cstring>

namespace Immortal
{
namespace SDLAudio
{

namespace
{

static ::Immortal::Format MapSdlToFormat(Uint16 f)
{
	switch (f)
	{
	case AUDIO_F32SYS:
		return ::Immortal::Format{ ::Immortal::Format::FLOAT };
	case AUDIO_S16SYS:
		return ::Immortal::Format{ ::Immortal::Format::R16_SINT };
	case AUDIO_S32SYS:
		return ::Immortal::Format{ ::Immortal::Format::R32_SINT };
	case AUDIO_U8:
		return ::Immortal::Format{ ::Immortal::Format::R8_SINT };
	default:
		return ::Immortal::Format{ ::Immortal::Format::FLOAT };
	}
}

static Uint16 MapFormatToSdl(::Immortal::Format f)
{
	using FV = ::Immortal::Format::ValueType;
	switch (static_cast<FV>(f))
	{
	case ::Immortal::Format::FLOAT:
		return AUDIO_F32SYS;
	case ::Immortal::Format::R16_SINT:
		return AUDIO_S16SYS;
	case ::Immortal::Format::R32_SINT:
		return AUDIO_S32SYS;
	case ::Immortal::Format::R8_SINT:
		return AUDIO_U8;
	default:
		return AUDIO_F32SYS;
	}
}

static AudioFormat ToAudioFormat(const SDL_AudioSpec &s)
{
	AudioFormat out{};
	out.format     = MapSdlToFormat(s.format);
	out.channels   = (int)s.channels;
	out.sampleRate = (int)s.freq;
	out.silence    = 0;
	out.mask       = 0;
	for (int i = 0; i < out.channels && i < 64; i++)
	{
		out.mask |= (1ull << i);
	}
	return out;
}

static int BytesPerSdlSample(Uint16 fmt)
{
	return (int)SDL_AUDIO_BITSIZE(fmt) / 8;
}

static uint8_t SilenceByteForFormat(Uint16 fmt)
{
	switch (fmt)
	{
	case AUDIO_U8:
		return 128;
	default:
		return 0;
	}
}

static std::mutex gSdlAudioSubsystemMutex;
static int gSdlAudioSubsystemRefs = 0;

static bool RefSdlAudioSubsystem(const char *logContext)
{
	(void)logContext;
	std::lock_guard lock{ gSdlAudioSubsystemMutex };
	if (gSdlAudioSubsystemRefs == 0 && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
	{
		LOG::ERR("SDL_InitSubSystem(SDL_INIT_AUDIO): {}", SDL_GetError());
		return false;
	}
	++gSdlAudioSubsystemRefs;
	return true;
}

static void UnrefSdlAudioSubsystem()
{
	std::lock_guard lock{ gSdlAudioSubsystemMutex };
	if (gSdlAudioSubsystemRefs <= 0)
	{
		return;
	}
	if (--gSdlAudioSubsystemRefs == 0)
	{
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
}

} // namespace

void Device::BumpSdlInit()
{
	RefSdlAudioSubsystem("Device");
}

void Device::DropSdlInit()
{
	UnrefSdlAudioSubsystem();
}

Device::Device() :
    IClass{ "SDLAudio" },
    Super{}
{
	// Match WASAPI/ALSA: probe default device so GetFormat() works before CreateStream.
	(void)OpenDevice();
}

Device::~Device()
{
	if (sdlBumpFromOpen)
	{
		DropSdlInit();
		sdlBumpFromOpen = false;
	}
}

bool Device::OpenDevice(const AudioDeviceInfo &deviceInfo)
{
	std::lock_guard lock{ mutex };

	const bool bumpedHere = !sdlBumpFromOpen;
	if (bumpedHere)
	{
		BumpSdlInit();
	}

	preferredDeviceIndex = -1;
	preferredDeviceName.clear();
	if (!deviceInfo.uuid.empty())
	{
		// Accept numeric index from EnumeratorDevices (uuid string) or pass-through name.
		char *end = nullptr;
		const long idx = std::strtol(deviceInfo.uuid.c_str(), &end, 10);
		if (end != deviceInfo.uuid.c_str() && *end == '\0' && idx >= 0)
		{
			preferredDeviceIndex = (int)idx;
		}
		else
		{
			preferredDeviceName = deviceInfo.uuid;
		}
	}

	hasProbedFormat = false;

	SDL_AudioSpec want;
	SDL_zero(want);
	want.freq     = 48000;
	want.format   = AUDIO_F32SYS;
	want.channels = 2;
	want.samples  = 1024;

	const char *devName = nullptr;
	if (preferredDeviceIndex >= 0)
	{
		devName = SDL_GetAudioDeviceName(preferredDeviceIndex, 0);
	}
	else if (!preferredDeviceName.empty())
	{
		devName = preferredDeviceName.c_str();
	}

	SDL_AudioSpec have;
	SDL_zero(have);
	SDL_AudioDeviceID probe =
	    SDL_OpenAudioDevice(devName, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
	if (!probe)
	{
		CLOG_ERROR("SDL_OpenAudioDevice (probe): {}", SDL_GetError());
		if (bumpedHere)
		{
			DropSdlInit();
		}
		return false;
	}

	probedFormat    = ToAudioFormat(have);
	hasProbedFormat = true;
	sdlBumpFromOpen = true;
	SDL_CloseAudioDevice(probe);
	return true;
}

IAudioStream *Device::CreateStream()
{
	return new AudioStream{ this };
}

AudioFormat Device::GetFormat()
{
	std::lock_guard lock{ mutex };
	if (hasProbedFormat)
	{
		return probedFormat;
	}
	AudioFormat def{};
	def.format     = ::Immortal::Format{ ::Immortal::Format::FLOAT };
	def.channels   = 2;
	def.sampleRate = 48000;
	def.silence    = 0;
	def.mask       = 3;
	return def;
}

bool Device::SetOnEvent(const std::function<void(Event &)> &callback)
{
	std::lock_guard lock{ mutex };
	onEvent = callback;
	return true;
}

int Device::EnumeratorDevices(AudioDeviceType type, AudioDeviceInfo *devices, uint32_t *numDevice)
{
	if (type != AudioDeviceType::Render)
	{
		*numDevice = 0;
		return 0;
	}

	BumpSdlInit();
	const int n = SDL_GetNumAudioDevices(0);
	if (n < 0)
	{
		*numDevice = 0;
		DropSdlInit();
		return -1;
	}

	*numDevice = (uint32_t)n;
	if (!devices)
	{
		DropSdlInit();
		return 0;
	}

	for (int i = 0; i < n; i++)
	{
		const char *name = SDL_GetAudioDeviceName(i, 0);
		devices[i].type = AudioDeviceType::Render;
		devices[i].name = name ? name : "";
		devices[i].uuid = std::to_string(i);
	}

	DropSdlInit();
	return 0;
}

void Device::NotifyProbeFormat(const SDL_AudioSpec &spec)
{
	std::lock_guard lock{ mutex };
	probedFormat    = ToAudioFormat(spec);
	hasProbedFormat = true;
}

AudioStream::AudioStream(Device *owner) :
    IClass{ "SDLAudioStream" },
    Immortal::AudioStream{},
    owner{ owner }
{
}

AudioStream::~AudioStream()
{
	CloseDevice();
	Destroy();
}

void AudioStream::CloseDevice()
{
	std::lock_guard lifecycleLock{ lifecycleMutex };
	closing.store(true, std::memory_order_release);

	const SDL_AudioDeviceID closingDevice = deviceId;
	deviceId = 0;
	if (closingDevice)
	{
		// Pause first so an in-flight callback has finished before its owner and
		// callback functor are released. SDL_CloseAudioDevice then only has to
		// terminate the already-quiescent backend thread.
		SDL_PauseAudioDevice(closingDevice, 1);
	}
	{
		std::lock_guard callbackLock{ mutex };
		callback = {};
	}
	if (closingDevice)
	{
		SDL_CloseAudioDevice(closingDevice);
	}

	have = {};
	bytesPerFrame = 0;
	bytePerSample = 0;
	silenceByte = 0;
	if (streamHoldsSdlSubsystemRef)
	{
		UnrefSdlAudioSubsystem();
		streamHoldsSdlSubsystemRef = false;
	}
	owner = nullptr;
}

bool AudioStream::Start()
{
	std::lock_guard lifecycleLock{ lifecycleMutex };
	if (deviceId && !closing.load(std::memory_order_acquire))
	{
		SDL_PauseAudioDevice(deviceId, 0);
		return true;
	}
	return false;
}

bool AudioStream::Stop()
{
	std::lock_guard lifecycleLock{ lifecycleMutex };
	if (deviceId)
	{
		SDL_PauseAudioDevice(deviceId, 1);
		return true;
	}
	return false;
}

bool AudioStream::Reset()
{
	std::lock_guard lifecycleLock{ lifecycleMutex };
	if (deviceId)
	{
		SDL_PauseAudioDevice(deviceId, 1);
		return true;
	}
	return false;
}

bool AudioStream::BeginRender(uint32_t frames)
{
	(void)frames;
	return true;
}

void AudioStream::WriteBuffer(const uint8_t *buffer, size_t size)
{
	(void)buffer;
	(void)size;
}

bool AudioStream::EndRender(uint32_t frames)
{
	(void)frames;
	return true;
}

uint32_t AudioStream::GetAvailableFrameCount()
{
	return 1u << 20;
}

AudioFormat AudioStream::GetFormat()
{
	std::lock_guard lifecycleLock{ lifecycleMutex };
	if (!have.format)
	{
		return owner ? owner->GetFormat() : AudioFormat{};
	}
	return ToAudioFormat(have);
}

bool AudioStream::OnDeviceChanged(IAudioDevice *device)
{
	(void)device;
	CloseDevice();
	return true;
}

uint32_t AudioStream::FfplayAudioHwBufferBytes() const
{
	std::lock_guard lifecycleLock{ lifecycleMutex };
	if (!have.format || !bytesPerFrame)
	{
		return 0;
	}
	return (uint32_t)have.samples * bytesPerFrame;
}

void AudioStream::Start(const PFN_AudioStreamPlayCallback &value)
{
	std::lock_guard lifecycleLock{ lifecycleMutex };
	closing.store(false, std::memory_order_release);
	{
		std::lock_guard callbackLock{ mutex };
		callback = value;
	}

	if (!owner)
	{
		return;
	}

	streamHoldsSdlSubsystemRef = RefSdlAudioSubsystem("AudioStream::Start");
	if (!streamHoldsSdlSubsystemRef)
	{
		closing.store(true, std::memory_order_release);
		std::lock_guard callbackLock{ mutex };
		callback = {};
		return;
	}

	SDL_AudioSpec want;
	SDL_zero(want);

	AudioFormat df = owner->GetFormat();
	want.freq     = df.sampleRate > 0 ? df.sampleRate : 48000;
	want.format   = MapFormatToSdl(df.format);
	want.channels = df.channels > 0 ? (Uint8)df.channels : 2;
	want.samples  = 1024;
	want.callback = SdlAudioCallback;
	want.userdata = this;

	const char *devName = nullptr;
	if (owner->preferredDeviceIndex >= 0)
	{
		devName = SDL_GetAudioDeviceName(owner->preferredDeviceIndex, 0);
	}
	else if (!owner->preferredDeviceName.empty())
	{
		devName = owner->preferredDeviceName.c_str();
	}

	SDL_AudioSpec obt;
	SDL_zero(obt);
	deviceId = SDL_OpenAudioDevice(devName, 0, &want, &obt,
	                               SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
	if (!deviceId)
	{
		CLOG_ERROR("SDL_OpenAudioDevice: {}", SDL_GetError());
		closing.store(true, std::memory_order_release);
		{
			std::lock_guard callbackLock{ mutex };
			callback = {};
		}
		if (streamHoldsSdlSubsystemRef)
		{
			UnrefSdlAudioSubsystem();
			streamHoldsSdlSubsystemRef = false;
		}
		owner = nullptr;
		return;
	}

	have           = obt;
	bytesPerFrame  = (uint32_t)obt.channels * (uint32_t)BytesPerSdlSample(obt.format);
	bytePerSample  = (uint32_t)bytesPerFrame;
	silenceByte    = SilenceByteForFormat(obt.format);

	owner->NotifyProbeFormat(obt);

	SDL_PauseAudioDevice(deviceId, 0);
}

void SDLCALL AudioStream::SdlAudioCallback(void *userdata, Uint8 *stream, int len)
{
	auto *self = static_cast<AudioStream *>(userdata);
	if (!self || !stream || len <= 0)
	{
		return;
	}
	if (self->closing.load(std::memory_order_acquire))
	{
		std::memset(stream, self->silenceByte, (size_t)len);
		return;
	}

	std::lock_guard lock{ self->mutex };
	if (self->closing.load(std::memory_order_acquire))
	{
		std::memset(stream, self->silenceByte, (size_t)len);
		return;
	}

	if (!self->bytesPerFrame)
	{
		std::memset(stream, 0, (size_t)len);
		return;
	}

	const uint32_t frames = (uint32_t)len / self->bytesPerFrame;
	if (!self->callback)
	{
		std::memset(stream, self->silenceByte, (size_t)len);
		return;
	}

	const uint32_t got = self->callback(stream, frames);
	if (got < frames)
	{
		const size_t filled = (size_t)got * (size_t)self->bytesPerFrame;
		const size_t rest   = (size_t)len - filled;
		if (rest)
		{
			std::memset(stream + filled, self->silenceByte, rest);
		}
	}
}

}
}
