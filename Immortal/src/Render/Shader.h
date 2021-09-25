#pragma once

#include "ImmortalCore.h"

namespace Immortal
{

class IMMORTAL_API Shader
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
        Vertex,
        TesselationControl,
        TesselationEvaluation,
        Geometry,
        Fragment,
        Compute,
        RayGen,
        AnyHit,
        ClosestHit,
        Miss,
        Intersection,
        Callable
    };

    enum class DataType
    {
        None = 0,
        Float,
        Float2,
        Float3,
        Float4,
        Mat3,
        Mat4,
        Int,
        Int2,
        Int3,
        Int4,
        Bool
    };

    enum class Type
    {
        Graphics,
        Compute
    };

public:
    virtual ~Shader() = default;

    virtual void Map() const { }
    virtual void Unmap() const { }

    virtual const char *Name() const
    {
        return "Un-Specified";
    }

    virtual const uint32_t Handle() const
    {
        return -1;
    }

    virtual void Set(const std::string& name, int value) { }
    virtual void Set(const std::string& name, int* values, uint32_t count) { }
    virtual void Set(const std::string& name, float value) { }
    virtual void Set(const std::string& name, const Vector2 &value) { }
    virtual void Set(const std::string& name, const Vector3 &value) { }
    virtual void Set(const std::string& name, const Vector4 &value) { }
    virtual void Set(const std::string& name, const Matrix4 &value) { }

    virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) { }

    static Ref<Shader> Shader::Create(const std::string &filepath);

    static Ref<Shader> Shader::Create(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);
};

using SuperShader = Shader;

using ShaderDataType = Shader::DataType;

class IMMORTAL_API ShaderMap
{
public:
    void Add(const std::string &name, const Ref<Shader> &shader);
    void Add(const Ref<Shader> &shader);
        
    Ref<Shader> Load(const std::string &filepath);
    Ref<Shader> Load(const std::string &name, const std::string &filepath);
        
    Ref<Shader> Get(const std::string &name);
    Ref<Shader> At(const std::string &name)
    {
        return this->Get(name);
    }

    bool Exists(const std::string &name) const;

private:
    std::unordered_map<std::string, Ref<Shader> > shaders;
};

}
