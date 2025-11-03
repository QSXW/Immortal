#pragma once

#include "Core.h"
#include "Format.h"
#include "Shared/IObject.h"
#include "Types.h"

namespace Immortal
{

class BufferView : public IObject
{
public:
    virtual ~BufferView() = default;
};

using SuperBufferView = BufferView;

}
