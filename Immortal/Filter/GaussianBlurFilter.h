/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Filter.h"

namespace Immortal
{

class GaussianBlurFilter : public FilterNode
{
public:
    GaussianBlurFilter(Device *device, float sigma, int kernalSize, float verticalSigma = 0, int verticalKernalSize = 0);

    void Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread()) override;

protected:
    void CalculateGaussianKernal(Device *device, Ref<Buffer> &buffer, float sigma, int size);

protected:
    Device *device;

    std::vector<Ref<DescriptorSet>> descriptorSets;

    Ref<Pipeline> pipelines[2];

    Ref<Buffer> kernal[2];

    std::vector<Ref<Texture>> temp;

    Ref<Buffer> stagingKernalBuffer[2];

    Ref<Sampler> sampler;

    int kernalSizes[2];
};

}
