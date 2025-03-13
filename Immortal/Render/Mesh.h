#pragma once

#include "Core.h"
#include "Config.h"

#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Algorithm/LightVector.h"
#include "Math/Vector.h"
#include "Material.h"

#include <cmath>
#include <vector>
#include <set>
#include <unordered_map>

struct aiScene;
struct aiMesh;
struct aiNode;
namespace Immortal
{

struct SkeletonVertex
{
    Vector3  Position;
    Vector3  Normal;
    Vector3  Tangent;
    Vector2  Texcoord;
    uint32_t BoneIds[4];
    Vector4  Weights;

    void AddBone(uint32_t id, float weight);
};

template <class T>
struct TemporalKey
{
    TemporalKey() :
        Time{},
        Value{}
    {}

    TemporalKey(float time) :
        Time{ (double)time }
    {}

    double Time;
    T Value;

    bool operator==(const TemporalKey &other) const
    {
        return Time == other.Time;
    }

    bool operator<(const TemporalKey &other) const
    {
        return Time < other.Time;
    }

    bool operator>(const TemporalKey &other) const
    {
        return Time > other.Time;
    }
};

using VectorKey = TemporalKey<Vector3>;

using QuaternionKey = TemporalKey<Quaternion>;

enum AnimationBehavior : uint32_t
{
    /** The value from the default node transformation is taken*/
    AnimationBehavior_Default = 0x0,

    /** The nearest key value is used without interpolation */
    AnimationBehavior_Constant = 0x1,

    /** The value of the nearest two keys is linearly
        *  extrapolated for the current time value.*/
    AnimationBehavior_Linear = 0x2,

    /** The animation is repeated.
     *
     *  If the animation key go from n to m and the current
     *  time is t, use the value at (t-n) % (|m-n|).*/
     AnimationBehavior_Repeat = 0x3,
};

struct AnimationNode
{
    /** The rotation keys of this animation channel. Rotations are
     *  given as quaternions,  which are 4D vectors. The array is
     *  mNumRotationKeys in size.
     *
     * If there are rotation keys, there will also be at least one
     * scaling and one position key. */
    std::set<VectorKey> PositionKeys;

    /** The rotation keys of this animation channel. Rotations are
     *  given as quaternions,  which are 4D vectors. The array is
     *  mNumRotationKeys in size.
     *
     * If there are rotation keys, there will also be at least one
     * scaling and one position key. */
    std::set<QuaternionKey> RotationKeys;

    /** The scaling keys of this animation channel. Scalings are
     *  specified as 3D vector. The array is mNumScalingKeys in size.
     *
     * If there are scaling keys, there will also be at least one
     * position and one rotation key.*/
    std::set<VectorKey> ScalingKeys;

    /** Defines how the animation behaves before the first
     *  key is encountered.
     *
     *  The default value is aiAnimBehaviour_DEFAULT (the original
     *  transformation matrix of the affected node is used).*/
    AnimationBehavior PreState;

    /** Defines how the animation behaves after the last
     *  key was processed.
     *
     *  The default value is aiAnimBehaviour_DEFAULT (the original
     *  transformation matrix of the affected node is taken).*/
    AnimationBehavior PostState;
};

struct Animation
{
    std::string Name;
    std::unordered_map<std::string, AnimationNode> Nodes;

    float Timestamp;
    float TicksPerSeconds = 25.0f;
    float Duration = 0;

    void Ticks(float deltaTime)
    {
        Timestamp += deltaTime * TicksPerSeconds;
        Timestamp = fmodf(Timestamp, Duration);
    }
};

struct BoneInfo
{
    BoneInfo() :
        Id{},
        OffsetMatrix{}
    {}

    BoneInfo(uint32_t id, const Matrix4 &matrix) :
        Id{ id },
        OffsetMatrix{ matrix }
    {

    }

    uint32_t Id;
    Matrix4 OffsetMatrix;
};

struct BoneNode
{
public:
    BoneNode() :
        Name{},
        Transform{ 1.0f },
        Parent{},
        Children{},
        Meshes{}
    {}

    ~BoneNode()
    {

    }

public:
    std::string Name;
    Matrix4 Transform;

