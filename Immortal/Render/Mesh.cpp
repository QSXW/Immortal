#include "impch.h"
#include "Mesh.h"

#include "Render.h"
#include "Math/Math.h"
#include "FileSystem/FileSystem.h"

#if HAVE_ASSIMP
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#endif

namespace Immortal
{

#if HAVE_ASSIMP
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

    virtual void write(const char *message) override
    {
        LOG::INFO("Assimp: {0}", message);
    }
};
#endif

std::vector<std::shared_ptr<Mesh>> Mesh::Primitives;

static inline std::string ExtractModelFileWorkspace(const std::string &modelpath)
{
    std::filesystem::path path{ modelpath };
    return path.parent_path().string();
}

void Mesh::LoadPrimitives()
{

}

Mesh::Mesh(const std::string &filepath) :
    path{ filepath }
{
#if !HAVE_ASSIMP
    ThrowIf(false, "Assimp library not Found! Unable to import mesh from local file");
#else
    std::string workspace = ExtractModelFileWorkspace(path);

    LogStream::initialize();

    LOG::INFO("Loading mesh: {0}", filepath.c_str());
    std::unique_ptr<Assimp::Importer> importer{ new Assimp::Importer() };

    const aiScene *scene = importer->ReadFile(filepath, ImportFlags);
    SLASSERT(scene && scene->HasMeshes() && "Failed to load Mesh file: {0}" && filepath.c_str());

    {
        std::vector<Vertex> vertices;
        std::vector<Face> faces;

        uint32_t totalVertexSize = 0;
        uint32_t totalFaceSize = 0;
        std::vector<Buffer::BindInfo> bindInfos;

        bindInfos.resize(scene->mNumMeshes * 2);
        for (size_t i = 0; i < scene->mNumMeshes; i++)
        {
            bindInfos[i].size   = scene->mMeshes[i]->mNumVertices * sizeof(Vertex);
            bindInfos[i].offset = totalVertexSize;
            bindInfos[i].type   = Buffer::Type::Vertex;

            totalVertexSize += bindInfos[i].size;
        }
        for (size_t i = 0, j = scene->mNumMeshes; i < scene->mNumMeshes; i++, j++)
        {
            bindInfos[j].type = Buffer::Type::Index;
            bindInfos[j].size = scene->mMeshes[i]->mNumFaces * sizeof(Face);
            bindInfos[j].offset = bindInfos[j - 1].size + bindInfos[j - 1].offset;

            totalFaceSize += bindInfos[j].size;
        }
        vertices.reserve(totalVertexSize);
        faces.reserve(totalFaceSize);

        buffer.reset(Render::CreateBuffer(totalVertexSize + totalFaceSize, Buffer::Type{ Buffer::Type::Vertex | Buffer::Type::Index }));

        nodes.reserve(scene->mNumMeshes);
        for (size_t i = 0; i < scene->mNumMeshes; i++)
        {
            auto mesh = scene->mMeshes[i];
            Node node{ mesh->mName.C_Str() };
            node.MaterialIndex = i;

            THROWIF(!mesh->HasPositions() || !mesh->HasNormals(), "No Position and Normals in the mesh object");

            for (size_t i = 0; i < mesh->mNumVertices; i++)
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

            for (size_t i = 0; i < mesh->mNumFaces; i++)
            {
                Face face{
                    mesh->mFaces[i].mIndices[0],
                    mesh->mFaces[i].mIndices[1],
                    mesh->mFaces[i].mIndices[2]
                };
                faces.push_back(face);
            }

            node.Vertex.reset(buffer->Bind(bindInfos[i]));
            node.Index.reset(buffer->Bind(bindInfos[scene->mNumMeshes + i]));
            nodes.emplace_back(node);
        }
        buffer->Update(vertices);
        buffer->Update(faces, vertices.size() * sizeof(Vertex));
    }

    if (scene->HasMaterials())
    {
        for (size_t i = 0; i < scene->mNumMaterials; i++)
        {
            auto &material = scene->mMaterials[i];
            aiString texturePath;
            auto count = material->GetTextureCount(aiTextureType_BASE_COLOR);
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
            {

            }
        }
    }
#endif
}

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<Index> &indicies)
{
    Node head{ "Undefined" };

    head.Vertex.reset(Render::Create<Buffer>(vertices, Buffer::Type::Vertex));
    head.Index.reset(Render::Create<Buffer>(indicies, Buffer::Type::Index));

    nodes.emplace_back(head);
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
