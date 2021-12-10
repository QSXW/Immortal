#include "impch.h"
#include "Mesh.h"

#include "Render.h"
#include "Math.h"

#include "Framework/Math.h"

namespace Immortal
{

std::vector<std::shared_ptr<Mesh>> Mesh::Primitives;

void Mesh::LoadPrimitives()
{
    constexpr const char *pathes[] = {
        IM_DEFAULT_MESH_PATH(Capsule),
        IM_DEFAULT_MESH_PATH(Cone),
        IM_DEFAULT_MESH_PATH(Cube),
        IM_DEFAULT_MESH_PATH(Cylinder),
        IM_DEFAULT_MESH_PATH(Plane),
        IM_DEFAULT_MESH_PATH(Sphere),
        IM_DEFAULT_MESH_PATH(Torus)
    };

    for (size_t i = 0; i < sizeof(pathes) / sizeof(pathes[0]); i++)
    {
        LOG::INFO("Loading Mesh Primitive: {0}", pathes[i]);
        if (i == static_cast<size_t>(Mesh::Primitive::Sphere))
        {
            Primitives.emplace_back(CreateSphere(0.5f));
            continue;
        }
        Primitives.emplace_back(new Mesh(pathes[i]));
    }
}

Mesh::Mesh(const std::string & filepath)
    : path(filepath)
{
    LogStream::initialize();

    LOG::INFO("Loading mesh: {0}", filepath.c_str());

    importer.reset(new Assimp::Importer());

    const aiScene *scene = importer->ReadFile(filepath, ImportFlags);
    SLASSERT(scene && scene->HasMeshes() && "Failed to load Mesh file: {0}" && filepath.c_str());

    const aiMesh *mesh = scene->mMeshes[0];
    SLASSERT(mesh->HasPositions() && mesh->HasNormals() && "No Position and Normals in the mesh object");

    // Set the capacity to the number of vertices
    vertices.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;
        vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
            vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
        }
        if (mesh->HasTextureCoords(0))
        {
            vertex.Texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
        vertices.push_back(vertex);
    }

    faces.reserve(mesh->mNumFaces);
    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        SLASSERT(mesh->mFaces[i].mNumIndices == 3 && "Must have three indices per face.");
        Face face = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
        faces.push_back(face);
    }

    vertexBuffer = Render::Create<Buffer>(vertices.size(), vertices.data(), Buffer::Type::Vertex);
    
    indexBuffer = Render::Create<Buffer>(faces.size(), faces.data(), Buffer::Type::Index);

    vertices.clear();
    faces.clear();
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indicies)
{
    vertexBuffer = Render::Create<Buffer>(vertices.size(), vertices.data(), Buffer::Type::Vertex);

    indexBuffer = Render::Create<Buffer>(faces.size(), faces.data(), Buffer::Type::Index);
}

std::shared_ptr<Mesh> Mesh::CreateSphere(float radius)
{
    std::vector<Vertex> vertices;
    std::vector<Face> indices;

    constexpr float latitudeBands = 30;
    constexpr float longitudeBands = 30;

    for (float latitude = 0.0F; latitude <= latitudeBands; latitude++)
    {
        float theta = latitude * (float)Math::PI / latitudeBands;
        float sinTheta = Math::Sin(theta);
        float cosTheta = Math::Cos(theta);

        for (float longitude = 0.0F; longitude <= longitudeBands; longitude++)
        {
            float phi = longitude * 2 * Math::PI / longitudeBands;
            float sinPhi = Math::Sin(phi);
            float cosPhi = Math::Cos(phi);

            Vertex vertex;
            vertex.Normal = { cosPhi * sinTheta, cosTheta, sinPhi * sinTheta };
            vertex.Position = { radius * vertex.Normal.x, radius * vertex.Normal.y, radius * vertex.Normal.z };
            vertices.push_back(vertex);
        }
    }

    for (uint32_t latitude = 0; latitude < latitudeBands; latitude++)
    {
        for (uint32_t longitude = 0; longitude < longitudeBands; longitude++)
        {
            uint32_t first = (latitude * (longitudeBands + 1)) + longitude;
            uint32_t second = first + longitudeBands + 1;

            indices.push_back({ first, second, first + 1 });
            indices.push_back({ second, second + 1, first + 1 });
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}
}
