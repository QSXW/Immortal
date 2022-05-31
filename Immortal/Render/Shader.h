#pragma once

#include "Core.h"
#include "Format.h"
#include "Buffer.h"

namespace Immortal
{

class IMMORTAL_API Shader : public IObject
{
public:
    enum class API
    {
        OpenGL,
        Vulkan,
        D3D12
    };

    enum class Stage
    {
        Vertex                = BIT(0),
        TesselationControl    = BIT(1),
        TesselationEvaluation = BIT(2),
        Geometry              = BIT(3),
        Fragment              = BIT(4),
        Compute               = BIT(5),
        RayGen                = BIT(6),
        AnyHit                = BIT(7),
        ClosestHit            = BIT(8),
        Miss                  = BIT(9),
        Intersection          = BIT(10),
        Callable              = BIT(11),
        Pixel = Fragment,
    };

    enum class Type
    {
        Graphics,
        Compute
    };

    struct Resource
    {
        enum class Type : uint32_t
        {
            Uniform       = BIT(1),
            Storage       = BIT(2),
            PushConstant  = BIT(3),
            Image         = BIT(4),
            ImageSampler  = BIT(5),
            ImageStorage  = BIT(6),
            Sampler       = BIT(7),
            Input         = BIT(8),
            Output        = BIT(9),
            None          = 0
        };

        enum class Mode
        {
            Static,
            Dynamic,
            UpdateAfterBind
        };

        enum class Qualifier
        {
            None,
            ReadOnly,
            WriteOnly
        };

        Type type;

        Format format;

        uint8_t set;

        uint8_t binding;

        uint8_t location;

        uint8_t count;

        uint32_t size;

        uint32_t constantID;

        std::string name;
    };

    struct Properties
    {
        std::string Name;

        uint32_t API;

        Shader::Type Type;
    };

    using Manager = std::map<std::string, Ref<Shader>>;

public:
    static Resource::Type GetResourceType(const std::string &key)
    {
        static std::map<std::string, Resource::Type> map = {
            { "uniform",   Resource::Type::Uniform      },
            { "sampler2D", Resource::Type::ImageSampler },
            { "texture2D", Resource::Type::Image        },
            { "in",        Resource::Type::Input        },
            { "out",       Resource::Type::Output       }
        };
        const auto &it = map.find(key);

        if (it == map.end())
        {
            return Resource::Type::None;
        }
        return it->second;
    }

    static Format GetResourceFormat(const std::string &key)
    {
        static std::map<std::string, Format> map = {
            { "int",   Format::INT     },
            { "float", Format::FLOAT   },
            { "vec2",  Format::VECTOR2 },
            { "vec3",  Format::VECTOR3 },
            { "vec4",  Format::VECTOR4 },
            { "mat4",  Format::MATRIX4 }
        };

        const auto &it = map.find(key);

        if (it == map.end())
        {
            return Format::None;
        }
        return it->second;
    }

public:
    Shader() :
        type{ Type::Graphics }
    {

    }

    Shader(Type type) :
        type{ type }
    {

    }

    virtual ~Shader() = default;

    virtual void Map() const { }
    virtual void Unmap() const { }

    virtual const char *Name() const
    {
        return "Un-Specified";
    }

    virtual const std::string &GetEntryPoint() const
    {
        return entryPoint;
    }

    virtual const uint32_t Handle() const
    {
        return -1;
    }

    virtual const bool IsGraphics() const
    {
        return type == Type::Graphics;
    }

    bool IsType(Type other) const
    {
        return type == other;
    }

protected:
    Type type{ Type::Graphics };

    std::string entryPoint = "main";
};

using SuperShader = Shader;

SL_DEFINE_BITWISE_OPERATION(Shader::Resource::Type, uint32_t)

}
