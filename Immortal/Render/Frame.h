#pragma once
#include "Core.h"

#include "Texture.h"
#include "Format.h"
#include "Vision/Interface/Codec.h"

namespace Immortal 
{

class IMMORTAL_API Frame
{
public:
    Frame() = default;

    Frame(const std::string &path);

    Frame(uint32_t width, uint32_t height, int depth = 1, const void *data = nullptr);

    virtual ~Frame();

    virtual UINT8 *Data() const
    { 
        return decoder->Data();
    };

    virtual bool Available()
    {
        return !!decoder && !!decoder->Data();
    }

    virtual Vision::Picture GetPicture() const
    {
        return decoder->GetPicture();
    }

private:
    MonoRef<Vision::Interface::Codec> decoder;
};

}
