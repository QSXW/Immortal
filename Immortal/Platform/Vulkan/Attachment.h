#include "Image.h"
#include "ImageView.h"

namespace Immortal
{
namespace Vulkan
{

struct Attachment
{
    Attachment() = default;

    Attachment(std::unique_ptr<Image> &&image,  std::unique_ptr<ImageView> &&view) :
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
        image.swap(other.image);
        view.swap(other.view);

        return *this;
    }

    std::unique_ptr<Image> image;

    std::unique_ptr<ImageView> view;
};

}
}
