#pragma once

#include "Core.h"
#include "Config.h"

#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Math/Vector.h"

#include <vector>
#include <list>

namespace Immortal
{

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

    static std::shared_ptr<Mesh> CreateSphere(float radius);

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
            Index{ other.Index },
            MaterialIndex{ other.MaterialIndex }
        {

        }

        Node(Node &&other) :
            Name{ std::move(other.Name) },
            Vertex{ std::move(other.Vertex) },
            Index{ std::move(other.Index) },
            MaterialIndex{ std::move(other.MaterialIndex) }
        {

        }

        std::string Name;
        std::shared_ptr<Buffer> Vertex;
        std::shared_ptr<Buffer> Index;

        uint32_t MaterialIndex = 0;
    };

    using Index = Face;

public:
    Mesh(const std::string &filepath);

    Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indicies);

    ~Mesh() { }

    const std::string &Source() const
    {
        return path;
    }

    std::vector<Node> &NodeList()
    {
        return nodes;
    }

    size_t Size() const
    {
        return nodes.size();
    }

private:
    std::unique_ptr<Buffer> buffer;

    std::string path;

    std::vector<Node> nodes;

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
