/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Filter.h"

namespace Immortal
{

class TransferNode : public FilterNode
{
public:
	SL_ENABLE_COPY(TransferNode)
    
    enum class Type
    {
        Upload,
        Download
    };

public:
	TransferNode();

    virtual ~TransferNode() override;

    TransferNode(const TransferNode &other);
    
	void Upload(const Picture &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

public:
    void Swap(TransferNode &other)
    {
		FilterNode::Swap(other);
		buffer.Swap(other.buffer);
    }

public:
	Ref<Buffer> buffer;
};

}
