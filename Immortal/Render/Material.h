#pragma once

#include "Core.h"

#include "Texture.h"
#include "Shader.h"

namespace Immortal
{

class IMMORTAL_API Material
{
public:
    enum class Flag
    {
        None      = BIT(0),
        DepthTest = BIT(1),
        Blend     = BIT(2),
        TwoSided  = BIT(3)
    };

    struct MaterialUniforms
    {
        Vector3  AlbedoColor;
        float            Metalness;
        float            Roughness;
    };

public:
    virtual ~Material() { }

    virtual void Set(const std::string &name, float  value)    = 0;
    virtual void Set(const std::string &name, INT32  value)      = 0;
    virtual void Set(const std::string &name, UINT32 value) = 0;
    virtual void Set(const std::string &name, bool   value)     = 0;
    virtual void Set(const std::string &name, const Vector2 &value) = 0;
    virtual void Set(const std::string &name, const Vector3 &value) = 0;
    virtual void Set(const std::string &name, const Vector4 &value) = 0;
    virtual void Set(const std::string &name, const Matrix3 &value) = 0;
    virtual void Set(const std::string &name, const Matrix4 &value) = 0;

    virtual void Set(const std::string &name, const std::shared_ptr<Texture> &texture)   = 0;
    virtual void Set(const std::string &name, const std::shared_ptr<TextureCube> &texture) = 0;

    virtual std::shared_ptr<Texture> texture(const std::string &name) = 0;
    virtual std::shared_ptr<TextureCube> textureCube(const std::string &name) = 0;

    virtual UINT32 Flags() const = 0;

    virtual std::shared_ptr<Shader> shader() = 0;
    virtual const std::string &Name() const = 0;
};

}
