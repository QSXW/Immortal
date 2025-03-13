#include "MeshletGenerator.h"
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <cstdint>
#include <meshoptimizer.h>

namespace Immortal
{

namespace
{
struct EdgeEntry
{
	uint32_t i0;
	uint32_t i1;
	uint32_t i2;

	uint32_t Face;
	EdgeEntry *Next;
};

size_t CRCHash(const uint32_t *dwords, uint32_t dwordCount)
{
	size_t h = 0;

	for (uint32_t i = 0; i < dwordCount; ++i)
	{
		uint32_t highOrd = h & 0xf8000000;
		h = h << 5;
		h = h ^ (highOrd >> 27);
		h = h ^ size_t(dwords[i]);
	}

	return h;
}

template <class T>
struct hash
{

};

template <>
struct hash<Vector3>
{
	size_t operator()(const Vector3 &v) const
	{
		return CRCHash(reinterpret_cast<const uint32_t *>(&v), sizeof(v) / 4);
	}
};

template <typename T>
inline size_t Hash(const T &val)
{
	return hash<T>()(val);
}

}

namespace internal
{
template <typename T>
void BuildAdjacencyList(
    const T *indices, uint32_t indexCount,
    const Vector3 *positions, uint32_t vertexCount,
    uint32_t *adjacency);
}

void BuildAdjacencyList(
    const uint16_t *indices, uint32_t indexCount,
    const Vector3 *positions, uint32_t vertexCount,
    uint32_t *adjacency)
{
	internal::BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency);
}

void BuildAdjacencyList(
    const uint32_t *indices, uint32_t indexCount,
    const Vector3 *positions, uint32_t vertexCount,
    uint32_t *adjacency)
{
	internal::BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency);
}

///
// Implementation

