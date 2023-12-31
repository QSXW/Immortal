/**
 * Copyright (C) 2023, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Texture.h"

namespace Immortal
{

struct CaptureFrameInfo
{
    uint64_t lastPresentTime;
    uint64_t lastMouseUpdateTime;
    uint32_t accumulatedFrames;
};

class WindowCapture : public IObject
{
public:
    virtual ~WindowCapture()
    {

    }

    virtual Texture *AcquireNextFrame(CaptureFrameInfo *pFrameInfo)
    {
        return nullptr;
    }
};

using SuperWindowCapture = WindowCapture;

}
