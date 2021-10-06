#pragma once
#include "ImmortalCore.h"

#include "Texture.h"
#include "Image/ColorSpace.h"
#include <opencv2/core/core.hpp> 

namespace Immortal 
{
class IMMORTAL_API Frame
{
public:
    enum class Format : int
    {
        None = 0,
        RGB,
        RGBA,
        RGBA16F,
        RGBA32F,
        RG32F,
        SRGB,
        DEPTH32F,
        Depth24Stencil8,
        Depth = Depth24Stencil8
    };

    static inline UINT32 map[] = {
            0,
            3,
            4,
            8,
        16,
            8,
            3
    };

public:
    Frame() = default;

    Frame(const std::string &path, int channels = 0, Texture::Format format = Texture::Format::None);

    Frame(const std::string &path, bool flip);

    Frame(UINT32 width, UINT32 height, int depth = 1, const void *data = nullptr, ColorSpace colorSpace = ColorSpace::RGB);

    virtual ~Frame();

    virtual UINT32 Width() const
    { 
        return width;
    }

    virtual UINT32 Height() const
    { 
        return height;
    }

    virtual Texture::Description Type() const
    { 
        return desc;
    }

    virtual UINT8 *Data() const
    { 
        return data.get();
    };

    virtual UINT32 Hash() const { return 0xff; };

private:
    void Frame::Read(const std::string &path, cv::Mat &outputMat);

private:
    ColorSpace colorSpace;

    Texture::Description desc;

    UINT32  width{ 0 };

    UINT32 height{ 0 };

    size_t spatial{ 0 };

    size_t size{ 0 };

    int depth{ 1 };

    std::unique_ptr<uint8_t>  data{ nullptr };

public:
    static inline std::shared_ptr<Frame> Create(UINT32 width, UINT32 height, int depth = 1, const void *data = nullptr, ColorSpace colorSpace = ColorSpace::RGB)
    {
        return std::make_shared<Frame>(width, height, depth, data, colorSpace);
    }

    static inline std::shared_ptr<Frame> Create(const std::string &filepath)
    {
        return std::make_shared<Frame>(filepath);
    }

    static inline UINT32 FormatBitsPerPixel(int format)
    {
        SLASSERT(format > SL_ARRAY_LEN(map) && "Unsupport format.");
        return map[format];
    }
};
}