template <typename T>
void internal::BuildAdjacencyList(
    const T *indices, uint32_t indexCount,
    const Vector3 *positions, uint32_t vertexCount,
    uint32_t *adjacency)
{
	const uint32_t triCount = indexCount / 3;
	// Find point reps (unique positions) in the position stream
	// Create a mapping of non-unique vertex indices to point reps
	std::vector<T> pointRep;
	pointRep.resize(vertexCount);

	std::unordered_map<size_t, T> uniquePositionMap;
	uniquePositionMap.reserve(vertexCount);

	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		Vector3 position = *(positions + i);
		size_t hash = Hash(position);

		auto it = uniquePositionMap.find(hash);
		if (it != uniquePositionMap.end())
		{
			// Position already encountered - reference previous index
			pointRep[i] = it->second;
		}
		else
		{
			// New position found - add to hash table and LUT
			uniquePositionMap.insert(std::make_pair(hash, static_cast<T>(i)));
			pointRep[i] = static_cast<T>(i);
		}
	}

	// Create a linked list of edges for each vertex to determine adjacency
	const uint32_t hashSize = vertexCount / 3;

	std::unique_ptr<EdgeEntry *[]> hashTable(new EdgeEntry *[hashSize]);
	std::unique_ptr<EdgeEntry[]> entries(new EdgeEntry[triCount * 3]);

	std::memset(hashTable.get(), 0, sizeof(EdgeEntry *) * hashSize);
	uint32_t entryIndex = 0;

	for (uint32_t iFace = 0; iFace < triCount; ++iFace)
	{
		uint32_t index = iFace * 3;

		// Create a hash entry in the hash table for each each.
		for (uint32_t iEdge = 0; iEdge < 3; ++iEdge)
		{
			T i0 = pointRep[indices[index + (iEdge % 3)]];
			T i1 = pointRep[indices[index + ((iEdge + 1) % 3)]];
			T i2 = pointRep[indices[index + ((iEdge + 2) % 3)]];

			auto &entry = entries[entryIndex++];
			entry.i0 = i0;
			entry.i1 = i1;
			entry.i2 = i2;

			uint32_t key = entry.i0 % hashSize;

			entry.Next = hashTable[key];
			entry.Face = iFace;

			hashTable[key] = &entry;
		}
	}

	// Initialize the adjacency list
	std::memset(adjacency, uint32_t(-1), indexCount * sizeof(uint32_t));

	for (uint32_t iFace = 0; iFace < triCount; ++iFace)
	{
		uint32_t index = iFace * 3;

		for (uint32_t point = 0; point < 3; ++point)
		{
			if (adjacency[iFace * 3 + point] != uint32_t(-1))
				continue;

			// Look for edges directed in the opposite direction.
			T i0 = pointRep[indices[index + ((point + 1) % 3)]];
			T i1 = pointRep[indices[index + (point % 3)]];
			T i2 = pointRep[indices[index + ((point + 2) % 3)]];

			// Find a face sharing this edge
			uint32_t key = i0 % hashSize;

			EdgeEntry *found = nullptr;
			EdgeEntry *foundPrev = nullptr;

			for (EdgeEntry *current = hashTable[key], *prev = nullptr; current != nullptr; prev = current, current = current->Next)
			{
				if (current->i1 == i1 && current->i0 == i0)
				{
					found = current;
					foundPrev = prev;
					break;
				}
			}

			// Cache this face's normal
			Vector3 n0;
			{
				Vector3 p0 = Vector3(positions[i1]);
				Vector3 p1 = Vector3(positions[i0]);
				Vector3 p2 = Vector3(positions[i2]);

				Vector3 e0 = p0 - p1;
				Vector3 e1 = p1 - p2;

				n0 = Vector::Normalize(Vector::Cross(e0, e1));
			}

			// Use face normal dot product to determine best edge-sharing candidate.
			float bestDot = -2.0f;
			for (EdgeEntry *current = found, *prev = foundPrev; current != nullptr; prev = current, current = current->Next)
			{
				if (bestDot == -2.0f || (current->i1 == i1 && current->i0 == i0))
				{
					Vector3 p0 = Vector3(positions[current->i0]);
					Vector3 p1 = Vector3(positions[current->i1]);
					Vector3 p2 = Vector3(positions[current->i2]);

					Vector3 e0 = p0 - p1;
					Vector3 e1 = p1 - p2;

					Vector3 n1 = Vector::Normalize(Vector::Cross(e0, e1));

					float dot = Vector::Dot(n0, n1);

					if (dot > bestDot)
					{
						found = current;
						foundPrev = prev;
						bestDot = dot;
					}
				}
			}

			// Update hash table and adjacency list
			if (found && found->Face != uint32_t(-1))
			{
				// Erase the found from the hash table linked list.
				if (foundPrev != nullptr)
				{
					foundPrev->Next = found->Next;
				}
				else
				{
					hashTable[key] = found->Next;
				}

				// Update adjacency information
				adjacency[iFace * 3 + point] = found->Face;

				// Search & remove this face from the table linked list
				uint32_t key2 = i1 % hashSize;

				for (EdgeEntry *current = hashTable[key2], *prev = nullptr; current != nullptr; prev = current, current = current->Next)
				{
					if (current->Face == iFace && current->i1 == i0 && current->i0 == i1)
					{
						if (prev != nullptr)
						{
							prev->Next = current->Next;
						}
						else
						{
							hashTable[key2] = current->Next;
						}

						break;
					}
				}

				bool linked = false;
				for (uint32_t point2 = 0; point2 < point; ++point2)
				{
					if (found->Face == adjacency[iFace * 3 + point2])
					{
						linked = true;
						adjacency[iFace * 3 + point] = uint32_t(-1);
						break;
					}
				}

				if (!linked)
				{
					uint32_t edge2 = 0;
					for (; edge2 < 3; ++edge2)
					{
						T k = indices[found->Face * 3 + edge2];
						if (k == uint32_t(-1))
							continue;

						if (pointRep[k] == i0)
							break;
					}

					if (edge2 < 3)
					{
						adjacency[found->Face * 3 + edge2] = iFace;
					}
				}
			}
		}
	}
}

Vector4 MinimumBoundingSphere(Vector3 *points, uint32_t count)
{
	assert(points != nullptr && count != 0);

	// Find the min & max points indices along each axis.
	uint32_t minAxis[3] = {0, 0, 0};
	uint32_t maxAxis[3] = {0, 0, 0};

	for (uint32_t i = 1; i < count; ++i)
	{
		float *point = (float *) (points + i);

		for (uint32_t j = 0; j < 3; ++j)
		{
			float *min = (float *) (&points[minAxis[j]]);
			float *max = (float *) (&points[maxAxis[j]]);

			minAxis[j] = point[j] < min[j] ? i : minAxis[j];
			maxAxis[j] = point[j] > max[j] ? i : maxAxis[j];
		}
	}

	// Find axis with maximum span.
	Vector3 distSqMax = 0;
	uint32_t axis = 0;

	for (uint32_t i = 0; i < 3u; ++i)
	{
		Vector3 min = Vector3(points[minAxis[i]]);
		Vector3 max = Vector3(points[maxAxis[i]]);

		Vector3 distSq = Vector3(Vector::Length2(max - min));
		if (Vector::Greater(distSq, distSqMax))
		{
			distSqMax = distSq;
			axis = i;
		}
	}

	// Calculate an initial starting center point & radius.
	Vector3 p1 = Vector3(points[minAxis[axis]]);
	Vector3 p2 = Vector3(points[maxAxis[axis]]);

	Vector3 center = (p1 + p2) * 0.5f;
	float radius = Vector::Length(p2 - p1) * 0.5f;
	float radiusSq = radius * radius;

	// Add all our points to bounding sphere expanding radius & recalculating center point as necessary.
	for (uint32_t i = 0; i < count; ++i)
	{
		Vector3 point = Vector3(points[i]);
		float distSq = Vector::Length2(point - center);

		if (distSq > radiusSq)
		{
			float dist = std::sqrt(distSq);
			float k = (radius / dist) * 0.5f + 0.5f;

			center = center * k + point * (1.0f - k);
			radius = (radius + dist) * 0.5f;
		}
	}

	// Populate a single XMVECTOR with center & radius data.
	return Vector4(center, radius);
}

