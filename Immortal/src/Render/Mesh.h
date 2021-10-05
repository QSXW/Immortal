#pragma once

#include "ImmortalCore.h"
#include "Framework/Object.h"

#include "Buffer.h"
#include "VertexArray.h"
#include "Shader.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Immortal
{

#define IM_DEFAULT_MESH_PATH(x) "Assets/Meshes/Default/"#x".fbx"

class IMMORTAL_API Mesh : public Object
{
public:
    enum class Primitive
    {
        Capsule = 0,
        Cone,
        Cube,
        Cylinder,
        Plane,
        Sphere,
        Torus
    };

    static std::vector<std::shared_ptr<Mesh>> Primitives;

    template <Primitive I>
    static inline constexpr std::shared_ptr<Mesh> Get()
    {
        return Primitives[static_cast<UINT32>(I)];
    }

    static void LoadPrimitives();

    static std::shared_ptr<Mesh> Mesh::CreateSphere(float radius);

private:
    VertexLayout mLayout{
        { Shader::DataType::Float3, "position"  },
        { Shader::DataType::Float3, "normal"    },
        { Shader::DataType::Float3, "tagent"    },
        { Shader::DataType::Float3, "bitangent" },
        { Shader::DataType::Float2, "texcoord"  },
    };

public:
    struct Vertex
    {
        Vector::Vector3 Position;
        Vector::Vector3 Normal;
        Vector::Vector3 Tangent;
        Vector::Vector3 Bitangent;
        Vector::Vector2 Texcoord;
    };

    struct Face
    {
        UINT32 v1, v2, v3;
    };

    using Index = Face;

    struct SubMesh
    {
        UINT32 BaseVertex;
        UINT32 BaseIndex;
        UINT32 MaterialIndex;
        UINT32 IndexCount;
        UINT32 VertexCount;

        Vector::Matrix4 Transform{ 1.0f };
        std::string NodeName;
        std::string MeshName;
    };

    static constexpr UINT32 ImportFlags =
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_PreTransformVertices |
        aiProcess_GenNormals |
        aiProcess_GenUVCoords |
        aiProcess_OptimizeMeshes |
        aiProcess_Debone |
        aiProcess_ValidateDataStructure;

    struct LogStream : public Assimp::LogStream
    {
        static void initialize()
        {
            if (Assimp::DefaultLogger::isNullLogger()) {
                Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
                Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
            }
        }

        void write(const char* message) override
        {
            std::fprintf(stderr, "Assimp: %s", message);
        }
    };

public:
    Mesh(const std::string &filepath);
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indicies);
    ~Mesh() { }

    std::shared_ptr<VertexArray> VertexArrayObject() { return mVertexArray; }

    const char *Path() const noexcept
    {
        return mPath.c_str();
    }

private:
    std::unique_ptr<Assimp::Importer> mImporter{ nullptr };

    std::string mPath;

    std::shared_ptr<Shader> mShader;

    std::vector<Vertex> mVertices;

    std::vector<Face> mFaces;

    std::shared_ptr<VertexBuffer> mVertexBuffer;

    std::shared_ptr<IndexBuffer>  mIndexBuffer;

    std::shared_ptr<VertexArray>  mVertexArray;

    float mAnimationTime    = 0.0f;
    float mWorldTime        = 0.0f;
    float mTimeMultiplier   = 1.0f;
    bool  mAnimated         = false;
    bool  mAnimationPlaying = true;
};

}
