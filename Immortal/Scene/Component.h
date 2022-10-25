/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include <queue>

#include "Core.h"
#include "Framework/Async.h"
#include "Render/Render.h"
#include "Render/Mesh.h"
#include "Render/Texture.h"
#include "SceneCamera.h"
#include "Vision/Interface/Codec.h"
#include "Vision/Interface/Demuxer.h"

namespace Immortal
{

class Scene;
struct Component
{
    enum class Type
    {
        None,
        Camera,
        ColorMixing,
        DirectionalLight,
        Filter,
        ID,
        Light,
        Mesh,
        Meta,
        Material,
        NativeScript,
        Script,
        Scene,
        SpriteRenderer,
        Tag,
        Transform,
        VideoPlayer
    };

    Component()
    {

    }
};

#define DEFINE_COMP_TYPE(T) static Component::Type GetType() { return Component::Type::T; }

struct IDComponent : public Component
{
    DEFINE_COMP_TYPE(ID)

    IDComponent(uint64_t id = 0) :
        uid(id)
    {

    }

    uint64_t uid;
};

struct TagComponent : public Component
{
    DEFINE_COMP_TYPE(Tag)

    TagComponent(const std::string &tag = "") :
        Tag{ tag }
    {

    }

    std::string Tag;
};

struct TransformComponent : public Component
{
    DEFINE_COMP_TYPE(Transform)

    void Set(Vector3 position, Vector3 rotation, Vector3 scale)
    {
        Position = position;
        Rotation = rotation;
        Scale    = scale;
    }

    Matrix4 Transform() const
    {
        return Vector::Translate(Position) * Vector::Rotate(Rotation) * Vector::Scale(Scale);
    }

    operator Matrix4() const
    {
        return Transform();
    }

    static inline Vector3 Up{ 0.0f, 1.0f, 0.0f };

    static inline Vector3 Right{ 1.0f, 0.0f, 0.0f };

    static inline Vector3 Forward{ 0.0f, 0.0f, -1.0f };

    Vector3 Position{ 0.0f, 0.0f, 0.0f };

    Vector3 Rotation{ 0.0f, 0.0f, 0.0f };

    Vector3 Scale{ 1.0f, 1.0f, 1.0f };
};

struct MeshComponent : public Component
{
    DEFINE_COMP_TYPE(Mesh)

    MeshComponent()
    {

    }

    MeshComponent(std::shared_ptr<Immortal::Mesh> mesh) :
        Mesh{ mesh }
    {

    }

    operator std::shared_ptr<Immortal::Mesh>()
    {
        return Mesh;
    }

    MeshComponent &operator=(const MeshComponent &other)
    {
        Mesh = other.Mesh;
        return *this;
    }

    std::shared_ptr<Immortal::Mesh> Mesh;
};

struct MaterialComponent : public Component
{
    DEFINE_COMP_TYPE(Material)

    MaterialComponent()
    {
        References.resize(1);
    }

    struct Reference
    {
        Reference() :
            AlbedoColor{ 0.995f, 0.995f, 0.995f, 1.0f },
            Metallic{ 1.0f },
            Roughness{ 1.0f }
        {
            Textures.Albedo = Render::Preset()->Textures.White;
            Textures.Normal = Render::Preset()->Textures.Normal;
            Textures.Metallic = Textures.Albedo;
            Textures.Roughness = Textures.Albedo;
            Textures.AO = Textures.Albedo;
        }

        struct {
            Ref<Texture> Albedo;
            Ref<Texture> Normal;
            Ref<Texture> Metallic;
            Ref<Texture> Roughness;
            Ref<Texture> AO;
        } Textures;

        Vector4 AlbedoColor;
        float   Metallic;
        float   Roughness;
    };

    std::vector<Reference> References;
};

struct LightComponent : public Component
{
    DEFINE_COMP_TYPE(Light)

    Vector4 Radiance{ 1.0f };
    bool Enabled = true;
};

struct SceneComponent : public Component
{
    DEFINE_COMP_TYPE(Scene)
};

struct SpriteRendererComponent : public Component
{
    DEFINE_COMP_TYPE(SpriteRenderer)

    static inline Texture::Description Desc = {
        Format::RGBA8,
        Wrap::Repeat,
        Filter::Linear
    };

    struct Extension : public IObject
    {
        Ref<Image> input[3];

        Format chromaFormat;
    };

    SpriteRendererComponent()
    {

    }

    void UpdateSprite(const Vision::Picture &picture);

    SpriteRendererComponent(const SpriteRendererComponent &other) = default;

    Ref<Texture> Sprite;

    Ref<Texture> Result = Sprite;

    Ref<Extension> pNext;

    Vector4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

    float TilingFactor = 1.0f;
};

struct CameraComponent : public Component
{
    DEFINE_COMP_TYPE(Camera)

    CameraComponent()
    {

    }

    CameraComponent(const CameraComponent &other) = default;