template <typename T>
struct InlineMeshlet
{
	struct PackedTriangle
	{
		uint32_t i0 : 10;
		uint32_t i1 : 10;
		uint32_t i2 : 10;
		uint32_t spare : 2;
	};

	std::vector<T> UniqueVertexIndices;
	std::vector<PackedTriangle> PrimitiveIndices;
};

namespace internal
{
    template <typename T>
    void Meshletize(
        uint32_t maxVerts, uint32_t maxPrims,
        const T* indices, uint32_t indexCount,
        const Vector3 *positions, uint32_t vertexCount,
        std::vector<InlineMeshlet<T>> &output);
}

void Meshletize(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint16_t *indices, uint32_t indexCount,
    const Vector3 *positions, uint32_t vertexCount,
    std::vector<InlineMeshlet<uint16_t>> &output
)
{
    return internal::Meshletize(maxVerts, maxPrims, indices, indexCount, positions, vertexCount, output);
}

void Meshletize(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint32_t *indices, uint32_t indexCount,
    const Vector3 *positions, uint32_t vertexCount,
    std::vector<InlineMeshlet<uint32_t>> &output
)
{
    return internal::Meshletize(maxVerts, maxPrims, indices, indexCount, positions, vertexCount, output);
}

bool CompareScores(const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b)
{
    return a.second > b.second;
}

Vector3 ComputeNormal(const Vector3 *tri)
{
    Vector3 p0 = tri[0];
    Vector3 p1 = tri[1];
    Vector3 p2 = tri[2];

    Vector3 v01 = p0 - p1;
	Vector3 v02 = p0 - p2;

    return Vector::Normalize(Vector::Cross(v01, v02));
}

// Compute number of triangle vertices already exist in the meshlet
template <typename T>
uint32_t ComputeReuse(const InlineMeshlet<T> &meshlet, T (&triIndices)[3])
{
    uint32_t count = 0;

    for (uint32_t i = 0; i < static_cast<uint32_t>(meshlet.UniqueVertexIndices.size()); ++i)
    {
        for (uint32_t j = 0; j < 3u; ++j)
        {
            if (meshlet.UniqueVertexIndices[i] == triIndices[j])
            {
                ++count;
            }
        }
    }

    return count;
}

// Computes a candidacy score based on spatial locality, orientational coherence, and vertex re-use within a meshlet.
template <typename T>
float ComputeScore(const InlineMeshlet<T> &meshlet, const Vector4 &sphere, const Vector4 &normal, T (&triIndices)[3], Vector3 *triVerts)
{
    const float reuseWeight = 0.334f;
    const float locWeight = 0.333f;
    const float oriWeight = 0.333f;
    
    // Vertex reuse
    uint32_t reuse = ComputeReuse(meshlet, triIndices);
	float reuseScore = 1.0f - (float(reuse) / 3.0f);

    // Distance from center point
	float maxSq = 0;
    for (uint32_t i = 0; i < 3u; ++i)
    {
		Vector3 v = Vector3(sphere) - Vector3(triVerts[i]);
        maxSq = std::max(maxSq, Vector::Dot(v, v));
    }

    float r  = sphere.w;
    float r2 = r * r;
	float locScore = Vector::Log(maxSq / r2 + 1.0f);

    // Angle between normal and meshlet cone axis
	Vector3 n = ComputeNormal(triVerts);
	float d = Vector::Dot(n, Vector3(normal));
	float oriScore = (-d + 1.0f) / 2.0f;

    float b = reuseWeight * reuseScore + locWeight * locScore + oriWeight * oriScore;

    return b;
}

