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

inline void InterpolateQuanternion(Quaternion &pOut, const Quaternion &pStart, const Quaternion &pEnd, float pFactor)
{
    using TReal = float;

    // calc cosine theta
    TReal cosom = pStart.x * pEnd.x + pStart.y * pEnd.y + pStart.z * pEnd.z + pStart.w * pEnd.w;

    // adjust signs (if necessary)
    Quaternion end = pEnd;
    if (cosom < static_cast<TReal>(0.0))
    {
        cosom = -cosom;
        end.x = -end.x;   // Reverse all signs
        end.y = -end.y;
        end.z = -end.z;
        end.w = -end.w;
    }

    // Calculate coefficients
    TReal sclp, sclq;
    if ((static_cast<TReal>(1.0) - cosom) > static_cast<TReal>(0.0001)) // 0.0001 -> some epsillon
    {
        // Standard case (slerp)
        TReal omega, sinom;
        omega = std::acos(cosom); // extract theta from dot product's cos theta
        sinom = std::sin(omega);
        sclp = std::sin((static_cast<TReal>(1.0) - pFactor) * omega) / sinom;
        sclq = std::sin(pFactor * omega) / sinom;
    }
    else
    {
        // Very close, do linear interp (because it's faster)
        sclp = static_cast<TReal>(1.0) - pFactor;
        sclq = pFactor;
    }

    pOut.x = sclp * pStart.x + sclq * end.x;
    pOut.y = sclp * pStart.y + sclq * end.y;
    pOut.z = sclp * pStart.z + sclq * end.z;
    pOut.w = sclp * pStart.w + sclq * end.w;
}

inline void InterpolateVector3(Vector3 &pOut, const Vector3 &pStart, const Vector3 &pEnd, float pFactor)
{
    float sclp = 1.0f - pFactor;
    float sclq = pFactor;

    pOut = sclp * pStart + sclq * pEnd;
}

template <class T, class U>
inline constexpr T Interpolate(const LightVector<U> &keys, float animationTime)
{
    T ret{};

    if (keys.Size() == 1)
    {
        return keys[0].Value;
    }

    size_t current = 0;
    size_t size = keys.Size() - 1;
    for (; current < size; current++)
    {
        if (animationTime < keys[current + 1].Time)
        {
            break;
        }
    }

    if (current == size)
    {
        return keys[current].Value;
    }
    size_t next = current + 1;

    float deltaTime = keys[next].Time - keys[current].Time;
    float factor = std::abs((animationTime - (float)keys[current].Time) / deltaTime);

    SLASSERT(factor >= 0.0f && factor <= 1.0f);
    const T &start = keys[current].Value;
    const T &end   = keys[next].Value;

    if constexpr (IsPrimitiveOf<QuanternionKey, U>())
    {
        InterpolateQuanternion(ret, start, end, factor);
        return T{ Vector::Normalize(ret) };
    }
    else
    {
        InterpolateVector3(ret, start, end, factor);
        return ret;
    }
}

void SkeletonVertex::AddBone(uint32_t id, float weight)
{
    for (size_t i = 0; i < SL_ARRAY_LENGTH(BoneIds); i++)
    {
        if (Weights[i] == 0.0)
        {
            BoneIds[i] = id;
            Weights[i] = weight;
            return;
        }
    }
}

#if HAVE_ASSIMP
static constexpr uint32_t ImportFlags =
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate |
    aiProcess_SortByPType |
    aiProcess_GenNormals |
    aiProcess_GenUVCoords |
    aiProcess_ValidateDataStructure;

static inline Matrix4 AssimpMatrix4x4ToNative(const aiMatrix4x4 &m)
{
    return Matrix4{
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4
    };
}

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

    LoadModelData(scene);

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

    head.Vertex = Render::Create<Buffer>(vertices, Buffer::Type::Vertex);
    head.Index  = Render::Create<Buffer>(indicies, Buffer::Type::Index);

    nodes.emplace_back(head);
}

