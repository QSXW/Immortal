#pragma once

#include "Core.h"
#include "Types.h"
#include "Format.h"
#include "Shared/IObject.h"

namespace Immortal
{

class IMMORTAL_API Texture : public IObject
{
public:
    template<class T>
    requires std::is_integral_v<T>
    static constexpr T CalculateMipmapLevels(T width, T height)
    {
        T levels = 1;
        while ((width >> levels) && (height >> levels)) {
            levels++;
        }
        return levels;
    }

public:
	Texture();

    /**
     * @brief When the texture is used by command buffer execution, it should not be released.
     */
    virtual ~Texture() = default;

    const Format &GetFormat() const;

    const uint32_t &GetWidth() const;

    const uint32_t &GetHeight() const;

    const uint16_t &GetMipLevels() const;

    const uint16_t &GetArrayLayers() const;

    uint32_t GetRatio() const;

protected:
	void SetMeta(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers);

    void Swap(Texture &other)
    {
		std::swap(_format,      other._format     );
        std::swap(_width,       other._width      );
        std::swap(_height,      other._height     );
        std::swap(_mipLevels,   other._mipLevels  );
        std::swap(_arrayLayers, other._arrayLayers);
    }

protected:
	Format _format;

	uint32_t _width;

	uint32_t _height;

	uint16_t _mipLevels;

	uint16_t _arrayLayers;
};

using Image = Texture;
using SuperTexture = Texture;

namespace Interface
{
    using Texture = SuperTexture;
}

}