// Determines whether a candidate triangle can be added to a specific meshlet; if it can, does so.
template <typename T>
bool AddToMeshlet(uint32_t maxVerts, uint32_t maxPrims, InlineMeshlet<T> &meshlet, T (&tri)[3])
{
    // Are we already full of vertices?
    if (meshlet.UniqueVertexIndices.size() == maxVerts)
        return false;

    // Are we full, or can we store an additional primitive?
    if (meshlet.PrimitiveIndices.size() == maxPrims)
        return false;

    static const uint32_t Undef = uint32_t(-1);
    uint32_t indices[3] = { Undef, Undef, Undef };
    uint32_t newCount = 3;

    for (uint32_t i = 0; i < meshlet.UniqueVertexIndices.size(); ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            if (meshlet.UniqueVertexIndices[i] == tri[j])
            {
                indices[j] = i;
                --newCount;
            }
        }
    }

    // Will this triangle fit?
    if (meshlet.UniqueVertexIndices.size() + newCount > maxVerts)
        return false;

    // Add unique vertex indices to unique vertex index list
    for (uint32_t j = 0; j < 3; ++j)
    {
        if (indices[j] == Undef)
        {
            indices[j] = static_cast<uint32_t>(meshlet.UniqueVertexIndices.size());
            meshlet.UniqueVertexIndices.push_back(tri[j]);
        }
    }

    // Add the new primitive 
    typename InlineMeshlet<T>::PackedTriangle prim = {};
    prim.i0 = indices[0];
    prim.i1 = indices[1];
    prim.i2 = indices[2];

    meshlet.PrimitiveIndices.push_back(prim);

    return true;
}

template <typename T>
bool IsMeshletFull(uint32_t maxVerts, uint32_t maxPrims, const InlineMeshlet<T>& meshlet)
{
    assert(meshlet.UniqueVertexIndices.size() <= maxVerts);
    assert(meshlet.PrimitiveIndices.size() <= maxPrims);

    return meshlet.UniqueVertexIndices.size() == maxVerts
        || meshlet.PrimitiveIndices.size() == maxPrims;
}

///
// Implementation 

template <typename T>
void internal::Meshletize(
    uint32_t maxVerts, uint32_t maxPrims,
    const T *indices, uint32_t indexCount,
    const Vector3 *positions, uint32_t vertexCount,
    std::vector<InlineMeshlet<T>>& output
)
{
    const uint32_t triCount = indexCount / 3;

    // Build a primitive adjacency list
    std::vector<uint32_t> adjacency;
    adjacency.resize(indexCount);

    BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency.data());

    // Rest our outputs
    output.clear();
    output.emplace_back();
    auto* curr = &output.back();

    // Bitmask of all triangles in mesh to determine whether a specific one has been added.
    std::vector<bool> checklist;
    checklist.resize(triCount);

    std::vector<Vector3> m_positions;
	std::vector<Vector3> normals;
    std::vector<std::pair<uint32_t, float>> candidates;
    std::unordered_set<uint32_t> candidateCheck;

    Vector4 psphere, normal;

    // Arbitrarily start at triangle zero.
    uint32_t triIndex = 0;
    candidates.push_back(std::make_pair(triIndex, 0.0f));
    candidateCheck.insert(triIndex);
    
    // Continue adding triangles until 
    while (!candidates.empty())
    {
        uint32_t index = candidates.back().first;
        candidates.pop_back();

        T tri[3] =
        {
            indices[index * 3],
            indices[index * 3 + 1],
            indices[index * 3 + 2],
        };

        assert(tri[0] < vertexCount);
        assert(tri[1] < vertexCount);
        assert(tri[2] < vertexCount);

        // Try to add triangle to meshlet
        if (AddToMeshlet(maxVerts, maxPrims, *curr, tri))
        {
            // Success! Mark as added.
            checklist[index] = true;

            // Add m_positions & normal to list
			Vector3 points[3] =
            {
                positions[tri[0]],
                positions[tri[1]],
                positions[tri[2]],
            };

            m_positions.push_back(points[0]);
            m_positions.push_back(points[1]);
            m_positions.push_back(points[2]);

            Vector3 Normal = ComputeNormal(points);
            normals.push_back(Normal);

            // Compute new bounding sphere & normal axis
            psphere = MinimumBoundingSphere(m_positions.data(), static_cast<uint32_t>(m_positions.size()));
            
            Vector4 nsphere = MinimumBoundingSphere(normals.data(), static_cast<uint32_t>(normals.size()));
            normal = Vector::Normalize(nsphere);

            // Find and add all applicable adjacent triangles to candidate list
            const uint32_t adjIndex = index * 3;

            uint32_t adj[3] =
            {
                adjacency[adjIndex],
                adjacency[adjIndex + 1],
                adjacency[adjIndex + 2],
            };

            for (uint32_t i = 0; i < 3u; ++i)
            {
                // Invalid triangle in adjacency slot
                if (adj[i] == -1)
                    continue;
                
                // Already processed triangle
                if (checklist[adj[i]])
                    continue;

                // Triangle already in the candidate list
                if (candidateCheck.count(adj[i]))
                    continue;

                candidates.push_back(std::make_pair(adj[i], FLT_MAX));
                candidateCheck.insert(adj[i]);
            }

            // Re-score remaining candidate triangles
            for (uint32_t i = 0; i < static_cast<uint32_t>(candidates.size()); ++i)
            {
                uint32_t candidate = candidates[i].first;

                T triIndices[3] =
                {
                    indices[candidate * 3],
                    indices[candidate * 3 + 1],
                    indices[candidate * 3 + 2],
                };

                assert(triIndices[0] < vertexCount);
                assert(triIndices[1] < vertexCount);
                assert(triIndices[2] < vertexCount);

                Vector3 triVerts[3] =
                {
                    positions[triIndices[0]],
                    positions[triIndices[1]],
                    positions[triIndices[2]],
                };

                candidates[i].second = ComputeScore(*curr, psphere, normal, triIndices, triVerts);
            }

            // Determine whether we need to move to the next meshlet.
            if (IsMeshletFull(maxVerts, maxPrims, *curr))
            {
                m_positions.clear();
                normals.clear();
                candidateCheck.clear();

                // Use one of our existing candidates as the next meshlet seed.
                if (!candidates.empty())
                {
                    candidates[0] = candidates.back();
                    candidates.resize(1);
                    candidateCheck.insert(candidates[0].first);
                }

                output.emplace_back();
                curr = &output.back();
            }
            else
            {
                std::sort(candidates.begin(), candidates.end(), &CompareScores);
            }
        }
        else
        {
            if (candidates.empty())
            {
                m_positions.clear();
                normals.clear();
                candidateCheck.clear();

                output.emplace_back();
                curr = &output.back();
            }
        }

        // Ran out of candidates; add a new seed candidate to start the next meshlet.
        if (candidates.empty())
        {
            while (triIndex < triCount && checklist[triIndex])
                ++triIndex;

            if (triIndex == triCount)
                break;

            candidates.push_back(std::make_pair(triIndex, 0.0f));
            candidateCheck.insert(triIndex);
        }
    }

    // The last meshlet may have never had any primitives added to it - in which case we want to remove it.
    if (output.back().PrimitiveIndices.empty())
    {
        output.pop_back();
    }
}

