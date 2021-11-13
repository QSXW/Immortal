#pragma once
#include "ImmortalCore.h"

#include "Texture.h"
#include <opencv2/core/core.hpp> 

#include "StillPicture.h"
#include "Format.h"

namespace Immortal 
{

class IMMORTAL_API Frame
{
public:
    Frame() = default;

    Frame(const std::string &path, int channels = 0, Format format = Format::None);

    Frame(const std::string &path, bool flip);

    Frame(uint32_t width, uint32_t height, int depth = 1, const void *data = nullptr);

    virtual ~Frame();

    virtual uint32_t Width() const
    { 
        return width;
    }

    virtual uint32_t Height() const
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

    virtual bool Available()
    {
        return !!data.get();
    }

    virtual size_t Size() const
    {
        return size;
    }

private:
    void ReadByOpenCV(const std::string &path);

    void ReadByOpenCV(const std::string &path, bool flip);

    void ReadByInternal(const std::string &path);

    bool Read(const std::string &path, cv::Mat &outputMat);

private:
    Texture::Description desc;

    uint32_t  width{ 0 };

    uint32_t height{ 0 };

    size_t spatial{ 0 };

    size_t size{ 0 };

    int depth{ 1 };

    std::unique_ptr<uint8_t>  data{ nullptr };

public:
    static inline std::shared_ptr<Frame> Create(uint32_t width, uint32_t height, int depth = 1, const void *data = nullptr)
    {
        return std::make_shared<Frame>(width, height, depth, data);
    }

    static inline std::shared_ptr<Frame> Create(const std::string &filepath)
    {
        return std::make_shared<Frame>(filepath);
    }
};
}
