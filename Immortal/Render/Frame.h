#pragma once
#include "Core.h"

#include "Texture.h"
#include "Format.h"
#include "Media/StillPicture.h"

namespace Immortal 
{

class IMMORTAL_API Frame
{
public:
    Frame() = default;

    Frame(const std::string &path);

    Frame(uint32_t width, uint32_t height, int depth = 1, const void *data = nullptr);

    virtual ~Frame();

    virtual uint32_t Width() const
    { 
        return decoder->Desc().Width;
    }

    virtual uint32_t Height() const
    {
        return decoder->Desc().Height;
    }

    virtual Media::Description Desc() const
    { 
        return decoder->Desc();
    }

    virtual UINT8 *Data() const
    { 
        return decoder->Data();
    };

    virtual bool Available()
    {
        return !!decoder && !!decoder->Data();
    }

    virtual size_t Size() const
    {
        return decoder->Desc().Size;
    }

private:
    std::unique_ptr<Media::Decoder> decoder;
};
}
