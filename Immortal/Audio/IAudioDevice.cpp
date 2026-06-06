/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "IAudioDevice.h"

#ifdef _WIN32
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
#ifdef WASAPI_CONTEXT_H_
    return new WASAPI::Device;
#endif

#ifdef ALSA_CONTEXT_H_
    return new ALSAContext;
#endif

#ifdef COREAUDIO_CONTEXT_H_
    return new CoreAudio::Device;
#endif

    return nullptr;
}

}
