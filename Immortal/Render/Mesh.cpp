#include "Mesh.h"

#include "Graphics.h"
#include "Math/Math.h"
#include "FileSystem/FileSystem.h"
#include "MeshletGenerator.h"
#include "Scene/Component.h"

#if HAVE_ASSIMP
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#endif

#include "DirectXCollision.h"

#include <algorithm>
#include <cctype>
#include <cstring>

namespace Immortal
{

inline void InterpolateQuaternion(Quaternion &pOut, const Quaternion &pStart, const Quaternion &pEnd, float pFactor)
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

    pOut = sclp * pStart + sclq * end;
}

inline void InterpolateVector3(Vector3 &pOut, const Vector3 &pStart, const Vector3 &pEnd, float pFactor)
{
    float sclp = 1.0f - pFactor;
    float sclq = pFactor;

    pOut = sclp * pStart + sclq * pEnd;
}

template <class T, class U>
inline constexpr T Interpolate(const std::set<U> &keys, float animationTime)
{
    T ret{};

    if (keys.size() == 1)
    {
        return keys.begin()->Value;
    }

    auto it = keys.lower_bound(animationTime);
    if (it == keys.end())
    {
        return keys.rbegin()->Value;
    }
    if (it == keys.begin())
    {
        return it->Value;
    }

    const U &end   = *it;
    const U &start = *--it;

    float deltaTime = end.Time - start.Time;
    float factor = (animationTime - (float)start.Time) / deltaTime;
    SLASSERT(factor >= 0.0f && factor <= 1.0f);

    if constexpr (IsPrimitiveOf<QuaternionKey, U>())
    {
        InterpolateQuaternion(ret, start.Value, end.Value, factor);
        return T{ Vector::Normalize(ret) };
    }
    else
    {
        InterpolateVector3(ret, start.Value, end.Value, factor);
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
    aiProcess_LimitBoneWeights |
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

std::vector<Ref<Mesh>> Mesh::Primitives;

static inline std::string ExtractModelFileWorkspace(const std::string &modelpath)
{
    std::filesystem::path path{ modelpath };
    return path.parent_path().string();
}

#if HAVE_ASSIMP
namespace
{

/** Assimp returns paths as in the file (often relative to the model, e.g. ../textures/a.png). */
static std::string ResolveTexturePathRelativeToModel(const std::string &modelFilepath, const char *fromAssimp)
{
    if (!fromAssimp || !fromAssimp[0])
    {
        return {};
    }

    std::error_code ec;
    std::filesystem::path raw{std::string(fromAssimp)};
    if (raw.is_absolute())
    {
        if (std::filesystem::exists(raw))
        {
            std::filesystem::path c = std::filesystem::weakly_canonical(raw, ec);
            return ec ? raw.string() : c.string();
        }
        return raw.string();
    }

    std::filesystem::path base = std::filesystem::path(modelFilepath).parent_path();

    std::filesystem::path path = base / raw;
    if (!std::filesystem::exists(path))
    {
        path = base.parent_path() / "textures" / raw;
        if (!std::filesystem::exists(path))
        {
            path = base.parent_path() / "textures" / raw.filename();
        }
    }
    if (std::filesystem::exists(path))
    {
        std::filesystem::path c = std::filesystem::weakly_canonical(path, ec);
        return ec ? path.string() : c.string();
    }

    return path.string();
}

static void TryLoadMaterialTexture(Ref<Texture> &out, const std::string &resolvedPath, AsyncComputeThread *asyncComputeThread)
{
    if (resolvedPath.empty())
    {
        return;
    }
    namespace fs = std::filesystem;
    if (!fs::exists(fs::path(resolvedPath)))
    {
        LOG::WARN("Mesh: texture not found (skipped): {0}", resolvedPath);
        return;
    }
    Ref<Texture> tex = Graphics::CreateTexture(String(resolvedPath), asyncComputeThread);
    if (tex)
    {
        out = tex;
    }
}

/** Many exporters put standalone roughness maps in UNKNOWN (e.g. UE-style T_Name_0_R.png). */
static bool FilenameLooksLikeRoughnessMap(const char *path)
{
    if (!path || !path[0])
    {
        return false;
    }
    std::string s(path);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    if (s.find("roughness") != std::string::npos || s.find("_rough") != std::string::npos)
    {
        return true;
    }
    static const char *suffixes[] = {
        "_r.png", "_r.jpg", "_r.jpeg", "_r.tga", "_r.tif", "_r.tiff", "_r.bmp", "_r.dds"
    };
    for (const char *suf : suffixes)
    {
        const size_t len = std::strlen(suf);
        if (s.size() >= len && s.compare(s.size() - len, len, suf) == 0)
        {
            return true;
        }
    }
    return false;
}

/**
 * Roughness: DIFFUSE_ROUGHNESS (all slots), then UNKNOWN filenames like *_R.png (before packed MR),
 * then glTF metallic-roughness, shininess, Maya specular roughness.
 * Enum values 25 / 27 are stable in Assimp 5.x (MAYA_SPECULAR_ROUGHNESS / GLTF_METALLIC_ROUGHNESS).
 */
static void TryLoadRoughnessMaps(aiMaterial *aiMaterial, const std::string &filepath, Material &material, AsyncComputeThread *asyncComputeThread)
{
    aiString texturePath;

    for (unsigned ti = 0; ti < aiMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS); ti++)
    {
        if (aiMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, ti, &texturePath) == AI_SUCCESS)
        {
            std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
            material.Pathes.Roughness = String(resolved);
            TryLoadMaterialTexture(material.Textures.Roughness, resolved, asyncComputeThread);
            return;
        }
    }

    // Dedicated roughness files are often aiTextureType_UNKNOWN (e.g. *_R.png); prefer before packed MR.
    for (unsigned ti = 0; ti < aiMaterial->GetTextureCount(aiTextureType_UNKNOWN); ti++)
    {
        if (aiMaterial->GetTexture(aiTextureType_UNKNOWN, ti, &texturePath) != AI_SUCCESS)
        {
            continue;
        }
        if (!FilenameLooksLikeRoughnessMap(texturePath.C_Str()))
        {
            continue;
        }
        std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
        material.Pathes.Roughness = String(resolved);
        TryLoadMaterialTexture(material.Textures.Roughness, resolved, asyncComputeThread);
        return;
    }

    const auto kGltfMetallicRoughness = (aiTextureType)27;
    if (aiMaterial->GetTextureCount(kGltfMetallicRoughness) > 0 &&
        aiMaterial->GetTexture(kGltfMetallicRoughness, 0, &texturePath) == AI_SUCCESS)
    {
        std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
        material.Pathes.Roughness = String(resolved);
        TryLoadMaterialTexture(material.Textures.Roughness, resolved, asyncComputeThread);
        return;
    }

    if (aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &texturePath) == AI_SUCCESS)
    {
        std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
        material.Pathes.Roughness = String(resolved);
        TryLoadMaterialTexture(material.Textures.Roughness, resolved, asyncComputeThread);
        return;
    }

    const auto kMayaSpecularRoughness = (aiTextureType)25;
    if (aiMaterial->GetTextureCount(kMayaSpecularRoughness) > 0 &&
        aiMaterial->GetTexture(kMayaSpecularRoughness, 0, &texturePath) == AI_SUCCESS)
    {
        std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
        material.Pathes.Roughness = String(resolved);
        TryLoadMaterialTexture(material.Textures.Roughness, resolved, asyncComputeThread);
        return;
    }
}

}
#endif

void Mesh::LoadPrimitives()
{

}

static Ref<Buffer> CreateStagingBuffer(const void *data, size_t size)
{
    Ref<Buffer> stagingBuffer = Graphics::GetCachedBuffer(BufferType::TransferSource, size);
    stagingBuffer->Fill(data, size, 0);

    return stagingBuffer;
}

static void TransferBuffer2Device(CommandBuffer *commandBuffer, Ref<Buffer> &buffer, const Ref<Buffer> &stagingBuffer, const void *data, size_t size)
{
    commandBuffer->MemoryCopy(buffer, 0, stagingBuffer, 0, size);
}

template <typename T>
class Span
{
public:
    Span() :
        m_data(nullptr), m_count(0)
    {}

    Span(T *data, uint32_t count) :
        m_data(data), m_count(count)
    {}

    // std library container interface
    T *data()
    {
        return m_data;
    }
    const T *data() const
    {
        return m_data;
    }