template <class T>
inline T QuantizeSNorm(T value)
{
	return (Vector::Clamp(value, T(-1.0f), T(1.0f)) * 0.5f + 0.5f) * 255.0f;
}

template <class T>
inline T QuantizeUNorm(T value)
{
	return Vector::Clamp(value, T(0.0f), T(1.0f)) * 255.0f;
}

namespace internal
{
template <typename T>
HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const T *indices, uint32_t indexCount,
    const Subset *indexSubsets, uint32_t subsetCount,
    const Vector3 *positions, uint32_t vertexCount,
    std::vector<Subset> &meshletSubsets,
    std::vector<Meshlet> &meshlets,
    std::vector<uint8_t> &uniqueVertexIndices,
    std::vector<PackedTriangle> &primitiveIndices);

template <typename T>
HRESULT ComputeCullData(
    const Vector3 *positions, uint32_t vertexCount,
    const Meshlet* meshlets, uint32_t meshletCount,
    const T* uniqueVertexIndices,
    const PackedTriangle* primitiveIndices,
    DWORD flags,
    CullData* cullData
);
}

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint16_t *indices, uint32_t indexCount,
    const Subset *indexSubsets, uint32_t subsetCount,
    const Vector3 *positions, uint32_t vertexCount,
    std::vector<Subset> &meshletSubsets,
    std::vector<Meshlet> &meshlets,
    std::vector<uint8_t> &uniqueVertexIndices,
    std::vector<PackedTriangle> &primitiveIndices)
{
    return internal::ComputeMeshlets(maxVerts, maxPrims, indices, indexCount, indexSubsets, subsetCount, positions, vertexCount, meshletSubsets, meshlets, uniqueVertexIndices, primitiveIndices);
}

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint32_t *indices, uint32_t indexCount,
    const Subset *indexSubsets, uint32_t subsetCount,
    const Vector3 *positions, uint32_t vertexCount,
    std::vector<Subset> &meshletSubsets,
    std::vector<Meshlet> &meshlets,
    std::vector<uint8_t> &uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices)
{
    return internal::ComputeMeshlets(maxVerts, maxPrims, indices, indexCount, indexSubsets, subsetCount, positions, vertexCount, meshletSubsets, meshlets, uniqueVertexIndices, primitiveIndices);
}

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint16_t *indices, uint32_t indexCount,
    const Vector3 *positions, uint32_t vertexCount,
    std::vector<Subset> &meshletSubsets,
    std::vector<Meshlet> &meshlets,
    std::vector<uint8_t> &uniqueVertexIndices,
    std::vector<PackedTriangle> &primitiveIndices)
{
    Subset s = { 0, indexCount };
    return internal::ComputeMeshlets(maxVerts, maxPrims, indices, indexCount, &s, 1u, positions, vertexCount, meshletSubsets, meshlets, uniqueVertexIndices, primitiveIndices);
}

