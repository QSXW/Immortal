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

    virtual const bool IsGraphics() const
    {
        return type == Type::Graphics;
    }

    virtual void Set(const std::string& name, int value) { }
    virtual void Set(const std::string& name, int* values, uint32_t count) { }
    virtual void Set(const std::string& name, float value) { }
    virtual void Set(const std::string& name, const Vector2 &value) { }
    virtual void Set(const std::string& name, const Vector3 &value) { }
    virtual void Set(const std::string& name, const Vector4 &value) { }
    virtual void Set(const std::string& name, const Matrix4 &value) { }

    virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) { }

    static std::shared_ptr<Shader> Shader::Create(const std::string &filepath);

    static std::shared_ptr<Shader> Shader::Create(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);

protected:
    Type type{ Type::Graphics };
};

using SuperShader = Shader;

class IMMORTAL_API ShaderMap
{
public:
    void Add(const std::string &name, const std::shared_ptr<Shader> &shader);

    void Add(const std::shared_ptr<Shader> &shader);
        
    std::shared_ptr<Shader> Load(const std::string &filepath);

    std::shared_ptr<Shader> Load(const std::string &name, const std::string &filepath);
        
    std::shared_ptr<Shader> Get(const std::string &name);

    std::shared_ptr<Shader> At(const std::string &name)
    {
        return this->Get(name);
    }

    bool Exists(const std::string &name) const;

private:
    std::unordered_map<std::string, std::shared_ptr<Shader> > shaders;
};

}