    T &back()
    {
        return *(m_data + m_count - 1);
    }
    const T &back() const
    {
        return *(m_data + m_count - 1);
    }

    size_t size() const
    {
        return m_count;
    }

    // Iterator interface
    T *begin()
    {
        return m_data;
    }
    T *end()
    {
        return m_data + m_count;
    }

    T &operator[](uint32_t i)
    {
        return *(m_data + i);
    }
    const T &operator[](uint32_t i) const
    {
        return *(m_data + i);
    }

private:
    T *m_data;
    uint32_t m_count;
};

template <typename T>
Span<T> MakeSpan(T *data, uint32_t size)
{
    return Span<T>(data, size);
}


struct Attribute
{
    enum EType : uint32_t
    {
        Position,
        Normal,
        TexCoord,
        Tangent,
        Bitangent,
        Count
    };

    EType Type;
    uint32_t Offset;
};

class Model
{
public:
    struct PackedTriangle
    {
        uint32_t i0 : 10;
        uint32_t i1 : 10;
        uint32_t i2 : 10;
    };

    struct Mesh
    {
        D3D12_INPUT_ELEMENT_DESC LayoutElems[Attribute::Count];
        D3D12_INPUT_LAYOUT_DESC LayoutDesc;

        std::vector<Span<uint8_t>> Vertices;
        std::vector<uint32_t> VertexStrides;
        uint32_t VertexCount;
        DirectX::BoundingSphere BoundingSphere;

        Span<Subset> IndexSubsets;
        Span<uint8_t> Indices;
        uint32_t IndexSize;
        uint32_t IndexCount;

        Span<Subset> MeshletSubsets;
        Span<Meshlet> Meshlets;
        Span<uint8_t> UniqueVertexIndices;
        Span<PackedTriangle> PrimitiveIndices;
        Span<CullData> CullingData;

        // Calculates the number of instances of the last meshlet which can be packed into a single threadgroup.
        uint32_t GetLastMeshletPackCount(uint32_t subsetIndex, uint32_t maxGroupVerts, uint32_t maxGroupPrims)
        {
            if (Meshlets.size() == 0)
                return 0;

            auto &subset = MeshletSubsets[subsetIndex];
            auto &meshlet = Meshlets[subset.Offset + subset.Count - 1];

            return std::min(maxGroupVerts / meshlet.VertCount, maxGroupPrims / meshlet.PrimCount);
        }

        void GetPrimitive(uint32_t index, uint32_t &i0, uint32_t &i1, uint32_t &i2) const
        {
            auto prim = PrimitiveIndices[index];
            i0 = prim.i0;
            i1 = prim.i1;
            i2 = prim.i2;
        }

        uint32_t GetVertexIndex(uint32_t index) const
        {
            const uint8_t *addr = UniqueVertexIndices.data() + index * IndexSize;
            if (IndexSize == 4)
            {
                return *reinterpret_cast<const uint32_t *>(addr);
            }
            else
            {
                return *reinterpret_cast<const uint16_t *>(addr);
            }
        }
    };

    std::vector<Mesh> m_meshes;
    DirectX::BoundingSphere m_boundingSphere;

    std::vector<uint8_t> m_buffer;

