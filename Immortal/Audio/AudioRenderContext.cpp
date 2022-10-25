/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "AudioRenderContext.h"

#ifdef _WIN32
#include "WASAPI.h"
#endif

namespace Immortal
{

AudioRenderContext *AudioRenderContext::CreateInstance()
{
#ifdef __WASAPI_CONTEXT__
    return new WASAPIContext;
#endif

    return nullptr;
}

}
