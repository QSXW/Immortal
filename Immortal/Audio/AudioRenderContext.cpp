/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "AudioRenderContext.h"

#ifdef _WIN32
#include "WASAPI.h"
#elif defined(__linux__)
#include "ALSA.h"
#endif

namespace Immortal
{

AudioRenderContext *AudioRenderContext::CreateInstance()
{
#ifdef WASAPI_CONTEXT_H_
    return new WASAPIContext;
#endif

#ifdef ALSA_CONTEXT_H_
    return new ALSAContext;
#endif

    return nullptr;
}

}