    const D3D12_INPUT_ELEMENT_DESC c_elementDescs[Attribute::Count] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1},
            {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1},
    };

    static inline const uint32_t c_sizeMap[] =
    {
            12,        // Position
            12,        // Normal
            8,         // TexCoord
            12,        // Tangent
            12,        // Bitangent
    };

    const uint32_t c_prolog = 'MSHL';

    enum FileVersion
    {
        FILE_VERSION_INITIAL = 0,
        CURRENT_FILE_VERSION = FILE_VERSION_INITIAL
    };

    struct FileHeader
    {
        uint32_t Prolog;
        uint32_t Version;

        uint32_t MeshCount;
        uint32_t AccessorCount;
        uint32_t BufferViewCount;
        uint32_t BufferSize;
    };

    struct MeshHeader
    {
        uint32_t Indices;
        uint32_t IndexSubsets;
        uint32_t Attributes[Attribute::Count];

        uint32_t Meshlets;
        uint32_t MeshletSubsets;
        uint32_t UniqueVertexIndices;
        uint32_t PrimitiveIndices;
        uint32_t CullData;
    };

    struct BufferView
    {
        uint32_t Offset;
        uint32_t Size;
    };

    struct Accessor
    {
        uint32_t BufferView;
        uint32_t Offset;
        uint32_t Size;
        uint32_t Stride;
        uint32_t Count;
    };

    uint32_t GetFormatSize(DXGI_FORMAT format)
    {
        switch (format)
        {
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
                return 16;
            case DXGI_FORMAT_R32G32B32_FLOAT:
                return 12;
            case DXGI_FORMAT_R32G32_FLOAT:
                return 8;
            case DXGI_FORMAT_R32_FLOAT:
                return 4;
            default:
                throw std::exception("Unimplemented type");
        }
    }

    template <typename T, typename U>
    constexpr T DivRoundUp(T num, U denom)
    {
        return (num + denom - 1) / denom;
    }

    template <typename T>
    size_t GetAlignedSize(T size)
    {
        const size_t alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
        const size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);
        return alignedSize;
    }

    void LoadFromFile(const wchar_t *filename)
    {
        std::ifstream stream(filename, std::ios::binary);
        if (!stream.is_open())
        {
            return;
        }

        std::vector<MeshHeader> meshes;
        std::vector<BufferView> bufferViews;
        std::vector<Accessor> accessors;

        FileHeader header;
        stream.read(reinterpret_cast<char *>(&header), sizeof(header));

        if (header.Prolog != c_prolog)
        {
            return;        // Incorrect file format.
        }

        if (header.Version != CURRENT_FILE_VERSION)
        {
            return;        // Version mismatch between export and import serialization code.
        }

        // Read mesh metdata
        meshes.resize(header.MeshCount);
        stream.read(reinterpret_cast<char *>(meshes.data()), meshes.size() * sizeof(meshes[0]));

        accessors.resize(header.AccessorCount);
        stream.read(reinterpret_cast<char *>(accessors.data()), accessors.size() * sizeof(accessors[0]));

        bufferViews.resize(header.BufferViewCount);
        stream.read(reinterpret_cast<char *>(bufferViews.data()), bufferViews.size() * sizeof(bufferViews[0]));

        m_buffer.resize(header.BufferSize);
        stream.read(reinterpret_cast<char *>(m_buffer.data()), header.BufferSize);

        char eofbyte;
        stream.read(&eofbyte, 1);        // Read last byte to hit the eof bit

        assert(stream.eof());        // There's a problem if we didn't completely consume the file contents.

        stream.close();

        // Populate mesh data from binary data and metadata.
        m_meshes.resize(meshes.size());
        for (uint32_t i = 0; i < static_cast<uint32_t>(meshes.size()); ++i)
        {
            auto &meshView = meshes[i];
            auto &mesh = m_meshes[i];

            // Index data
            {
                Accessor &accessor = accessors[meshView.Indices];
                BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.IndexSize = accessor.Size;
                mesh.IndexCount = accessor.Count;

                mesh.Indices = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
            }

            // Index Subset data
            {
                Accessor &accessor = accessors[meshView.IndexSubsets];
                BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.IndexSubsets = MakeSpan(reinterpret_cast<Subset *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Vertex data & layout metadata

            // Determine the number of unique Buffer Views associated with the vertex attributes & copy vertex buffers.
            std::vector<uint32_t> vbMap;

            mesh.LayoutDesc.pInputElementDescs = mesh.LayoutElems;
            mesh.LayoutDesc.NumElements = 0;

            for (uint32_t j = 0; j < Attribute::Count; ++j)
            {
                if (meshView.Attributes[j] == -1)
                    continue;

                Accessor &accessor = accessors[meshView.Attributes[j]];

                auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);
                if (it != vbMap.end())
                {
                    continue;        // Already added - continue.
                }

                // New buffer view encountered; add to list and copy vertex data
                vbMap.push_back(accessor.BufferView);
                BufferView &bufferView = bufferViews[accessor.BufferView];

                Span<uint8_t> verts = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);

                mesh.VertexStrides.push_back(accessor.Stride);
                mesh.Vertices.push_back(verts);
                mesh.VertexCount = static_cast<uint32_t>(verts.size()) / accessor.Stride;
            }

            // Populate the vertex buffer metadata from accessors.
            for (uint32_t j = 0; j < Attribute::Count; ++j)
            {
                if (meshView.Attributes[j] == -1)
                    continue;

                Accessor &accessor = accessors[meshView.Attributes[j]];

                // Determine which vertex buffer index holds this attribute's data
                auto it = std::find(vbMap.begin(), vbMap.end(), accessor.BufferView);

                D3D12_INPUT_ELEMENT_DESC desc = c_elementDescs[j];
                desc.InputSlot = static_cast<uint32_t>(std::distance(vbMap.begin(), it));

                mesh.LayoutElems[mesh.LayoutDesc.NumElements++] = desc;
            }

            // Meshlet data
            {
                Accessor &accessor = accessors[meshView.Meshlets];
                BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.Meshlets = MakeSpan(reinterpret_cast<Meshlet *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Meshlet Subset data
            {
                Accessor &accessor = accessors[meshView.MeshletSubsets];
                BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.MeshletSubsets = MakeSpan(reinterpret_cast<Subset *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Unique Vertex Index data
            {
                Accessor &accessor = accessors[meshView.UniqueVertexIndices];
                BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.UniqueVertexIndices = MakeSpan(m_buffer.data() + bufferView.Offset, bufferView.Size);
            }

            // Primitive Index data
            {
                Accessor &accessor = accessors[meshView.PrimitiveIndices];
                BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.PrimitiveIndices = MakeSpan(reinterpret_cast<PackedTriangle *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }

            // Cull data
            {
                Accessor &accessor = accessors[meshView.CullData];
                BufferView &bufferView = bufferViews[accessor.BufferView];

                mesh.CullingData = MakeSpan(reinterpret_cast<CullData *>(m_buffer.data() + bufferView.Offset), accessor.Count);
            }
        }

        // Build bounding spheres for each mesh
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_meshes.size()); ++i)
        {
            auto &m = m_meshes[i];

            uint32_t vbIndexPos = 0;

            // Find the index of the vertex buffer of the position attribute
            for (uint32_t j = 1; j < m.LayoutDesc.NumElements; ++j)
            {
                auto &desc = m.LayoutElems[j];
                if (strcmp(desc.SemanticName, "POSITION") == 0)
                {
                    vbIndexPos = j;
                    break;
                }
            }

            // Find the byte offset of the position attribute with its vertex buffer
            uint32_t positionOffset = 0;

            for (uint32_t j = 0; j < m.LayoutDesc.NumElements; ++j)
            {
                auto &desc = m.LayoutElems[j];
                if (strcmp(desc.SemanticName, "POSITION") == 0)
                {
                    break;
                }

                if (desc.InputSlot == vbIndexPos)
                {
                    positionOffset += GetFormatSize(m.LayoutElems[j].Format);
                }
            }

            Vector3 *v0 = reinterpret_cast<Vector3 *>(m.Vertices[vbIndexPos].data() + positionOffset);
            uint32_t stride = m.VertexStrides[vbIndexPos];

            //DirectX::BoundingSphere::CreateFromPoints(m.BoundingSphere, m.VertexCount, (XMFLOAT3 *)v0, stride);

            //if (i == 0)
            //{
            //	m_boundingSphere = m.BoundingSphere;
            //}
            //else
            //{
            //	BoundingSphere::CreateMerged(m_boundingSphere, m_boundingSphere, m.BoundingSphere);
            //}
        }
    }
};

Mesh::Mesh(AsyncComputeThread *asyncComputeThread, CommandBuffer *commandBuffer, const std::string &filepath) :
    path{ filepath },
    vertexType{ VertexType::Common }
{
#if !HAVE_ASSIMP
    ThrowIf(false, "Assimp library not Found! Unable to import mesh from local file");
#else
    if (std::filesystem::path(filepath).extension() == ".bin")
    {
        std::filesystem::path file = filepath;
        Model model;
        model.LoadFromFile(file.c_str());

        auto &m = model.m_meshes[0];

        uint32_t vertexBufferSize              = m.Vertices[0].size();
        uint32_t indexBufferSize               = m.Indices.size();
        uint32_t meshletBufferSize             = m.Meshlets.size() * sizeof(m.Meshlets[0]);
        uint32_t uniqueVertexIndicesBufferSize = m.UniqueVertexIndices.size();
        uint32_t primitiveIndicesBufferSize    = m.PrimitiveIndices.size() * sizeof(m.PrimitiveIndices[0]);

        nodes.resize(1);
        Node &node = nodes[0];

        {
            uint32_t stride = m.VertexStrides.empty() ? sizeof(DirectXSampleVertex) : m.VertexStrides[0];
            uint32_t count = m.VertexCount;
            const uint8_t *base = m.Vertices[0].data();
            Vector3 mn{ FLT_MAX, FLT_MAX, FLT_MAX };
            Vector3 mx{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
            for (uint32_t vi = 0; vi < count; vi++)
            {
                const Vector3 &p = *reinterpret_cast<const Vector3 *>(base + vi * stride);
                mn.x = std::min(mn.x, p.x); mn.y = std::min(mn.y, p.y); mn.z = std::min(mn.z, p.z);
                mx.x = std::max(mx.x, p.x); mx.y = std::max(mx.y, p.y); mx.z = std::max(mx.z, p.z);
            }
            node.AABBMin = mn;
            node.AABBMax = mx;
        }

        auto device = Graphics::GetDevice();
        node.Name = "Unknown";
        node.MeshletSubsetCount = model.m_meshes[0].MeshletSubsets[0].Count;
        node.Vertex              = device->CreateBuffer(BufferType::Vertex | BufferType::Storage, vertexBufferSize,              MemoryType::Device, sizeof(DirectXSampleVertex));
        node.Index               = device->CreateBuffer(BufferType::Index,   indexBufferSize,               MemoryType::Device, sizeof(uint32_t));
        node.Meshlets            = device->CreateBuffer(BufferType::Storage, meshletBufferSize,             MemoryType::Device, sizeof(Meshlet));
        node.UniqueVertexIndices = device->CreateBuffer(BufferType::Storage, uniqueVertexIndicesBufferSize, MemoryType::Device, sizeof(uint32_t));
        node.PrimitiveIndices    = device->CreateBuffer(BufferType::Storage, primitiveIndicesBufferSize,    MemoryType::Device, sizeof(m.PrimitiveIndices[0]));

        node.Vertex->SetDebugName("VertexBuffer");

        node.Meshlets->SetDebugName("Meshlets");

        node.UniqueVertexIndices->SetDebugName("UniqueVertexIndices");

        node.PrimitiveIndices->SetDebugName("PrimitiveIndices");
        
        Ref<Buffer> stagingVertex              = CreateStagingBuffer(m.Vertices[0].data(),         vertexBufferSize);            
        Ref<Buffer> stagingIndex               = CreateStagingBuffer(m.Indices.data(),             indexBufferSize);             
        Ref<Buffer> stagingMeshlet             = CreateStagingBuffer(m.Meshlets.data(),            meshletBufferSize);           
        Ref<Buffer> stagingUniqueVertexIndices = CreateStagingBuffer(m.UniqueVertexIndices.data(), uniqueVertexIndicesBufferSize);
        Ref<Buffer> stagingPrimitiveIndices    = CreateStagingBuffer(m.PrimitiveIndices.data(),    primitiveIndicesBufferSize);
        asyncComputeThread->Execute<RecordingTask>([&, this] (CommandBuffer *commandBuffer) {
            TransferBuffer2Device(commandBuffer, node.Vertex,              stagingVertex,              m.Vertices[0].data(),         vertexBufferSize);
            TransferBuffer2Device(commandBuffer, node.Index,               stagingIndex,               m.Indices.data(),             indexBufferSize);
            TransferBuffer2Device(commandBuffer, node.Meshlets,            stagingMeshlet,             m.Meshlets.data(),            meshletBufferSize);
            TransferBuffer2Device(commandBuffer, node.UniqueVertexIndices, stagingUniqueVertexIndices, m.UniqueVertexIndices.data(), uniqueVertexIndicesBufferSize);
            TransferBuffer2Device(commandBuffer, node.PrimitiveIndices,    stagingPrimitiveIndices,    m.PrimitiveIndices.data(),    primitiveIndicesBufferSize);
        });

        asyncComputeThread->Execute<ExecutionCompletedTask>([=] {
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingVertex);
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingIndex);
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingMeshlet);
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingUniqueVertexIndices);
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingPrimitiveIndices);
        });

        return;
    }

    std::string workspace = ExtractModelFileWorkspace(path);

    LogStream::initialize();

    LOG::INFO("Loading mesh: {0}", filepath.c_str());
    std::unique_ptr<Assimp::Importer> importer{ new Assimp::Importer() };

    const aiScene *scene = importer->ReadFile(filepath, ImportFlags);
    SLASSERT(scene && scene->HasMeshes() && "Failed to load Mesh file: {0}" && filepath.c_str());

    std::vector<CommonVertex> vertices;
    std::vector<SkeletonVertex> skeletonVertices;
    bool useSkeletonVertices = false;
    std::vector<Face> faces;
    std::vector<BufferBindInfo> vertexBindInfo;
    std::vector<BufferBindInfo> indexBindInfo;

    LoadModelData(scene, vertices, skeletonVertices, useSkeletonVertices, faces, vertexBindInfo, indexBindInfo);

    for (size_t i = 0; i < nodes.size(); i++)
    {
        auto &node = nodes[i];
        const size_t vertStride = useSkeletonVertices ? sizeof(SkeletonVertex) : sizeof(CommonVertex);
        auto numVertices = vertexBindInfo[i].size / vertStride;
        auto startVertices = vertexBindInfo[i].offset / vertStride;
        auto numIndicies = indexBindInfo[i].size / sizeof(uint32_t);
        auto startIndices = indexBindInfo[i].offset / sizeof(uint32_t);

        std::vector<Vector3> positions;
        positions.resize(numVertices);
        for (size_t vi = 0; vi < numVertices; vi++)
        {
            positions[vi] = useSkeletonVertices ? skeletonVertices[startVertices + vi].Position : vertices[startVertices + vi].Position;
        }

        size_t vertexStride = vertStride;
        const void *pVertex = useSkeletonVertices ? (const void *)&skeletonVertices[startVertices] : (const void *)&vertices[startVertices];
        const uint32_t *indices = (uint32_t *)faces.data();
        MeshletGenerator generator;
        generator.BuildMeshlets((const void *)pVertex, numVertices, vertexStride, &indices[startIndices], numIndicies);

  //      auto &meshlets            = generator.meshlets;
        //auto &uniqueVertexIndices = generator.uniqueVertexIndices;
        //auto &primitiveIndices    = generator.primitiveIndices;
        auto &meshlets            = generator.meshlets;
        auto &uniqueVertexIndices = generator.meshletVertices;
        auto &primitiveIndices    = generator.meshletTrianglesU32;

        uint32_t vertexBufferSize              = numVertices           * vertexStride;
        uint32_t indexBufferSize               = faces.size()               * sizeof(faces[0]);
        uint32_t meshletBufferSize             = meshlets.size()            * sizeof(meshlets[0]);
        uint32_t uniqueVertexIndicesBufferSize = uniqueVertexIndices.size() * sizeof(uniqueVertexIndices[0]);
        uint32_t primitiveIndicesBufferSize    = primitiveIndices.size()    * sizeof(primitiveIndices[0]);

        node.MeshletSubsetCount = meshlets.size();// generator.meshletSubsets[0].Count;

        auto device = Graphics::GetDevice();
        node.Vertex              = device->CreateBuffer(BufferType::Storage, vertexBufferSize,              MemoryType::Device, vertexStride  );
        node.Index               = device->CreateBuffer(BufferType::Index,   indexBufferSize,               MemoryType::Device);
        node.Meshlets            = device->CreateBuffer(BufferType::Storage, meshletBufferSize,             MemoryType::Device, sizeof(meshlets[0])   );
        node.UniqueVertexIndices = device->CreateBuffer(BufferType::Storage, uniqueVertexIndicesBufferSize, MemoryType::Device, sizeof(uniqueVertexIndices[0]));
        node.PrimitiveIndices    = device->CreateBuffer(BufferType::Storage, primitiveIndicesBufferSize,    MemoryType::Device, sizeof(primitiveIndices[0])   );

        Ref<Buffer> stagingVertex              = CreateStagingBuffer(pVertex,                    vertexBufferSize             );
        Ref<Buffer> stagingIndex               = CreateStagingBuffer(faces.data(),               indexBufferSize              );
        Ref<Buffer> stagingMeshlet             = CreateStagingBuffer(meshlets.data(),            meshletBufferSize            );
        Ref<Buffer> stagingUniqueVertexIndices = CreateStagingBuffer(uniqueVertexIndices.data(), uniqueVertexIndicesBufferSize);
        Ref<Buffer> stagingPrimitiveIndices    = CreateStagingBuffer(primitiveIndices.data(),    primitiveIndicesBufferSize   );

        asyncComputeThread->Execute<RecordingTask>([=, this] (CommandBuffer *commandBuffer) {
            auto &node = nodes[i];
            TransferBuffer2Device(commandBuffer, node.Vertex,              stagingVertex,              pVertex,                    vertexBufferSize             );
            TransferBuffer2Device(commandBuffer, node.Index,               stagingIndex,               faces.data(),               indexBufferSize              );
            TransferBuffer2Device(commandBuffer, node.Meshlets,            stagingMeshlet,             meshlets.data(),            meshletBufferSize            );
            TransferBuffer2Device(commandBuffer, node.UniqueVertexIndices, stagingUniqueVertexIndices, uniqueVertexIndices.data(), uniqueVertexIndicesBufferSize);
            TransferBuffer2Device(commandBuffer, node.PrimitiveIndices,    stagingPrimitiveIndices,    primitiveIndices.data(),    primitiveIndicesBufferSize   );
        });

        asyncComputeThread->Execute<ExecutionCompletedTask>([=] {
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingVertex);
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingIndex);
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingMeshlet);
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingUniqueVertexIndices);
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingPrimitiveIndices);
        });

    }

    if (scene->HasMaterials())
    {
        materials.resize(scene->mNumMaterials);
        for (size_t i = 0; i < scene->mNumMaterials; i++)
        {
            auto &aiMaterial = scene->mMaterials[i];
            auto &material   = materials[i];
            aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE,   material.AlbedoColor);
            aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR,  material.Specular   );
            aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT,   material.Ambient    );
            aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE,  material.Emissive   );
            aiMaterial->Get(AI_MATKEY_OPACITY,         material.Opacity    );

            if (aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, material.Roughness) != AI_SUCCESS)
            {
                float shininess = 0.0f;
                if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS && shininess > 0.0f)
                {
                    material.Roughness = std::sqrt(2.0f / (shininess + 2.0f));
                }
            }

            if (aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, material.Metallic) != AI_SUCCESS)
            {
                float specularIntensity = 0.0f;
                if (aiMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, specularIntensity) == AI_SUCCESS && specularIntensity > 0.5f)
                {
                    material.Metallic = std::clamp((specularIntensity - 0.5f) * 2.0f, 0.0f, 1.0f);
                }
            }

            aiString texturePath;
            if (aiMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath) == AI_SUCCESS ||
                aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
            {
                std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
                material.Pathes.Diffuse = String(resolved);
                TryLoadMaterialTexture(material.Textures.Albedo, resolved, asyncComputeThread);
            }
            if (aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS)
            {
                std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
                material.Pathes.Specular = String(resolved);
                TryLoadMaterialTexture(material.Textures.Specular, resolved, asyncComputeThread);
            }
            if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS ||
                aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS)
            {
                std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
                material.Pathes.Normal = String(resolved);
                TryLoadMaterialTexture(material.Textures.Normal, resolved, asyncComputeThread);
            }
            if (aiMaterial->GetTexture(aiTextureType_LIGHTMAP, 0, &texturePath) == AI_SUCCESS ||
                aiMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) == AI_SUCCESS)
            {
                std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
                material.Pathes.AmbientOcclusion = String(resolved);
                TryLoadMaterialTexture(material.Textures.AmbientOcclusion, resolved, asyncComputeThread);
            }
            if (aiMaterial->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS)
            {
                std::string resolved = ResolveTexturePathRelativeToModel(filepath, texturePath.C_Str());
                material.Pathes.Metallic = String(resolved);
                TryLoadMaterialTexture(material.Textures.Metallic, resolved, asyncComputeThread);
            }
            TryLoadRoughnessMaps(aiMaterial, filepath, material, asyncComputeThread);
        }
    }

    if (vertexType == VertexType::Skeleton)
    {
        CalculatedBoneTransform(Matrix4(1.0f));
        UpdateBoneTransforms();
    }
