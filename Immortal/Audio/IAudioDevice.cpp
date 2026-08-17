/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "IAudioDevice.h"

#if defined(IMMORTAL_AUDIO_USE_SDL2)
#include "SDLDevice.h"
#elif defined(_WIN32)
#include "WASAPI.h"
#elif defined(__linux__)
#include "ALSA.h"
#elif defined(__APPLE__)
#include "CoreAudio.h"
#endif

namespace Immortal
{

IAudioDevice *IAudioDevice::CreateInstance()
{
#if defined(IMMORTAL_AUDIO_USE_SDL2)
	return new SDLAudio::Device;
#elif defined(WASAPI_CONTEXT_H_)
	return new WASAPI::Device;
#elif defined(ALSA_CONTEXT_H_)
	return new ALSAContext;
#elif defined(COREAUDIO_CONTEXT_H_)
	return new CoreAudio::Device;
#else
	return nullptr;
#endif
}

}