HRESULT ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint32_t *indices, uint32_t indexCount,
    const Vector3 *positions, uint32_t vertexCount,
    std::vector<Subset> &meshletSubsets,
    std::vector<Meshlet> &meshlets,
    std::vector<uint8_t> &uniqueVertexIndices,
    std::vector<PackedTriangle> &primitiveIndices)
{
    Subset s = { 0, indexCount };
    return internal::ComputeMeshlets(maxVerts, maxPrims, indices, indexCount, &s, 1u, positions, vertexCount, meshletSubsets, meshlets, uniqueVertexIndices, primitiveIndices);
}

HRESULT ComputeCullData(
    const Vector3 *positions, uint32_t vertexCount,
    const Meshlet *meshlets, uint32_t meshletCount,
    const uint16_t *uniqueVertexIndices,
    const PackedTriangle* primitiveIndices,
    DWORD flags,
    CullData* cullData
)
{
    return internal::ComputeCullData(positions, vertexCount, meshlets, meshletCount, uniqueVertexIndices, primitiveIndices, flags, cullData);
}

HRESULT ComputeCullData(
    const Vector3 *positions, uint32_t vertexCount,
    const Meshlet *meshlets, uint32_t meshletCount,
    const uint32_t *uniqueVertexIndices,
    const PackedTriangle *primitiveIndices,
    DWORD flags,
    CullData* cullData
)
{
    return internal::ComputeCullData(positions, vertexCount, meshlets, meshletCount, uniqueVertexIndices, primitiveIndices, flags, cullData);
}


template <typename T>
HRESULT internal::ComputeMeshlets(
    uint32_t maxVerts, uint32_t maxPrims,
    const T *indices, uint32_t indexCount,
    const Subset *indexSubsets, uint32_t subsetCount,
    const Vector3 *positions, uint32_t vertexCount,
    std::vector<Subset> &meshletSubsets,
    std::vector<Meshlet> &meshlets,
    std::vector<uint8_t> &uniqueVertexIndices,
    std::vector<PackedTriangle>& primitiveIndices)
{
	(void)indexCount;

    for (uint32_t i = 0; i < subsetCount; ++i)
    {
        Subset s = indexSubsets[i];

        assert(s.Offset + s.Count <= indexCount);

        std::vector<InlineMeshlet<T>> builtMeshlets;
        Meshletize(maxVerts, maxPrims, indices + s.Offset, s.Count, positions, vertexCount, builtMeshlets);

        Subset meshletSubset;
        meshletSubset.Offset = static_cast<uint32_t>(meshlets.size());
        meshletSubset.Count = static_cast<uint32_t>(builtMeshlets.size());
        meshletSubsets.push_back(meshletSubset);

        // Determine final unique vertex index and primitive index counts & offsets.
        uint32_t startVertCount = static_cast<uint32_t>(uniqueVertexIndices.size()) / sizeof(T);
        uint32_t startPrimCount = static_cast<uint32_t>(primitiveIndices.size());

        uint32_t uniqueVertexIndexCount = startVertCount;
        uint32_t primitiveIndexCount = startPrimCount;

        // Resize the meshlet output array to hold the newly formed meshlets.
        uint32_t meshletCount = static_cast<uint32_t>(meshlets.size());
        meshlets.resize(meshletCount + builtMeshlets.size());

        for (uint32_t j = 0, dest = meshletCount; j < static_cast<uint32_t>(builtMeshlets.size()); ++j, ++dest)
        {
            meshlets[dest].VertOffset = uniqueVertexIndexCount;
            meshlets[dest].VertCount = static_cast<uint32_t>(builtMeshlets[j].UniqueVertexIndices.size());
            uniqueVertexIndexCount += static_cast<uint32_t>(builtMeshlets[j].UniqueVertexIndices.size());

            meshlets[dest].PrimOffset = primitiveIndexCount;
            meshlets[dest].PrimCount = static_cast<uint32_t>(builtMeshlets[j].PrimitiveIndices.size());
            primitiveIndexCount += static_cast<uint32_t>(builtMeshlets[j].PrimitiveIndices.size());
        }

        // Allocate space for the new data.
        uniqueVertexIndices.resize(uniqueVertexIndexCount * sizeof(T));
        primitiveIndices.resize(primitiveIndexCount);

        // Copy data from the freshly built meshlets into the output buffers.
        auto vertDest = reinterpret_cast<T*>(uniqueVertexIndices.data()) + startVertCount;
        auto primDest = reinterpret_cast<uint32_t*>(primitiveIndices.data()) + startPrimCount;

        for (uint32_t j = 0; j < static_cast<uint32_t>(builtMeshlets.size()); ++j)
        {
            std::memcpy(vertDest, builtMeshlets[j].UniqueVertexIndices.data(), builtMeshlets[j].UniqueVertexIndices.size() * sizeof(T));
            std::memcpy(primDest, builtMeshlets[j].PrimitiveIndices.data(), builtMeshlets[j].PrimitiveIndices.size() * sizeof(uint32_t));

            vertDest += builtMeshlets[j].UniqueVertexIndices.size();
            primDest += builtMeshlets[j].PrimitiveIndices.size();
        }
    }

    return S_OK;
}

