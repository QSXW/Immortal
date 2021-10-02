#pragma once

#include "ImmortalCore.h"
#include "Camera.h"
#include "OrthographicCamera.h"
#include "RenderContext.h"
#include "Renderer.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

namespace Immortal
{

struct RenderData
{
    Ref<ShaderMap>   ShaderLibrary;
    Ref<Texture2D>   BlackTexture;
    Ref<Texture2D>   TransparentTexture;
    Ref<Texture2D>   WhiteTexture;
    Ref<VertexArray> FullScreenVertexArray;
};

class Render
{
public:
    enum class Type
    {
        None,
        Vulkan,
        OpenGL,
        D3D12
    };

    static inline const char Stringify[4][8]
    {
        { "None"   },
        { "Vulkan" },
        { "OpenGL" },
        { "D3D12"  }
    };

    enum class ShaderName : INT32
    {
        Texture,
        PBR,
        Skybox,
        Renderer2D,
        Tonemap,
        Test,
        Last
    };

    static std::vector<Ref<Immortal::Shader>> ShaderContainer;

    static const std::string Render::ShaderProfiles[];

    template <class T, ShaderName U>
    static constexpr inline Ref<Shader> Get()
    {
        static_assert(typeof<T, Shader>() && "No suitable Type for getter");
        if constexpr (typeof<T, Shader>())
        {
            static_assert(U < ShaderName::Last, "Shader Index out of bound.");
            return ShaderContainer[ncast<INT32>(U)];
        }
    }

    template <class T>
    static inline Ref<Shader> Get(const ShaderName index)
    {
        if constexpr (typeof<T, Shader>())
        {
            size_t i = static_cast<size_t>(index);
            SLASSERT(i < ShaderContainer.size() && "Shader Index out of bound.");
            return ShaderContainer[i];
        }
        static_assert(false, "No suitable Type for getter");
    }

public:
    static void INIT(RenderContext *context);

    static void Set(Type type)
    {
        API = type;
    }

    static void OnWindowResize(UINT32 width, UINT32 height)
    {
        handle->OnResize(0, 0, width, height);
    }

    static void EnableDepthTest()
    {
        handle->EnableDepthTest();
    }

    static void DisableDepthTest()
    {
        handle->DisableDepthTest();
    }

    static void SetClearColor(Color &color)
    {
        handle->SetClearColor(color);
    }

    static void Clear(Color &color = Color{ 0.0f })
    {
        handle->SetClearColor(color);
        handle->Clear();
    }

    static void DrawIndexed(Ref<VertexArray> &vao, UINT32 indexCount)
    {
        handle->DrawIndexed(vao, indexCount);
    }

    static void Render::Begin(OrthographicCamera &camera)
    {
        scene.viewProjectionMatrix = camera.ViewPorjectionMatrix();
    }

    static void Render::End()
    {
        
    }

    static RenderData *Data()
    {
        return &data;
    }

    static uint32_t CurrentPresentedFrameIndex()
    {
        return handle->Index();
    }

    static void Submit(const Ref<Shader> &shader, const Ref<VertexArray> &vertexArray, const Matrix4& transform = Matrix4{ 1.0f });

    static void Submit(const Ref<Shader> &shader, const Ref<Mesh> &mesh, const Matrix4 &transform = Matrix4{ 1.0f });

    static void SwapBuffers()
    {
        handle->SwapBuffers();
    }

    static void RenderFrame()
    {
        handle->RenderFrame();
    }

    static const char *Api()
    {
        return Stringify[ncast<int>(API)];
    }

private:
    static std::unique_ptr<Renderer> handle;

    struct Scene
    {
        Matrix4 viewProjectionMatrix;
    };

    static RenderData data;
    static Scene scene;

public:
    static inline Type API{ Type::None };
};

using ShaderName = Render::ShaderName;

template <class SuperType, class OPENGL, class VULKAN, class D3D12, class ... Args>
inline constexpr Ref<SuperType> CreateSuper(Args&& ... args)
{
    switch (Render::API)
    {
    case Render::Type::OpenGL:
        return CreateRef<OPENGL>(std::forward<Args>(args)...);
    case Render::Type::Vulkan:
        return CreateRef<VULKAN>(std::forward<Args>(args)...);
    case Render::Type::D3D12:
        return CreateRef<D3D12>(std::forward<Args>(args)...);
    default:
        SLASSERT(false && "Render::None is currently not supported!");
        return nullptr;
    }
}

}