    operator SceneCamera&()
    {
        return Camera;
    }

    operator const SceneCamera&() const
    {
        return Camera;
    }

    SceneCamera Camera;

    bool Primary = false;
};

struct DirectionalLightComponent : public Component
{
    DEFINE_COMP_TYPE(DirectionalLight)

    Vector3 Radiance{ 1.0f, 1.0f, 1.0f };

    float Intensity   = 1.0f;
    bool  CastShadows = true;
    bool  SoftShadows = true;
    float LightSize   = 0.5f; // For PCSS
};

struct ScriptComponent : public Component
{
    DEFINE_COMP_TYPE(Script)

    ScriptComponent() :
        path{},
        vtable{},
        object{}
    {

    }

    ScriptComponent(const ScriptComponent &other) = default;

    ScriptComponent(const std::string &path) :
        vtable{},
        object{},
        path{ path }
    {

    }

    void Init(int id, Scene *scene);

    void Update(int id, Scene *scene, float deltaTime);

    void OnKeyDown(int id, Scene *scene, int keyCode);

    struct {
        Anonymous __setId;
        Anonymous __update;
        Anonymous __onKeyDown;
    } vtable;

    Anonymous object;

    std::string path;
    std::string className;
};

struct MetaComponent : public Component
{
    DEFINE_COMP_TYPE(Meta)

    void *Meta = nullptr;
};

struct ColorMixingComponent : public Component
{
    DEFINE_COMP_TYPE(ColorMixing)

    bool Modified    = false;
    bool Initialized = false;

    Vector4 RGBA = Vector4{ 0, 0, 0, 1.0f };
    Vector4 HSL;

    struct {
        float ColorTemperature;
        float Hue;
    } WhiteBalance{ 0, 0 };

    struct {
        float White;
        float Black;
    } Gradation{ 0, 0 };

    float Exposure    = 0;
    float Contrast    = 0;
    float Hightlights = 0;
    float Shadow      = 0;
    float Vividness   = 0;

    static size_t Length;
};

inline size_t ColorMixingComponent::Length = sizeof(ColorMixingComponent) - offsetof(ColorMixingComponent, RGBA);

enum class FilterType
{
    None,
    GaussianBlur,
    AverageBlur,
    DCT
};

struct FilterComponent : public Component
{
    DEFINE_COMP_TYPE(Filter)

    FilterComponent()
    {

    }

    std::vector<FilterType> Filter;
};

struct VideoPlayerContext
{
public:
	VideoPlayerContext(Ref<Vision::Interface::Demuxer> demuxer, Ref<Vision::VideoCodec> decoder, Ref<Vision::VideoCodec> audioDecoder = nullptr);

	~VideoPlayerContext();

	VideoPlayerContext(const VideoPlayerContext &&other) = delete;

	VideoPlayerContext &operator=(const VideoPlayerContext &&other) = delete;

	Picture GetPicture() const
	{
		return pictures.empty() ? Vision::Picture{} : pictures.front();
	}
	
    Picture GetAudioFrame() const
    {
        return audioFrames.empty() ? Vision::Picture{} : audioFrames.front();
    }

	void PopPicture()
	{
		std::unique_lock lock{mutex};
		pictures.pop();
	}

    void PopAudioFrame()
    {
        std::unique_lock lock{ mutex };
        audioFrames.pop();
    }

	const std::string &GetSource() const
	{
		return demuxer->GetSource();
	}

public:
	URef<Thread> demuxerThread;

    URef<Thread> videoDecodeThread;

    URef<Thread> audioDecodeThread;

	std::mutex mutex;

	Ref<Vision::VideoCodec> decoder;

    Ref<Vision::VideoCodec> audioDecoder;

	Ref<Vision::Interface::Demuxer> demuxer;

	std::queue<Picture> pictures;

	std::queue<Picture> audioFrames;

    std::array<std::queue<Vision::CodedFrame>, 2> queues; 

	Picture lastPicture;

	struct State
	{
		bool playing = false;
		bool exited = false;
	} state;
};

struct VideoPlayerComponent : public Component
{
    DEFINE_COMP_TYPE(VideoPlayer)

    VideoPlayerComponent(Ref<Vision::Interface::Demuxer> demuxer, Ref<Vision::VideoCodec> decoder, Ref<Vision::VideoCodec> audioDecoder = nullptr);

    ~VideoPlayerComponent();

    VideoPlayerComponent(VideoPlayerComponent &&other)
    {
        Swap(other);
    }

    Picture GetPicture();

    Picture GetAudioFrame();

    void PopPicture();

    void PopAudioFrame();

    VideoPlayerComponent &operator=(VideoPlayerComponent &&other)
    {
        Swap(other);
        return *this;
    }

    void Swap(VideoPlayerComponent &other)
    {
		player.Swap(other.player);
    }

    Animator *GetAnimator() const;

    const std::string &GetSource() const;

public:
	URef<VideoPlayerContext> player;
};

}
