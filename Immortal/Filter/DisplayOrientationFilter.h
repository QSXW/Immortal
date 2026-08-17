/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Filter.h"

namespace Immortal
{

class DisplayOrientationFilter : public FilterNode
{
public:
    DisplayOrientationFilter(Device *device, int hflip, int vflip, int anticlockwiseRotation);

    void Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread()) override;

protected:
    Device *device;

    std::vector<Ref<DescriptorSet>> descriptorSets;

    Ref<Pipeline> pipeline;
    
    int hflip;

    int vflip;

    int anticlockwiseRotation;
};

}