#endif
}

Mesh::Mesh(const std::vector<SimpleVertex> &vertices, const std::vector<Index> &indicies) :
    vertexType{ VertexType::Common }
{
    Node head{ "Undefined" };

    std::vector<CommonVertex> commonVertices(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++)
    {
        auto &sv = vertices[i];
        auto &cv = commonVertices[i];
        cv.Position = sv.Position;
        cv.Normal   = sv.Normal;
        cv.Texcoord = sv.Texcoord;

        static const Vector3 kUp = { 0.0f, 1.0f, 0.0f };
        static const Vector3 kRight = { 1.0f, 0.0f, 0.0f };
        Vector3 ref = (std::abs(sv.Normal.y) < 0.999f) ? kUp : kRight;
        cv.Tangent = Vector::Normalize(Vector::Cross(sv.Normal, ref));
    }

    const uint32_t *indices = (const uint32_t *)indicies.data();
    uint32_t numIndices = (uint32_t)indicies.size() * 3;

    MeshletGenerator generator;
    generator.BuildMeshlets(commonVertices.data(), (uint32_t)commonVertices.size(), sizeof(CommonVertex), indices, numIndices);

    auto &meshlets            = generator.meshlets;
    auto &uniqueVertexIndices = generator.meshletVertices;
    auto &primitiveIndices    = generator.meshletTrianglesU32;

    uint32_t vertexBufferSize              = (uint32_t)(sizeof(CommonVertex) * commonVertices.size());
    uint32_t indexBufferSize               = (uint32_t)(sizeof(Index) * indicies.size());
    uint32_t meshletBufferSize             = (uint32_t)(meshlets.size() * sizeof(meshlets[0]));
    uint32_t uniqueVertexIndicesBufferSize = (uint32_t)(uniqueVertexIndices.size() * sizeof(uniqueVertexIndices[0]));
    uint32_t primitiveIndicesBufferSize    = (uint32_t)(primitiveIndices.size() * sizeof(primitiveIndices[0]));

    head.MeshletSubsetCount = (uint32_t)meshlets.size();

    auto device = Graphics::GetDevice();
    head.Vertex              = device->CreateBuffer(BufferType::Storage, vertexBufferSize,              MemoryType::Device, sizeof(CommonVertex));
    head.Index               = device->CreateBuffer(BufferType::Index,   indexBufferSize,               MemoryType::Device);
    head.Meshlets            = device->CreateBuffer(BufferType::Storage, meshletBufferSize,             MemoryType::Device, sizeof(meshlets[0]));
    head.UniqueVertexIndices = device->CreateBuffer(BufferType::Storage, uniqueVertexIndicesBufferSize, MemoryType::Device, sizeof(uniqueVertexIndices[0]));
    head.PrimitiveIndices    = device->CreateBuffer(BufferType::Storage, primitiveIndicesBufferSize,    MemoryType::Device, sizeof(primitiveIndices[0]));

    Ref<Buffer> stagingVertex              = CreateStagingBuffer(commonVertices.data(),      vertexBufferSize);
    Ref<Buffer> stagingIndex               = CreateStagingBuffer(indicies.data(),            indexBufferSize);
    Ref<Buffer> stagingMeshlet             = CreateStagingBuffer(meshlets.data(),            meshletBufferSize);
    Ref<Buffer> stagingUniqueVertexIndices = CreateStagingBuffer(uniqueVertexIndices.data(), uniqueVertexIndicesBufferSize);
    Ref<Buffer> stagingPrimitiveIndices    = CreateStagingBuffer(primitiveIndices.data(),    primitiveIndicesBufferSize);

    nodes.emplace_back(head);

    auto *asyncComputeThread = Graphics::GetAsyncComputeThread();
    asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
        auto &h = nodes.back();
        TransferBuffer2Device(commandBuffer, h.Vertex,              stagingVertex,              commonVertices.data(),      vertexBufferSize);
        TransferBuffer2Device(commandBuffer, h.Index,               stagingIndex,               indicies.data(),            indexBufferSize);
        TransferBuffer2Device(commandBuffer, h.Meshlets,            stagingMeshlet,             meshlets.data(),            meshletBufferSize);
        TransferBuffer2Device(commandBuffer, h.UniqueVertexIndices, stagingUniqueVertexIndices, uniqueVertexIndices.data(), uniqueVertexIndicesBufferSize);
        TransferBuffer2Device(commandBuffer, h.PrimitiveIndices,    stagingPrimitiveIndices,    primitiveIndices.data(),    primitiveIndicesBufferSize);
    });
    asyncComputeThread->Execute<ExecutionCompletedTask>([=] {
        Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingVertex);
        Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingIndex);
        Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingMeshlet);
        Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingUniqueVertexIndices);
        Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingPrimitiveIndices);
    });
}

