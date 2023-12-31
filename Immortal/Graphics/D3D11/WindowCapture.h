/**
 * Copyright (C) 2023, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Graphics/WindowCapture.h"
#include "Common.h"

namespace Immortal
{

namespace D3D11
{

class Device;
class WindowCapture : public SuperWindowCapture
{
public:
    WindowCapture(Device *device);

    virtual ~WindowCapture() override;

    virtual SuperTexture *AcquireNextFrame(CaptureFrameInfo *pFrameInfo) override;

protected:
    Device *device;

    ComPtr<IDXGIOutput5> output;

    ComPtr<IDXGIOutputDuplication> outputDuplication;
};

}

}