//
// Strongly influenced by https://github.com/zeux/meshoptimizer - Thanks amigo!
//

template <typename T>
HRESULT internal::ComputeCullData(
    const Vector3 *positions, uint32_t vertexCount,
    const Meshlet *meshlets, uint32_t meshletCount,
    const T *uniqueVertexIndices,
    const PackedTriangle *primitiveIndices,
    DWORD flags,
    CullData* cullData
)
{
    Vector3 vertices[256];
    Vector3 normals[256];

    for (uint32_t mi = 0; mi < meshletCount; ++mi)
    {
        auto& m = meshlets[mi];
        auto& c = cullData[mi];

        // Cache vertices
        for (uint32_t i = 0; i < m.VertCount; ++i)
        {
            uint32_t vIndex = uniqueVertexIndices[m.VertOffset + i];

            assert(vIndex < vertexCount);
            vertices[i] = positions[vIndex];
        }

        // Generate primitive normals & cache
        for (uint32_t i = 0; i < m.PrimCount; ++i)
        {
            auto primitive = primitiveIndices[m.PrimOffset + i];

            Vector3 triangle[3]
            {
			    Vector3(vertices[primitive.indices.i0]),
			    Vector3(vertices[primitive.indices.i1]),
			    Vector3(vertices[primitive.indices.i2]),
            };

            Vector3 p10 = triangle[1] - triangle[0];
			Vector3 p20 = triangle[2] - triangle[0];
			Vector3 n = Vector::Normalize(Vector::Cross(p10, p20));

            normals[i] = (flags & CNORM_WIND_CW) != 0 ? -n : n;
        }

        // Calculate spatial bounds
		Vector4 positionBounds = MinimumBoundingSphere(vertices, m.VertCount);
        c.BoundingSphere = positionBounds;

        // Calculate the normal cone
        // 1. Normalized center point of minimum bounding sphere of unit normals == conic axis
		Vector4 normalBounds = MinimumBoundingSphere(normals, m.PrimCount);

        // 2. Calculate dot product of all normals to conic axis, selecting minimum
		Vector3 axis = Vector::Normalize(Vector3(normalBounds));

        float minDot = 1.0f;
        for (uint32_t i = 0; i < m.PrimCount; ++i)
        {
            float dot = Vector::Dot(axis, normals[i]);
            minDot = std::min(minDot, dot);
        }

        if (minDot < 0.1f)
        {
            // Degenerate cone
            c.NormalCone[0] = 127;
            c.NormalCone[1] = 127;
            c.NormalCone[2] = 127;
            c.NormalCone[3] = 255;
            continue;
        }

        // Find the point on center-t*axis ray that lies in negative half-space of all triangles
        float maxt = 0;

        for (uint32_t i = 0; i < m.PrimCount; ++i)
        {
            auto primitive = primitiveIndices[m.PrimOffset + i];

            uint32_t indices[3]
            {
                primitive.indices.i0,
                primitive.indices.i1,
                primitive.indices.i2,
            };

            Vector3 triangle[3]
            {
                vertices[indices[0]],
                vertices[indices[1]],
                vertices[indices[2]],
            };

            Vector3 c = Vector3(positionBounds) - triangle[0];

            Vector3 n = normals[i];
            float dc = Vector::Dot(c, n);
            float dn = Vector::Dot(axis, n);

            // dn should be larger than mindp cutoff above
            assert(dn > 0.0f);
            float t = dc / dn;

            maxt = (t > maxt) ? t : maxt;
        }

        // cone apex should be in the negative half-space of all cluster triangles by construction
        c.ApexOffset = maxt;

        // cos(a) for normal cone is minDot; we need to add 90 degrees on both sides and invert the cone
        // which gives us -cos(a+90) = -(-sin(a)) = sin(a) = sqrt(1 - cos^2(a))
        float coneCutoff = std::sqrt(1.0f - minDot * minDot);

        // 3. Quantize to uint8
		Vector3 quantized = QuantizeSNorm(axis);
        c.NormalCone[0] = (uint8_t)quantized.x;
        c.NormalCone[1] = (uint8_t)quantized.y;
        c.NormalCone[2] = (uint8_t)quantized.z;

        float error =  Vector::Length((quantized / 127.0f) - axis);
        quantized = QuantizeUNorm(Vector3(coneCutoff + error));
        c.NormalCone[3] = (uint8_t)quantized.x;
    }

    return S_OK;
}

