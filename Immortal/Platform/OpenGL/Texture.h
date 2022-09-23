#pragma once

#include "Render/Texture.h"
#include "Common.h"
#include "Descriptor.h"

namespace Immortal
{
namespace OpenGL
{

class Texture : public SuperTexture
{
public:
	struct DataType
	{
		GL_FORMAT SizedFormat;
		GL_FORMAT BaseFormat;
		GL_FORMAT BinaryType;
		GLFilter  Filter;
		GLSampler Wrap;
	};

    using Super = SuperTexture;
	GLCPP_OPERATOR_HANDLE()

public:
    Texture(const std::string &path, const Description &description);

    Texture(const uint32_t width, const uint32_t height, const void *data, const Texture::Description &description);

    virtual ~Texture();

    virtual operator uint64_t() const override;

    virtual void Update(const void *data, uint32_t pitchX = 0) override;

    virtual void Blit() override;

    virtual bool operator==(const Super &super) const override;

    virtual void As(DescriptorBuffer *descriptors, size_t index) override;

    uint32_t MipLevels() const override
    {
		return mipLevels;
    }

protected:
    DataType type;
};

Texture::DataType NativeTypeToOpenGl(const Texture::Description &desc);

}
}
