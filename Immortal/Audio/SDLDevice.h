/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * SDL2 audio output — callback-driven timing like fftools/ffplay.c (sdl_audio_callback).
 */

#pragma once

#ifndef IMMORTAL_SDL_AUDIO_DEVICE_H_
#define IMMORTAL_SDL_AUDIO_DEVICE_H_

#include "IAudioDevice.h"
#include "AudioStream.h"

#include <SDL.h>

#include <atomic>
#include <mutex>
#include <string>

namespace Immortal
{
namespace SDLAudio
{

class Device;

class AudioStream final : public IClass, public Immortal::AudioStream
{
public:
	explicit AudioStream(Device *owner);

	~AudioStream() override;

	bool Start() override;

	bool Stop() override;

	bool Reset() override;

	bool BeginRender(uint32_t frames) override;

	void WriteBuffer(const uint8_t *buffer, size_t size) override;

	bool EndRender(uint32_t frames) override;

	uint32_t GetAvailableFrameCount() override;

	AudioFormat GetFormat() override;

	bool OnDeviceChanged(IAudioDevice *device) override;

	uint32_t FfplayAudioHwBufferBytes() const override;

	void Start(const PFN_AudioStreamPlayCallback &callback) override;

protected:
	void CloseDevice();

	static void SDLCALL SdlAudioCallback(void *userdata, Uint8 *stream, int len);

protected:
	Device *owner;

	/// Serializes open/pause/close so shutdown cannot race playback controls.
	mutable std::mutex lifecycleMutex;

	/// Prevents a callback from entering its owner while SDL is closing the device.
	std::atomic_bool closing{ false };

	SDL_AudioDeviceID deviceId = 0;

	SDL_AudioSpec have{};

	uint32_t bytesPerFrame = 0;

	uint8_t silenceByte = 0;

	/// One RefSdlAudioSubsystem per opened stream (paired with Unref in CloseDevice).
	bool streamHoldsSdlSubsystemRef = false;
};

class Device final : public IClass, public IAudioDevice
{
	friend class AudioStream;

public:
	using Super = IAudioDevice;

	Device();

	~Device() override;

	bool OpenDevice(const AudioDeviceInfo &deviceInfo = {}) override;

	IAudioStream *CreateStream() override;

	AudioFormat GetFormat() override;

	bool SetOnEvent(const std::function<void(Event &)> &callback) override;

	int EnumeratorDevices(AudioDeviceType type, AudioDeviceInfo *devices, uint32_t *numDevice) override;

protected:
	void NotifyProbeFormat(const SDL_AudioSpec &spec);

	void BumpSdlInit();

	void DropSdlInit();

protected:
	std::mutex mutex;

	std::function<void(Event &)> onEvent;

	AudioFormat probedFormat{};

	bool hasProbedFormat = false;

	int preferredDeviceIndex = -1;

	std::string preferredDeviceName;

	/// One refcount bump per Device instance after a successful OpenDevice probe.
	bool sdlBumpFromOpen = false;
};

}
}

#endif
