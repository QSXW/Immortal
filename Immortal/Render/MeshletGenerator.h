#pragma once

#include <cstdint>
#include "Mesh.h"
#include <meshoptimizer.h>

namespace Immortal
{

enum MeshletGeneratorFlags : uint32_t
{
	CNORM_WIND_CW = 0x4
};

struct MeshletOptions
{
	uint32_t maxVertices;
	uint32_t maxPrimitives;
	float    unitScale;
	bool     flip;

	MeshletOptions(void) :
	    maxVertices(64), 
		maxPrimitives(126),
		unitScale(1.0f),
		flip(false)
	{
	
	}
};

class MeshletGenerator
{
public:
	MeshletGenerator(const MeshletOptions &options = {});

	void Generate(const std::vector<Mesh::CommonVertex> &vertices, const std::vector<Mesh::Face> &indices);

	void BuildMeshlets(const void *vertices, uint32_t numVertices, uint32_t vertexStride, const uint32_t *indices, uint32_t numIndices);

public:
	MeshletOptions options;
	std::vector<Subset>                  indexSubsets;
	//std::vector<Meshlet>                 meshlets;
	std::vector<Subset>                  meshletSubsets;
	std::vector<uint8_t>                 uniqueVertexIndices;
	std::vector<PackedTriangle>          primitiveIndices;
	std::vector<CullData>                cullData;
	std::vector<uint32_t>                faceRemap;
	std::vector<uint32_t>                vertexRemap;
	std::vector<uint8_t>                 indexReorder;
	std::vector<uint32_t>                dupVerts;
	std::vector<Vector3>                 positionReorder;
	std::vector<Vector3>                 normalReorder;
	std::vector<Vector2>                 uvReorder;
	std::unordered_map<size_t, uint32_t> uniqueVertices;

	std::vector<meshopt_Meshlet> meshlets;
	std::vector<uint32_t> meshletVertices;
	std::vector<uint32_t> meshletTrianglesU32;
};

}