Mesh::Mesh(AsyncComputeThread *asyncComputeThread, CommandBuffer *commandBuffer, const void *pVertex, size_t numVertex, const Index *pIndex, size_t numIndex, VertexType type, const std::string &name) :
    vertexType{type}
{
    Node head{name};

    static const size_t kVertexSize[] = {
        sizeof(SimpleVertex),
        sizeof(CommonVertex),
        sizeof(SkeletonVertex)
    };

    auto &size = kVertexSize[(size_t) type];

    uint32_t vertexBufferSize = size * numVertex;
    uint32_t indexBufferSize  = sizeof(Index) * numIndex;

    head.Vertex = Graphics::CreateBuffer(Buffer::Type::Vertex, vertexBufferSize, MemoryType::Device);
    head.Index  = Graphics::CreateBuffer(Buffer::Type::Index,   indexBufferSize,  MemoryType::Device);

    Ref<Buffer> stagingVertex = CreateStagingBuffer(pVertex, vertexBufferSize);
    Ref<Buffer> stagingIndex  = CreateStagingBuffer(pIndex, indexBufferSize);

    nodes.emplace_back(head);

    asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
        auto &h = nodes.back();
        TransferBuffer2Device(commandBuffer, h.Vertex, stagingVertex, pVertex, vertexBufferSize);
        TransferBuffer2Device(commandBuffer, h.Index,  stagingIndex, pIndex, indexBufferSize);
    });
    asyncComputeThread->Execute<ExecutionCompletedTask>([=] {
        Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingVertex);
        Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingIndex);
    });
}

Mesh::~Mesh()
{
    for (auto &node : nodes)
    {
        Graphics::ReleaseResource(node.Vertex             );
        Graphics::ReleaseResource(node.Index         	  );
        Graphics::ReleaseResource(node.Meshlets           );
        Graphics::ReleaseResource(node.UniqueVertexIndices);        
        Graphics::ReleaseResource(node.PrimitiveIndices   );
    }

    materials.clear();
}

void Mesh::ReadHierarchyBoneNode(float animationTime, const BoneNode *node, const Matrix4 &parentTransform)
{
    Matrix4 globalTransform{};
    Matrix4 nodeTransform = node->Transform;

    if (state.currentAnimation < animations.size())
    {
        auto &animationsNode = animations[state.currentAnimation].Nodes;   
        if (auto it = animationsNode.find(node->Name); it != animationsNode.end())
        {
            AnimationNode &node = it->second;
            Vector3 position    = Interpolate<Vector3, VectorKey>(node.PositionKeys, animationTime);
            Vector3 scaling     = Interpolate<Vector3, VectorKey>(node.ScalingKeys, animationTime);
            Quaternion rotation = Interpolate<Quaternion, QuaternionKey>(node.RotationKeys, animationTime);

            nodeTransform = Vector::Translate(position) * Vector::ToMatrix4(rotation) * Vector::Scale(scaling);
        }
    }

    globalTransform  = parentTransform * nodeTransform;  
    if (auto it = bones.find(node->Name); it != bones.end())
    {
        auto &boneInfo = it->second;
        transforms[boneInfo.Id] = globalInverseTransform * globalTransform * boneInfo.OffsetMatrix;
    }
    else
    {
        for (const auto &mesh : node->Meshes)
        {
            transforms[mesh] = globalInverseTransform * globalTransform;
        }
    }

    auto &children = node->Children;
    for (auto &child : children)
    {
        ReadHierarchyBoneNode(animationTime, &child, globalTransform);
    }
}

