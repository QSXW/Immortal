# Immortal
Immortal is a powerful, advanced, and cross-platform graphics and media library. It's targeted to push forward by two parts, one is an object-oriented, and friendly graphics API that is used for real-time rendering, the other is a vision library consisting of native implemented media codec, image processing, and external codec integrated.

## Graphics API Support
| Graphics API | Progress |
| --- | ----------- |
| Vulkan | 100% |
| D3D11 | 100% |
| D3D12 | 100% |
| OpenGL | 100% |

 Select any graphics API you want, Vulkan, D3D12, or OpenGL. You just need to change the name literally.
 ``` C++
 #include "VulkanSample.h"

int main()
{
    LOG::Setup();
    Render::Set(Render::Type::Vulkan);

    std::unique_ptr<Application> app{ new VulkanSample() };

    app->Run();

    return 0;
}
 ```

## What you can do with Immortal?
#### Improve your image processing by rendering pipeline of Immortal.
![Color Editor](https://user-images.githubusercontent.com/47172719/158025476-0ab8a532-c193-4b27-8b87-f81ff7cb4a64.png)

#### Build a 3D model lighting preview application
![Model Editor](https://user-images.githubusercontent.com/47172719/158025865-1606e292-d871-4939-9717-e886ced3852c.png)
