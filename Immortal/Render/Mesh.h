#pragma once

#include "Core.h"

#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Immortal
{

#define IM_DEFAULT_MESH_PATH(x) "Assets/Meshes/Default/"#x".fbx"

class IMMORTAL_API Mesh
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
        Vector3 Position;
        Vector3 Normal;
        Vector3 Tangent;
        Vector3 Bitangent;
        Vector2 Texcoord;
    };

    struct Face
    {
        uint32_t v1, v2, v3;
    };

    struct Node
    {
        Node()
        {

        }

        Node(const std::string &name) :
            Name{ name }
        {

        }

        Node(const char *name) :
            Name{ name }
        {

        }

        Node(const Node &other) :
            Name{ other.Name },
            Vertex{ other.Vertex },
            Index{ other.Index }
        {

        }

        Node(Node &&other) :
            Name{ std::move(other.Name) },
            Vertex{ std::move(other.Vertex) },
            Index{ std::move(other.Index) }
        {

        }

        std::string Name;
        std::shared_ptr<Buffer> Vertex;
        std::shared_ptr<Buffer> Index;
    };

    using Index = Face;

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

    template <Buffer::Type T>
    std::shared_ptr<Buffer> Get()
    {
        if constexpr (T == Buffer::Type::Vertex)
        {
            return buffer.vertex;
        }
        if constexpr (T == Buffer::Type::Index)
        {
            return buffer.index;
        }
    }

    std::list<Node> &NodeList()
    {
        return nodes;
    }

private:
    std::unique_ptr<Assimp::Importer> importer{ nullptr };

    std::string path;

    std::list<Node> nodes;

    struct
    {
        float time = 0.0f;

        float worldTime = 0.0f;

        float timeMultiplier = 1.0f;

        bool animated = false;

        bool playing = true;
    } animation;

public:
    struct {
        std::shared_ptr<Texture> Albedo;
    } Textures[8];
};

}