void Mesh::CalculatedBoneTransform(const Matrix4 &parentTransform)
{
    transforms[0] = parentTransform;

    float timestamp = 0.0f;
    if (IsAnimated())
    {
        timestamp = animations[state.currentAnimation].Timestamp;
    }

    ReadHierarchyBoneNode(timestamp, rootNode, parentTransform);
}

void Mesh::UpdateBoneTransforms()
{
    if (!transformBuffer || transforms.empty())
    {
        return;
    }
    const size_t bytes = transforms.size() * sizeof(Matrix4);
    void *mapped = nullptr;
    transformBuffer->Map(&mapped, bytes, 0);
    if (mapped)
    {
        std::memcpy(mapped, transforms.data(), bytes);
        transformBuffer->Unmap();
    }
}

#if HAVE_ASSIMP
void Mesh::LoadModelData(const aiScene *scene, std::vector<CommonVertex> &vertices, std::vector<SkeletonVertex> &skeletonVertices, bool &useSkeletonVertices, std::vector<Face> &faces, std::vector<BufferBindInfo> &vertexBindInfo, std::vector<BufferBindInfo> &indexBindInfo)
{
    useSkeletonVertices = false;
    uint32_t totalVertices = 0;
    uint32_t totalFaces = 0;

    auto &numMeshes = scene->mNumMeshes;
    for (size_t i = 0; i < numMeshes; i++)
    {
        totalVertices += scene->mMeshes[i]->mNumVertices;
        totalFaces += scene->mMeshes[i]->mNumFaces;
    }

    vertices.reserve(totalVertices);
    faces.reserve(totalFaces);

    vertexBindInfo.resize(numMeshes);
    indexBindInfo.resize(numMeshes);
    nodes.resize(numMeshes);

    for (uint32_t i = 0; i < numMeshes; i++)
    {
        auto mesh = scene->mMeshes[i];
        auto &node = nodes[i];
        node.Name = mesh->mName.C_Str();
        node.MaterialIndex = mesh->mMaterialIndex;

        THROWIF(!mesh->HasPositions() || !mesh->HasNormals(), "No Position or Normals in the mesh object");

        Vector3 aabbMin{ FLT_MAX, FLT_MAX, FLT_MAX };
        Vector3 aabbMax{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
        for (size_t j = 0; j < mesh->mNumVertices; j++)
        {
            auto &vertex = vertices.emplace_back();
            vertex.Position = { mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z };
            vertex.Normal = { mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z };

            aabbMin.x = std::min(aabbMin.x, vertex.Position.x);
            aabbMin.y = std::min(aabbMin.y, vertex.Position.y);
            aabbMin.z = std::min(aabbMin.z, vertex.Position.z);
            aabbMax.x = std::max(aabbMax.x, vertex.Position.x);
            aabbMax.y = std::max(aabbMax.y, vertex.Position.y);
            aabbMax.z = std::max(aabbMax.z, vertex.Position.z);

            if (mesh->HasTangentsAndBitangents())
            {
                vertex.Tangent = { mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z };
            }
            if (mesh->HasTextureCoords(0))
            {
                vertex.Texcoord = { mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y };
            }
        }
        node.AABBMin = aabbMin;
        node.AABBMax = aabbMax;
        vertexBindInfo[i].size = mesh->mNumVertices * sizeof(vertices[0]);

        for (size_t j = 0; j < mesh->mNumFaces; j++)
        {
            auto &face = faces.emplace_back();
            face.v1 = mesh->mFaces[j].mIndices[0];
            face.v2 = mesh->mFaces[j].mIndices[1];
            face.v3 = mesh->mFaces[j].mIndices[2];
        }
        indexBindInfo[i].size = mesh->mNumFaces * sizeof(Face);

        if (i > 0)
        {
            vertexBindInfo[i].offset = vertexBindInfo[i - 1].offset + vertexBindInfo[i - 1].size;
            indexBindInfo[i].offset  = indexBindInfo[i - 1].offset + indexBindInfo[i - 1].size;
        }
    }

    LoadAnimationData(scene);

    bool anyMeshHasBones = false;
    if (animations.size() > 0)
    {
        for (uint32_t i = 0; i < numMeshes; i++)
        {
            if (scene->mMeshes[i]->mNumBones > 0)
            {
                anyMeshHasBones = true;
                break;
            }
        }
    }

    uint32_t nextBoneId = scene->mNumMeshes;
    if (anyMeshHasBones)
    {
        useSkeletonVertices = true;
        vertexType = VertexType::Skeleton;
        skeletonVertices.resize(vertices.size());
        for (size_t i = 0; i < vertices.size(); i++)
        {
            skeletonVertices[i].Position = vertices[i].Position;
            skeletonVertices[i].Normal   = vertices[i].Normal;
            skeletonVertices[i].Tangent  = vertices[i].Tangent;
            skeletonVertices[i].Texcoord = vertices[i].Texcoord;
            skeletonVertices[i].BoneIds[0] = skeletonVertices[i].BoneIds[1] = skeletonVertices[i].BoneIds[2] = skeletonVertices[i].BoneIds[3] = 0;
            skeletonVertices[i].Weights    = Vector4{ 0.0f, 0.0f, 0.0f, 0.0f };
        }

        for (uint32_t i = 0; i < numMeshes; i++)
        {
            auto *mesh = scene->mMeshes[i];
            uint32_t baseVertex = (uint32_t)(vertexBindInfo[i].offset / sizeof(CommonVertex));
            if (mesh->mNumBones > 0)
            {
                LoadBoneData(mesh, skeletonVertices, baseVertex, nextBoneId);
                nodes[i].Animated = true;
            }
        }

        for (auto &sv : skeletonVertices)
        {
            float w = sv.Weights.x + sv.Weights.y + sv.Weights.z + sv.Weights.w;
            if (w < 1e-6f)
            {
                sv.BoneIds[0] = 0;
                sv.Weights    = Vector4{ 1.0f, 0.0f, 0.0f, 0.0f };
            }
            else if (w > 1e-6f && std::abs(w - 1.0f) > 1e-3f)
            {
                float inv = 1.0f / w;
                sv.Weights.x *= inv;
                sv.Weights.y *= inv;
                sv.Weights.z *= inv;
                sv.Weights.w *= inv;
            }
        }

        for (uint32_t i = 0; i < numMeshes; i++)
        {
            vertexBindInfo[i].offset = (i == 0) ? 0 : vertexBindInfo[i - 1].offset + vertexBindInfo[i - 1].size;
            vertexBindInfo[i].size   = scene->mMeshes[i]->mNumVertices * sizeof(SkeletonVertex);
        }

        vertices.clear();
        vertices.shrink_to_fit();
    }
    else
    {
        vertexType = VertexType::Common;
    }

    uint32_t transformCount = anyMeshHasBones ? std::max(nextBoneId, 1u) : (scene->mNumMeshes * 2);
    transforms.resize(transformCount);
    auto *device = Graphics::GetDevice();
    transformBuffer = device->CreateBuffer(BufferType::Storage, transforms.size() * sizeof(Matrix4), MemoryType::Host, sizeof(Matrix4));
    if (transformBuffer)
    {
        transformBuffer->SetDebugName("MeshBoneMatrices");
    }

    rootNode = new BoneNode{};
    ReadAssimpNode(rootNode, scene->mRootNode);
    globalInverseTransform = Vector::Inverse(rootNode->Transform);
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
static void CopyAssimpAnimationKey(std::set<T> &dst, const U *src, uint32_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        T element{};
        if constexpr (IsPrimitiveOf<QuaternionKey, T>())
        {
            element.Value.w = src[i].mValue.w;
        }

        element.Time    = src[i].mTime;
        element.Value.x = src[i].mValue.x;
        element.Value.y = src[i].mValue.y;
        element.Value.z = src[i].mValue.z;

        dst.insert(std::move(element));
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
        animations[i].Duration        = pAnimation->mDuration;

        for (size_t j = 0; j < pAnimation->mNumChannels; j++)
        {
            auto pChannel = pAnimation->mChannels[j];
            AnimationNode node{};
            node.PreState  = (AnimationBehavior)pChannel->mPreState;
            node.PostState = (AnimationBehavior)pChannel->mPostState;
            CopyAssimpAnimationKey(node.PositionKeys, pChannel->mPositionKeys, pChannel->mNumPositionKeys);
            CopyAssimpAnimationKey(node.RotationKeys, pChannel->mRotationKeys, pChannel->mNumRotationKeys);
            CopyAssimpAnimationKey(node.ScalingKeys,  pChannel->mScalingKeys,  pChannel->mNumScalingKeys );
            animations[i].Nodes.insert({ pChannel->mNodeName.C_Str(), std::move(node) });
        }
    }
}
#endif

Ref<Mesh> Mesh::CreateSphere(float radius)
{
    std::vector<SimpleVertex> vertices;
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

            SimpleVertex vertex;
            vertex.Normal = { cosPhi * sinTheta, cosTheta, sinPhi * sinTheta };
            vertex.Position = { radius * vertex.Normal.x, radius * vertex.Normal.y, radius * vertex.Normal.z };
            /* Equirectangular-style UV (was uninitialized — garbage texcoords on D3D12 mesh path). */
            vertex.Texcoord = { longitude / longitudeBands, latitude / latitudeBands };
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

    return new Mesh(vertices, indices);
}

Ref<Mesh> Mesh::CreateCube(AsyncComputeThread *asyncComputeThread, CommandBuffer *commandBuffer, float size, bool rhcoords)
{
    constexpr uint32_t kFaceCount = 6;

    static const Vector3 kFaceNormals[kFaceCount] =
    {
        {  0,  0,  1},
        {  0,  0, -1},
        {  1,  0,  0},
        { -1,  0,  0},
        {  0,  1,  0},
        {  0, -1,  0},
    };

    static const Vector2 kTextureCoordinates[4] =
    {
        { 1, 0 },
        { 1, 1 },
        { 0, 1 },
        { 0, 0 },
    };

    std::vector<SimpleVertex> vertices;
    std::vector<Face> indices;

    size /= 2;

    static const Vector4 kIdentityR2 = {0.0f, 0.0f, 1.0f, 0.0f};
    static const Vector4 kIdentityR1 = {0.0f, 1.0f, 0.0f, 0.0f};
    for (int i = 0; i < kFaceCount; i++)
    {
        Vector3 normal = kFaceNormals[i];
        const Vector4 &basis = (i >= 4) ? kIdentityR2 : kIdentityR1;

        Vector3 side1 = Vector::Cross(normal, Vector3(basis));
        Vector3 side2 = Vector::Cross(normal, side1);

        uint32_t vbase = vertices.size();
        indices.push_back({vbase + 0, vbase + 1, vbase + 2});
        indices.push_back({vbase + 0, vbase + 2, vbase + 3});

        vertices.push_back(SimpleVertex((normal - side1 - side2) * size, normal, kTextureCoordinates[0]));
        vertices.push_back(SimpleVertex((normal - side1 + side2) * size, normal, kTextureCoordinates[1]));
        vertices.push_back(SimpleVertex((normal + side1 + side2) * size, normal, kTextureCoordinates[2]));
        vertices.push_back(SimpleVertex((normal + side1 - side2) * size, normal, kTextureCoordinates[3]));
    }

    return new Mesh(asyncComputeThread, commandBuffer, (void *) vertices.data(), vertices.size(), indices.data(), indices.size(), VertexType::Simple, "Cube");
}

Ref<Mesh> Mesh::CreatePlane(float size)
{
    float h = size * 0.5f;
    std::vector<SimpleVertex> vertices = {
        {{ -h, 0.0f,  h }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},
        {{  h, 0.0f,  h }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
        {{  h, 0.0f, -h }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},
        {{ -h, 0.0f, -h }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }},
    };
    std::vector<Face> indices = {
        { 0, 1, 2 },
        { 0, 2, 3 },
    };
    return new Mesh(vertices, indices);
}

Ref<Mesh> Mesh::CreateCube(float size)
{
    constexpr uint32_t kFaceCount = 6;
    static const Vector3 kFaceNormals[kFaceCount] = {
        {  0,  0,  1 }, {  0,  0, -1 },
        {  1,  0,  0 }, { -1,  0,  0 },
        {  0,  1,  0 }, {  0, -1,  0 },
    };
    static const Vector2 kTextureCoordinates[4] = {
        { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
    };

    std::vector<SimpleVertex> vertices;
    std::vector<Face> indices;
    float s = size * 0.5f;

    static const Vector4 kIdentityR2 = { 0.0f, 0.0f, 1.0f, 0.0f };
    static const Vector4 kIdentityR1 = { 0.0f, 1.0f, 0.0f, 0.0f };
    for (uint32_t i = 0; i < kFaceCount; i++)
    {
        Vector3 normal = kFaceNormals[i];
        const Vector4 &basis = (i >= 4) ? kIdentityR2 : kIdentityR1;
        Vector3 side1 = Vector::Cross(normal, Vector3(basis));
        Vector3 side2 = Vector::Cross(normal, side1);
        uint32_t vbase = (uint32_t)vertices.size();
        indices.push_back({ vbase + 0, vbase + 1, vbase + 2 });
        indices.push_back({ vbase + 0, vbase + 2, vbase + 3 });
        vertices.push_back(SimpleVertex{ (normal - side1 - side2) * s, normal, kTextureCoordinates[0] });
        vertices.push_back(SimpleVertex{ (normal - side1 + side2) * s, normal, kTextureCoordinates[1] });
        vertices.push_back(SimpleVertex{ (normal + side1 + side2) * s, normal, kTextureCoordinates[2] });
        vertices.push_back(SimpleVertex{ (normal + side1 - side2) * s, normal, kTextureCoordinates[3] });
    }
    return new Mesh(vertices, indices);
}

Ref<Mesh> Mesh::CreateCylinder(float radius, float height, uint32_t segments)
{
    std::vector<SimpleVertex> vertices;
    std::vector<Face> indices;
    float halfH = height * 0.5f;

    for (uint32_t i = 0; i <= segments; i++)
    {
        float angle = (float)i / segments * 2.0f * Math::PI;
        float x = Math::Cos(angle) * radius;
        float z = Math::Sin(angle) * radius;
        Vector3 n = Vector::Normalize(Vector3{ x, 0.0f, z });
        float u = (float)i / segments;
        vertices.push_back({{ x,  halfH, z }, n, { u, 0.0f }});
        vertices.push_back({{ x, -halfH, z }, n, { u, 1.0f }});
    }
    for (uint32_t i = 0; i < segments; i++)
    {
        uint32_t a = i * 2, b = a + 1, c = a + 2, d = a + 3;
        indices.push_back({ a, c, b });
        indices.push_back({ b, c, d });
    }

    uint32_t topCenter = (uint32_t)vertices.size();
    vertices.push_back({{ 0.0f, halfH, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.5f, 0.5f }});
    for (uint32_t i = 0; i <= segments; i++)
    {
        float angle = (float)i / segments * 2.0f * Math::PI;
        float x = Math::Cos(angle) * radius;
        float z = Math::Sin(angle) * radius;
        vertices.push_back({{ x, halfH, z }, { 0.0f, 1.0f, 0.0f }, { x / radius * 0.5f + 0.5f, z / radius * 0.5f + 0.5f }});
    }
    for (uint32_t i = 0; i < segments; i++)
    {
        indices.push_back({ topCenter, topCenter + 1 + i, topCenter + 2 + i });
    }

    uint32_t botCenter = (uint32_t)vertices.size();
    vertices.push_back({{ 0.0f, -halfH, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.5f }});
    for (uint32_t i = 0; i <= segments; i++)
    {
        float angle = (float)i / segments * 2.0f * Math::PI;
        float x = Math::Cos(angle) * radius;
        float z = Math::Sin(angle) * radius;
        vertices.push_back({{ x, -halfH, z }, { 0.0f, -1.0f, 0.0f }, { x / radius * 0.5f + 0.5f, z / radius * 0.5f + 0.5f }});
    }
    for (uint32_t i = 0; i < segments; i++)
    {
        indices.push_back({ botCenter, botCenter + 2 + i, botCenter + 1 + i });
    }

    return new Mesh(vertices, indices);
}

Ref<Mesh> Mesh::CreateCapsule(float radius, float cylinderHeight, uint32_t segments, uint32_t rings)
{
    std::vector<SimpleVertex> vertices;
    std::vector<Face> indices;
    float halfH = cylinderHeight * 0.5f;

    auto addVertex = [&](Vector3 pos, Vector3 normal, Vector2 uv) -> uint32_t {
        uint32_t idx = (uint32_t)vertices.size();
        vertices.push_back({ pos, normal, uv });
        return idx;
    };

    uint32_t topPole = addVertex({ 0.0f, halfH + radius, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.5f, 0.0f });
    for (uint32_t ring = 1; ring <= rings; ring++)
    {
        float phi = (float)ring / rings * (Math::PI * 0.5f);
        float y = Math::Cos(phi) * radius + halfH;
        float r = Math::Sin(phi) * radius;
        float v = (float)ring / (rings * 2 + 1);
        for (uint32_t seg = 0; seg <= segments; seg++)
        {
            float theta = (float)seg / segments * 2.0f * Math::PI;
            float x = Math::Cos(theta) * r;
            float z = Math::Sin(theta) * r;
            Vector3 n = Vector::Normalize(Vector3{ x, y - halfH, z });
            addVertex({ x, y, z }, n, { (float)seg / segments, v });
        }
    }

    uint32_t cylinderRings = std::max(1u, rings / 2);
    for (uint32_t ring = 0; ring <= cylinderRings; ring++)
    {
        float y = halfH - (float)ring / cylinderRings * cylinderHeight;
        float v = (float)(rings + ring) / (rings * 2 + 1);
        for (uint32_t seg = 0; seg <= segments; seg++)
        {
            float theta = (float)seg / segments * 2.0f * Math::PI;
            float x = Math::Cos(theta) * radius;
            float z = Math::Sin(theta) * radius;
            Vector3 n = Vector::Normalize(Vector3{ x, 0.0f, z });
            addVertex({ x, y, z }, n, { (float)seg / segments, v });
        }
    }

    for (uint32_t ring = 1; ring <= rings; ring++)
    {
        float phi = (float)ring / rings * (Math::PI * 0.5f);
        float y = -halfH - Math::Sin(phi) * radius;
        float r = Math::Cos(phi) * radius;
        float v = (float)(rings + cylinderRings + ring) / (rings * 2 + 1);
        for (uint32_t seg = 0; seg <= segments; seg++)
        {
            float theta = (float)seg / segments * 2.0f * Math::PI;
            float x = Math::Cos(theta) * r;
            float z = Math::Sin(theta) * r;
            Vector3 n = Vector::Normalize(Vector3{ x, y + halfH, z });
            addVertex({ x, y, z }, n, { (float)seg / segments, v });
        }
    }
    uint32_t botPole = addVertex({ 0.0f, -halfH - radius, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 1.0f });

    for (uint32_t seg = 0; seg < segments; seg++)
    {
        indices.push_back({ topPole, 1 + seg, 1 + seg + 1 });
    }

    uint32_t totalRings = rings + cylinderRings + 1 + rings;
    for (uint32_t ring = 0; ring < totalRings - 1; ring++)
    {
        uint32_t rowStart = 1 + ring * (segments + 1);
        uint32_t nextRow  = rowStart + (segments + 1);
        for (uint32_t seg = 0; seg < segments; seg++)
        {
            uint32_t a = rowStart + seg, b = rowStart + seg + 1;
            uint32_t c = nextRow + seg,  d = nextRow + seg + 1;
            indices.push_back({ a, c, b });
            indices.push_back({ b, c, d });
        }
    }

    uint32_t lastRowStart = botPole - (segments + 1);
    for (uint32_t seg = 0; seg < segments; seg++)
    {
        indices.push_back({ botPole, lastRowStart + seg + 1, lastRowStart + seg });
    }

    return new Mesh(vertices, indices);
}

Ref<Mesh> Mesh::CreateCone(float radius, float height, uint32_t segments)
{
    std::vector<SimpleVertex> vertices;
    std::vector<Face> indices;
    float halfH = height * 0.5f;
    float slopeLen = std::sqrt(radius * radius + height * height);
    float ny = radius / slopeLen;
    float nr = height / slopeLen;

    for (uint32_t i = 0; i <= segments; i++)
    {
        float angle = (float)i / segments * 2.0f * Math::PI;
        float cs = Math::Cos(angle), sn = Math::Sin(angle);
        Vector3 n = Vector::Normalize(Vector3{ cs * nr, ny, sn * nr });
        vertices.push_back({{ 0.0f, halfH, 0.0f }, n, { (float)i / segments, 0.0f }});
        vertices.push_back({{ cs * radius, -halfH, sn * radius }, n, { (float)i / segments, 1.0f }});
    }
    for (uint32_t i = 0; i < segments; i++)
    {
        uint32_t a = i * 2, b = a + 1, c = a + 2, d = a + 3;
        indices.push_back({ a, c, b });
        indices.push_back({ b, c, d });
    }

    uint32_t botCenter = (uint32_t)vertices.size();
    vertices.push_back({{ 0.0f, -halfH, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.5f }});
    for (uint32_t i = 0; i <= segments; i++)
    {
        float angle = (float)i / segments * 2.0f * Math::PI;
        float x = Math::Cos(angle) * radius;
        float z = Math::Sin(angle) * radius;
        vertices.push_back({{ x, -halfH, z }, { 0.0f, -1.0f, 0.0f }, { x / radius * 0.5f + 0.5f, z / radius * 0.5f + 0.5f }});
    }
    for (uint32_t i = 0; i < segments; i++)
    {
        indices.push_back({ botCenter, botCenter + 2 + i, botCenter + 1 + i });
    }

    return new Mesh(vertices, indices);
}

Ref<Mesh> Mesh::CreateTorus(float majorRadius, float minorRadius, uint32_t majorSegments, uint32_t minorSegments)
{
    std::vector<SimpleVertex> vertices;
    std::vector<Face> indices;

    for (uint32_t i = 0; i <= majorSegments; i++)
    {
        float u = (float)i / majorSegments * 2.0f * Math::PI;
        float cu = Math::Cos(u), su = Math::Sin(u);
        for (uint32_t j = 0; j <= minorSegments; j++)
        {
            float v = (float)j / minorSegments * 2.0f * Math::PI;
            float cv = Math::Cos(v), sv = Math::Sin(v);
            float x = (majorRadius + minorRadius * cv) * cu;
            float y = minorRadius * sv;
            float z = (majorRadius + minorRadius * cv) * su;
            Vector3 center = { majorRadius * cu, 0.0f, majorRadius * su };
            Vector3 pos = { x, y, z };
            Vector3 n = Vector::Normalize(pos - center);
            vertices.push_back({ pos, n, { (float)i / majorSegments, (float)j / minorSegments }});
        }
    }

    for (uint32_t i = 0; i < majorSegments; i++)
    {
        for (uint32_t j = 0; j < minorSegments; j++)
        {
            uint32_t a = i * (minorSegments + 1) + j;
            uint32_t b = a + 1;
            uint32_t c = a + (minorSegments + 1);
            uint32_t d = c + 1;
            indices.push_back({ a, c, b });
            indices.push_back({ b, c, d });
        }
    }

    return new Mesh(vertices, indices);
}

namespace
{

static void CopyMaterialToReference(const Material &src, MaterialComponent::Reference &dst)
{
    dst.Name = src.Name;
    dst.AlbedoColor = src.AlbedoColor;
    dst.Specular = src.Specular;
    dst.Ambient = src.Ambient;
    dst.Emissive = src.Emissive;
    dst.Metallic = src.Metallic;
    dst.Roughness = src.Roughness;
    dst.Opacity = src.Opacity;
    dst.Textures.Albedo = src.Textures.Albedo;
    dst.Textures.Normal = src.Textures.Normal;
    dst.Textures.Specular = src.Textures.Specular;
    dst.Textures.Metallic = src.Textures.Metallic;
    dst.Textures.Roughness = src.Textures.Roughness;
    dst.Textures.AmbientOcclusion = src.Textures.AmbientOcclusion;
    dst.Pathes.Diffuse = src.Pathes.Diffuse;
    dst.Pathes.Normal = src.Pathes.Normal;
    dst.Pathes.Specular = src.Pathes.Specular;
    dst.Pathes.Metallic = src.Pathes.Metallic;
    dst.Pathes.Roughness = src.Pathes.Roughness;
    dst.Pathes.AmbientOcclusion = src.Pathes.AmbientOcclusion;
}

}

void Mesh::PopulateMaterialComponent(MaterialComponent &material) const
{
    if (nodes.empty())
    {
        material.References.clear();
        return;
    }
    material.References.resize(nodes.size());
    if (materials.empty())
    {
        return;
    }
    for (size_t i = 0; i < nodes.size(); i++)
    {
        uint32_t mi = nodes[i].MaterialIndex;
        if (mi >= materials.size())
        {
            mi = 0;
        }
        CopyMaterialToReference(materials[mi], material.References[i]);
    }
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
