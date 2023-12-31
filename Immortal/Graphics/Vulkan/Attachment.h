#pragma once

#include "Image.h"
#include "ImageView.h"

namespace Immortal
{
namespace Vulkan
{

class Attachment
{
public:
    Attachment() :
        image{},
        view{}
    {

    }

    Attachment(Image &&image, ImageView &&view) :
        image{ std::move(image) },
        view{ std::move(view) }
    {

    }

    Attachment(Attachment &&other) :
        image{ std::move(other.image) },
        view{ std::move(other.view) }
    {

    }

    Attachment &operator=(Attachment &&other)
    {
        image = std::move(other.image);
        view  = std::move(other.view);
        return *this;
    }

    Attachment(const Attachment &) = delete;
    Attachment &operator =(const Attachment &) = delete;

    Image image;

    ImageView view;
};

}
}
