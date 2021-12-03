#pragma once

#include "ImmortalCore.h"
#include "Framework/Object.h"

#include "Buffer.h"
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
        return Primitives[static_cast<uint32_t>(I)];
    }

    static void LoadPrimitives();

    static std::shared_ptr<Mesh> Mesh::CreateSphere(float radius);

private:
    VertexLayout mLayout {
        { Format::VECTOR3, "position"  },
        { Format::VECTOR3, "normal"    },
        { Format::VECTOR3, "tagent"    },
        { Format::VECTOR3, "bitangent" },
        { Format::VECTOR2, "texcoord"  },
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
        uint32_t v1, v2, v3;
    };

    using Index = Face;

    struct SubMesh
    {
        uint32_t BaseVertex;
        uint32_t BaseIndex;
        uint32_t MaterialIndex;
        uint32_t IndexCount;
        uint32_t VertexCount;

        Vector::Matrix4 Transform{ 1.0f };
        std::string NodeName;
        std::string MeshName;
    };

    static constexpr uint32_t ImportFlags =
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

        void write(const char *message) override
        {
            std::fprintf(stderr, "Assimp: %s", message);
        }
    };

public:
    Mesh(const std::string &filepath);

    Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indicies);

    ~Mesh() { }

    const std::string &Path() const
    {
        return path;
    }

private:
    std::unique_ptr<Assimp::Importer> importer{ nullptr };

    std::string path;

    std::vector<Vertex> vertices;

    std::vector<Face> faces;

    std::shared_ptr<Buffer>vertexBuffer;

    std::shared_ptr<Buffer> indexBuffer;

    struct
    {
        float time = 0.0f;

        float worldTime = 0.0f;

        float timeMultiplier = 1.0f;

        bool animated = false;

        bool playing = true;
    } animation;

};

}