MeshletGenerator::MeshletGenerator(const MeshletOptions &options) :
    options{options}
{

}

void MeshletGenerator::Generate(const std::vector<Mesh::CommonVertex> &vertices, const std::vector<Mesh::Face> &indices)
{
	const uint32_t triCount    = (uint32_t) indices.size();
	const uint32_t indexCount  = triCount * 3;
	const uint32_t vertexCount = (uint32_t)vertices.size();

    positionReorder.resize(vertexCount);
	indexReorder.resize(indexCount * sizeof(uint32_t));

	faceRemap.resize(triCount);
	vertexRemap.resize(vertexCount);

    std::vector<Vector3> positions;
	positions.resize(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++)
	{
		positions[i] = vertices[i].Position;
    }

    indexSubsets.resize(1);
	indexSubsets[0].Offset = 0;
	indexSubsets[0].Count  = indexCount;

	//ComputeMeshlets(
	//    options.maxVertices,
 //       options.maxPrimitives,
 //       (const uint32_t *)indices.data(),
 //       indexCount,
	//    indexSubsets.data(),
 //       (uint32_t)indexSubsets.size(),
	//    positions.data(),
 //       (uint32_t)positions.size(),
	//    meshletSubsets,
	//    meshlets,
	//    uniqueVertexIndices,
	//    primitiveIndices);

 //   cullData.resize(meshlets.size());
	//ComputeCullData(
	//    positions.data(),
 //       (uint32_t)(positions.size()),
	//    meshlets.data(), 
 //       (uint32_t)(meshlets.size()),
	//    (uint32_t *)(uniqueVertexIndices.data()),
	//    primitiveIndices.data(),
	//    0,
	//    cullData.data()
 //   );
}

void MeshletGenerator::BuildMeshlets(const void *vertices, uint32_t numVertices, uint32_t vertexStride, const uint32_t *indices, uint32_t numIndices)
{
	//std::vector<Vector3> positions;
	//positions.resize(vertices.size());
	//for (size_t i = 0; i < vertices.size(); i++)
	//{
	//	positions[i] = vertices[i].Position;
	//}

	const uint32_t indexCount = numIndices;
    const size_t kMaxVertices  = 64;
	const size_t kMaxTriangles = 124;
	const float kConeWeight    = 0.0f;

    const size_t maxMeshlets = meshopt_buildMeshletsBound(indexCount, kMaxVertices, kMaxTriangles);
	std::vector<uint8_t>         meshletTriangles;

	meshlets.resize(maxMeshlets);
	meshletVertices.resize(maxMeshlets * kMaxVertices);
	meshletTriangles.resize(maxMeshlets * kMaxTriangles * 3);

    size_t meshletCount = meshopt_buildMeshlets(
	    meshlets.data(),
	    meshletVertices.data(),
	    meshletTriangles.data(),
	    indices,
	    indexCount,
	    reinterpret_cast<const float *>(vertices),
	    numVertices,
	    vertexStride,
	    kMaxVertices,
	    kMaxTriangles,
	    kConeWeight
    );

    auto &last = meshlets[meshletCount - 1];
	meshletVertices.resize(last.vertex_offset + last.vertex_count);
	meshletTriangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
	meshlets.resize(meshletCount);

    meshletTrianglesU32.reserve(meshlets.back().triangle_count * meshlets.size());
	for (auto &m : meshlets)
	{
		// Save triangle offset for current meshlet
		uint32_t triangleOffset = static_cast<uint32_t>(meshletTrianglesU32.size());

		// Repack to uint32_t
		for (uint32_t i = 0; i < m.triangle_count; ++i)
		{
			uint32_t i0 = 3 * i + 0 + m.triangle_offset;
			uint32_t i1 = 3 * i + 1 + m.triangle_offset;
			uint32_t i2 = 3 * i + 2 + m.triangle_offset;

			uint8_t vIdx0 = meshletTriangles[i0];
			uint8_t vIdx1 = meshletTriangles[i1];
			uint8_t vIdx2 = meshletTriangles[i2];
			uint32_t packed = ((static_cast<uint32_t>(vIdx0) & 0xFF) << 0) |
			                  ((static_cast<uint32_t>(vIdx1) & 0xFF) << 8) |
			                  ((static_cast<uint32_t>(vIdx2) & 0xFF) << 16);
			meshletTrianglesU32.push_back(packed);
		}

		// Update triangle offset for current meshlet
		m.triangle_offset = triangleOffset;
	}
}

}
 