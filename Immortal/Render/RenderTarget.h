#pragma once

#include "Core.h"

#include "Texture.h"
#include "Types.h"
#include "Interface/IObject.h"

namespace Immortal
{
  
class IMMORTAL_API RenderTarget : public IObject
{
public:
    RenderTarget() = default;

    virtual ~RenderTarget() = default;
         
    virtual void Resize(uint32_t width, uint32_t height) = 0;

    virtual Texture *GetColorAttachment(uint32_t index) = 0;
    
    virtual Texture *GetDepthAttachment() = 0;
};

using SuperRenderTarget = RenderTarget;

namespace Interface
{
    using RenderTarget = SuperRenderTarget;
}

}