    BoneNode *Parent;
    LightVector<BoneNode> Children;
    LightVector<uint32_t> Meshes;
};

struct Meshlet
{
	uint32_t VertCount;
	uint32_t VertOffset;
	uint32_t PrimCount;
	uint32_t PrimOffset;
};

struct Subset
{
	uint32_t Offset;
	uint32_t Count;
};

union PackedTriangle
{
	struct
	{
		uint32_t i0 : 10;
		uint32_t i1 : 10;
		uint32_t i2 : 10;
		uint32_t _unused : 2;
	} indices;
	uint32_t packed;
};

struct CullData
{
	Vector4 BoundingSphere;        // xyz = center, w = radius
	uint8_t NormalCone[4];         // xyz = axis, w = sin(a + 90)
	float ApexOffset;              // apex = center - axis * offset
};

class Mesh : public IObject
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
    static inline std::shared_ptr<Mesh> Get()
    {
        return Primitives[static_cast<uint32_t>(I)];
    }

    static void LoadPrimitives();

    static std::shared_ptr<Mesh> CreateSphere(float radius);

    static Ref<Mesh> CreateCube(AsyncComputeThread *asyncComputeThread, CommandBuffer *commandBuffer, float size, bool rhcoords);

public:
    enum class VertexType
    {
        Simple,
        Common,
        Skeleton
    };

    struct DirectXSampleVertex
    {
		Vector3 Position;
		Vector3 Normal;
    };

    struct SimpleVertex
    {
		Vector3 Position;
		Vector3 Normal;
		Vector2 Texcoord;
    };

    struct CommonVertex
    {
        Vector3 Position;
        Vector3 Normal;
        Vector3 Tangent;
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
        Ref<Buffer> Vertex;
        Ref<Buffer> Index;
		Ref<Buffer> Meshlets;
		Ref<Buffer> UniqueVertexIndices;
		Ref<Buffer> PrimitiveIndices;
		uint32_t MeshletSubsetCount;
        uint32_t MaterialIndex = 0;
		Ref<DescriptorSet> descriptorSet;
        bool Animated = false;
    };

    using Index = Face;

public:
	Mesh(AsyncComputeThread *asyncComputeThread, CommandBuffer *commandBuffer, const std::string &filepath);

    Mesh(const std::vector<SimpleVertex> &vertices, const std::vector<Index> &indicies);

    Mesh(AsyncComputeThread *asyncComputeThread, CommandBuffer *commandBuffer, const void *pVertex, size_t numVertex, const Index *pIndex, size_t numIndex, VertexType type, const std::string &name = "Untitled");

    ~Mesh() { }

    const std::string &Source() const
    {
        return path;
    }

    std::vector<Node> &NodeList()
    {
        return nodes;
    }

    BoneNode *GetRootNode()
    {
        return rootNode;
    }

    size_t Size() const
    {
        return nodes.size();
    }

    std::vector<Animation> &GetAnimation()
    {
        return animations;
    }

    Ref<Buffer> GetTransforms() const
    {
        return transformBuffer;
    }

    bool IsAnimated() const
    {
        return !animations.empty();
    }

    uint32_t GetAnimationState() const;

    void SwitchToAnimation(uint32_t index);

    void ReadHierarchyBoneNode(float animationTime, const BoneNode *node, const Matrix4 &parentTransform);

    void CalculatedBoneTransform(const Matrix4 &parentTransform);

private:
	void LoadModelData(const aiScene *scene, std::vector<CommonVertex> &vertices, std::vector<Face> &faces, std::vector <BufferBindInfo> &vertexBindInfo, std::vector<BufferBindInfo> &indexBindInfo);

    void LoadAnimationData(const aiScene *scene);

    bool LoadBoneData(const aiMesh *mesh, std::vector<SkeletonVertex> &vertex, uint32_t baseVertex, uint32_t &numBones);

    void ReadAssimpNode(BoneNode *boneNode, const aiNode *src);

private:
	VertexType vertexType;

    std::vector<Material> materials;

    URef<Buffer> buffer;

    std::string path;

    std::vector<Node> nodes;

    URef<Buffer> meshletBuffer;

    std::unordered_map<std::string, BoneInfo> bones;

    URef<BoneNode> rootNode;

    std::vector<Matrix4> transforms;

    Ref<Buffer> transformBuffer;

    std::vector<Animation> animations;

    Matrix4 globalInverseTransform;

    struct
    {
        uint32_t currentAnimation = 0;
    } state;
};

}
