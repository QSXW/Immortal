#pragma once

#include "Core.h"
#include "Config.h"

#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Algorithm/LightVector.h"
#include "Math/Vector.h"

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

struct Animation
{
    std::string Name;
    float TicksPerSeconds = 25.0f;
    float Duration = 0;
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

        uint32_t MaterialIndex = 0;
        bool Animated = false;
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

    void CalculatedBoneTransform(float timeInSeconds, const Matrix4 &parentTransform);

private:
    void LoadModelData(const aiScene *scene);

    void LoadAnimationData(const aiScene *scene);

    bool LoadBoneData(const aiMesh *mesh, std::vector<SkeletonVertex> &vertex, uint32_t baseVertex, uint32_t &numBones);

    void ReadAssimpNode(BoneNode *boneNode, const aiNode *src);

private:
    MonoRef<Buffer> buffer;

    std::string path;

    std::vector<Node> nodes;

    std::unordered_map<std::string, BoneInfo> bones;

    MonoRef<BoneNode> rootNode;

    std::vector<Matrix4> transforms;

    Ref<Buffer> transformBuffer;

    std::vector<Animation> animations;

    std::unordered_map<std::string, AnimationNode> animationNodes;

    Matrix4 globalInverseTransform;

    struct
    {
        uint32_t currentAnimation = 0;
    } state;
};

}