void Mesh::ReadHierarchyBoneNode(float animationTime, const BoneNode *node, const Matrix4 &parentTransform)
{
    Matrix4 globalTransform{};
    Matrix4 nodeTransform = node->Transform;

    auto it = animationNodes.find(node->Name);
    if (it != animationNodes.end())
    {
        AnimationNode &node = it->second;
        Vector3 position    = Interpolate<Vector3, VectorKey>(node.PositionKeys, animationTime);
        Vector3 scaling     = Interpolate<Vector3, VectorKey>(node.ScalingKeys, animationTime);
        Quaternion rotation = Interpolate<Quaternion, QuanternionKey>(node.RotationKeys, animationTime);

        nodeTransform = Vector::Translate(position) * Vector::ToMatrix4(rotation) * Vector::Scale(scaling);
    }

    globalTransform  = parentTransform * nodeTransform;
    const auto &boneIt = bones.find(node->Name);
    if (boneIt != bones.end())
    {
        auto &boneInfo = boneIt->second;
        transforms[boneInfo.Id] = globalInverseTransform * globalTransform * boneInfo.OffsetMatrix;
    }
    else
    {
        if (!node->Meshes.Empty())
        {
            transforms[node->Meshes[0]] = globalInverseTransform * globalTransform;
        }
    }

    auto &children = node->Children;
    for (auto &child : children)
    {
        ReadHierarchyBoneNode(animationTime, &child, globalTransform);
    }
}

void Mesh::CalculatedBoneTransform(float timeInSeconds, const Matrix4 &parentTransform)
{
    transforms[0] = parentTransform;

    float animationTime = 0.0f;
    if (IsAnimated())
    {
        float timeInTicks = timeInSeconds * animations[state.currentAnimation].TicksPerSeconds;
        animationTime = fmod(timeInTicks, animations[state.currentAnimation].Duration);
    } 

    ReadHierarchyBoneNode(animationTime, rootNode, parentTransform);
    transformBuffer->Update(transforms);
}

#if HAVE_ASSIMP
void Mesh::LoadModelData(const aiScene *scene)
{
    std::vector<SkeletonVertex> vertices;
    std::vector<Face> faces;

    uint32_t totalVertices = 0;
    uint32_t totalFaces = 0;
    uint32_t numBones = 1;

    Buffer::BindInfo vertexBindInfo{ Buffer::Type::Vertex, 0, 0 };
    Buffer::BindInfo faceBindInfo{ Buffer::Type::Index, 0, 0 };

    for (size_t i = 0; i < scene->mNumMeshes; i++)
    {
        totalVertices += scene->mMeshes[i]->mNumVertices;
        totalFaces += scene->mMeshes[i]->mNumFaces;
    }

    vertices.reserve(totalVertices);
    faces.reserve(totalFaces);

    faceBindInfo.offset = totalVertices * sizeof(SkeletonVertex);

    buffer = Render::CreateBuffer(faceBindInfo.offset + totalFaces * sizeof(Face), Buffer::Type{Buffer::Type::Vertex | Buffer::Type::Index});

    nodes.resize(scene->mNumMeshes);
    for (size_t i = 0; i < scene->mNumMeshes; i++)
    {
        auto mesh = scene->mMeshes[i];
        auto &node = nodes[i];
        node.Name = mesh->mName.C_Str();
        node.MaterialIndex = mesh->mMaterialIndex;

        THROWIF(!mesh->HasPositions() || !mesh->HasNormals(), "No Position or Normals in the mesh object");

        for (size_t j = 0; j < mesh->mNumVertices; j++)
        {
            auto &vertex = vertices.emplace_back();
            vertex.Position = { mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z };
            vertex.Normal = { mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z };

            if (mesh->HasTangentsAndBitangents())
            {
                vertex.Tangent = { mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z };
            }
            if (mesh->HasTextureCoords(0))
            {
                vertex.Texcoord = { mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y };
            }
        }
        vertexBindInfo.size = mesh->mNumVertices * sizeof(SkeletonVertex);

        uint32_t baseVertex = vertexBindInfo.offset / sizeof(SkeletonVertex);
        bool hasBone = LoadBoneData(mesh, vertices, baseVertex, numBones);

        if (!hasBone && scene->HasAnimations())
        {
            for (size_t j = baseVertex; j < vertices.size(); j++)
            {
                vertices[j].BoneIds[0] = i;
                vertices[j].Weights[0] = 1.0f;
            }
        }

        for (size_t j = 0; j < mesh->mNumFaces; j++)
        {
            auto &face = faces.emplace_back();
            face.v1 = mesh->mFaces[j].mIndices[0];
            face.v2 = mesh->mFaces[j].mIndices[1];
            face.v3 = mesh->mFaces[j].mIndices[2];
        }
        faceBindInfo.size = mesh->mNumFaces * sizeof(Face);

        node.Vertex = buffer->Bind(vertexBindInfo);
        node.Index  = buffer->Bind(faceBindInfo);

        vertexBindInfo.Increase();
        faceBindInfo.Increase();
    }

    LoadAnimationData(scene);

    transforms.resize(bones.empty() ? scene->mNumMeshes : bones.size() + 1);
    transformBuffer = Render::CreateBuffer(transforms.size() * sizeof(Matrix4), Buffer::Type{ Buffer::Type::Uniform });

    rootNode = new BoneNode{};
    ReadAssimpNode(rootNode, scene->mRootNode);
    globalInverseTransform = Vector::Inverse(rootNode->Transform);

    buffer->Update(vertices);
    buffer->Update(faces, vertices.size() * sizeof(SkeletonVertex));
}

