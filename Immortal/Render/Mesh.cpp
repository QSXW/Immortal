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
	namespace fs = std::filesystem;
	fs::path raw{ std::string(fromAssimp) };
	if (raw.is_absolute())
	{
		std::error_code ec;
		if (fs::exists(raw))
		{
			fs::path c = fs::weakly_canonical(raw, ec);
			return ec ? raw.string() : c.string();
		}
		return raw.string();
	}

	fs::path base = fs::path(modelFilepath).parent_path();
	fs::path combined = (base / raw).lexically_normal();
	std::error_code ec;
	if (fs::exists(combined))
	{
		fs::path c = fs::weakly_canonical(combined, ec);
		return ec ? combined.string() : c.string();
	}
	return combined.string();
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
    path{ filepath }
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
		asyncComputeThread->Execute<RecordingTask>([&, this] (uint64_t, CommandBuffer *commandBuffer) {
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
	std::vector<Face> faces;
	std::vector<BufferBindInfo> vertexBindInfo;
	std::vector<BufferBindInfo> indexBindInfo;

	LoadModelData(scene, vertices, faces, vertexBindInfo, indexBindInfo);

    for (size_t i = 0; i < nodes.size(); i++)
	{
		auto &node = nodes[i];
		auto numVertices = vertexBindInfo[i].size / sizeof(vertices[0]);
		auto startVertices = vertexBindInfo[i].offset / sizeof(vertices[0]);
		auto numIndicies = indexBindInfo[i].size / sizeof(uint32_t);
		auto startIndices = indexBindInfo[i].offset / sizeof(uint32_t);

		std::vector<Vector3> positions;
		positions.resize(numVertices);
		for (size_t i = 0; i < numVertices; i++)
		{
			positions[i] = vertices[startVertices + i].Position;
		}

		size_t vertexStride = sizeof(CommonVertex);
		auto pVertex = &vertices[startVertices];
		const uint32_t *indices = (uint32_t *)faces.data();
		MeshletGenerator generator;
		generator.BuildMeshlets(pVertex, numVertices, vertexStride, &indices[startIndices], numIndicies);

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

		asyncComputeThread->Execute<RecordingTask>([=, this] (uint64_t, CommandBuffer *commandBuffer) {
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
			aiMaterial->Get(AI_MATKEY_SHININESS,       material.Roughness  );
			aiMaterial->Get(AI_MATKEY_OPACITY,         material.Opacity    );

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
#endif
}

Mesh::Mesh(const std::vector<SimpleVertex> &vertices, const std::vector<Index> &indicies) :
    vertexType{ VertexType::Simple }
{
    Node head{ "Undefined" };

    head.Vertex = Graphics::CreateBuffer(Buffer::Type::Vertex, sizeof(SimpleVertex) * vertices.size(), vertices.data());
	head.Index  = Graphics::CreateBuffer(Buffer::Type::Index,   sizeof(Index)       * indicies.size(), indicies.data());

    nodes.emplace_back(head);
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

	asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t, CommandBuffer *commandBuffer) {
		auto &h = nodes.back();
		TransferBuffer2Device(commandBuffer, h.Vertex, stagingVertex, pVertex, vertexBufferSize);
		TransferBuffer2Device(commandBuffer, h.Index,  stagingIndex, pIndex, indexBufferSize);
	});
    asyncComputeThread->Execute<ExecutionCompletedTask>([=] {
		Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingVertex);
		Graphics::ReleaseCachedBuffer(BufferType::TransferSource, stagingIndex);
	});


}

void Mesh::ReadHierarchyBoneNode(float animationTime, const BoneNode *node, const Matrix4 &parentTransform)
{
    Matrix4 globalTransform{};
    Matrix4 nodeTransform = node->Transform;

    auto &animationsNode = animations[state.currentAnimation].Nodes;   
    if (auto it = animationsNode.find(node->Name); it != animationsNode.end())
    {
        AnimationNode &node = it->second;
        Vector3 position    = Interpolate<Vector3, VectorKey>(node.PositionKeys, animationTime);
        Vector3 scaling     = Interpolate<Vector3, VectorKey>(node.ScalingKeys, animationTime);
        Quaternion rotation = Interpolate<Quaternion, QuaternionKey>(node.RotationKeys, animationTime);

        nodeTransform = Vector::Translate(position) * Vector::ToMatrix4(rotation) * Vector::Scale(scaling);
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
    //transformBuffer->Update(transforms);
}

#if HAVE_ASSIMP
void Mesh::LoadModelData(const aiScene *scene, std::vector<CommonVertex> &vertices, std::vector<Face> &faces, std::vector<BufferBindInfo> &vertexBindInfo, std::vector<BufferBindInfo> &indexBindInfo)
{
    uint32_t numBones = scene->mNumMeshes;
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
		vertexBindInfo[i].size = mesh->mNumVertices * sizeof(vertices[0]);

        //uint32_t baseVertex = vertexBindInfo[i].offset / sizeof(vertices[0]);
        //bool hasBone = LoadBoneData(mesh, vertices, baseVertex, numBones);

        //if (!hasBone && scene->HasAnimations())
        //{
        //    for (size_t j = baseVertex; j < vertices.size(); j++)
        //    {
        //        vertices[j].BoneIds[0] = i;
        //        vertices[j].Weights[0] = 1.0f;
        //    }
        //}

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

    transforms.resize(numBones + scene->mNumMeshes);
	transformBuffer = Graphics::GetDevice()->CreateBuffer(Buffer::Type::ConstantBuffer, transforms.size() * sizeof(Matrix4), MemoryType::Device, Format::Matric4);

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

std::shared_ptr<Mesh> Mesh::CreateSphere(float radius)
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