bool Mesh::LoadBoneData(const aiMesh *mesh, std::vector<SkeletonVertex> &vertices, uint32_t baseVertex, uint32_t &numBones)
{
    if (!mesh->mNumBones)
    {
        return false;
    }

    for (size_t i = 0; i < mesh->mNumBones; i++)
    {
        std::string name = mesh->mBones[i]->mName.C_Str();
        LOG::DEBUG("Mesh::LoadBoneDataRead::{}", name);
        
        if (bones.find(name) == bones.end())
        {
            bones.insert({ name, { numBones++, AssimpMatrix4x4ToNative(mesh->mBones[i]->mOffsetMatrix) } });
        }
        
        auto &boneInfo = bones.find(name)->second;
        for (size_t j = 0; j < mesh->mBones[i]->mNumWeights; j++)
        {
            auto &pWeight = mesh->mBones[i]->mWeights[j];
            vertices[baseVertex + pWeight.mVertexId].AddBone(boneInfo.Id, pWeight.mWeight);
        }
    }

    return true;
}

void Mesh::ReadAssimpNode(BoneNode *node, const aiNode *src)
{
    node->Name = src->mName.C_Str();
    LOG::DEBUG("Mesh::ReadAssimpNode::{}", node->Name);
    node->Transform = AssimpMatrix4x4ToNative(src->mTransformation);

    node->Meshes.Resize(src->mNumMeshes);
    for (size_t i = 0; i < src->mNumMeshes; i++)
    {
        node->Meshes[i] = src->mMeshes[i];
    }

    node->Children.Resize(src->mNumChildren);
    for (size_t i = 0; i < src->mNumChildren; i++)
    {
        ReadAssimpNode(&node->Children[i], src->mChildren[i]);
        node->Children[i].Parent = node;
    }
}

template <class T, class U>
static void CopyAssimpAnimationKey(LightVector<T> &dst, const U *src, uint32_t size)
{
    dst.Resize(size);
    for (size_t i = 0; i < size; i++)
    {
        if constexpr (IsPrimitiveOf<QuanternionKey, T>())
        {
            dst[i].Value.w = src[i].mValue.w;
        }

        dst[i].Time = src[i].mTime;
        dst[i].Value.x = src[i].mValue.x;
        dst[i].Value.y = src[i].mValue.y;
        dst[i].Value.z = src[i].mValue.z;
    }
}

void Mesh::LoadAnimationData(const aiScene *scene)
{
    animations.resize(scene->mNumAnimations);
    for (size_t i = 0; i < animations.size(); i++)
    {
        auto pAnimation = scene->mAnimations[i];
        animations[i].Name            = pAnimation->mName.C_Str();
        animations[i].TicksPerSeconds = pAnimation->mTicksPerSecond;
        animations[i].Duration        = std::floorf(pAnimation->mDuration);

        for (size_t j = 0; j < pAnimation->mNumChannels; j++)
        {
            auto pChannel = pAnimation->mChannels[j];
            AnimationNode node{};
            node.PreState  = (AnimationBehavior)pChannel->mPreState;
            node.PostState = (AnimationBehavior)pChannel->mPostState;
            CopyAssimpAnimationKey(node.PositionKeys, pChannel->mPositionKeys, pChannel->mNumPositionKeys);
            CopyAssimpAnimationKey(node.RotationKeys, pChannel->mRotationKeys, pChannel->mNumRotationKeys);
            CopyAssimpAnimationKey(node.ScalingKeys,  pChannel->mScalingKeys,  pChannel->mNumScalingKeys );
            animationNodes.insert({ pChannel->mNodeName.C_Str(), std::move(node)});
        }
    }
}
#endif

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

void Mesh::SwitchToAnimation(uint32_t index)
{
    state.currentAnimation = index;
}

uint32_t Mesh::GetAnimationState() const
{
    return state.currentAnimation;
}

}
