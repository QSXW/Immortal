#pragma once

#include "Render/Graphics.h"

#include <type_traits>
#include <utility>

namespace Immortal
{

template <class T, class = void>
struct GObjectType
{
    static_assert(!sizeof(T), "Unsupported graphics object type.");
};

#define IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(Type, TypeEnum)                                      \
template <class T>                                                                                \
struct GObjectType<T, std::enable_if_t<std::is_base_of_v<Type, std::remove_cv_t<T>>>>              \
{                                                                                                  \
    static constexpr ObjectType Value = ObjectType::TypeEnum;                                      \
}

IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(AccelerationStructure, AccelerationStructure);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(RenderTarget,           RenderTarget);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(Texture,                Texture);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(Buffer,                 Buffer);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(BufferView,             BufferView);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(CommandBuffer,          CommandBuffer);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(DescriptorSet,          DescriptorSet);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(GPUEvent,               GPUEvent);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(Pipeline,               Pipeline);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(PipelineCache,          PipelineCache);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(Queue,                  Queue);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(Sampler,                Sampler);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(Shader,                 Shader);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(Swapchain,              Swapchain);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(Window,                 Window);
IMMORTAL_GOBJECT_TYPE_SPECIALIZATION(WindowCapture,          WindowCapture);

#undef IMMORTAL_GOBJECT_TYPE_SPECIALIZATION

template <class T>
class GObject
{
public:
    using ObjectType = T;
    using PointerType = ObjectType *;

    template <class U>
    friend class GObject;

public:
    GObject() :
        _obj{ nullptr }
    {
    }

    GObject(decltype(nullptr)) :
        _obj{ nullptr }
    {
    }

    explicit GObject(PointerType object) :
        _obj{ object }
    {
        AddRef();
    }

    GObject(const Ref<T> &ref) :
        _obj{ (T *)ref }
    {
        AddRef();
    }

    template <class U, class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    GObject(const Ref<U> &ref) :
        _obj{ (T *)(U *)ref }
    {
        AddRef();
    }

    GObject(const GObject &other) :
        _obj{ other._obj }
    {
        AddRef();
    }

    template <class U>
    GObject(const GObject<U> &other) :
        _obj{ nullptr }
    {
        static_assert(std::is_convertible_v<U *, T *>);
        _obj = other._obj;
        AddRef();
    }

    GObject(GObject &&other) noexcept :
        _obj{ other._obj }
    {
        other._obj = nullptr;
    }

    template <class U>
    GObject(GObject<U> &&other) noexcept :
        _obj{ nullptr }
    {
        static_assert(std::is_convertible_v<U *, T *>);
        _obj = other._obj;
        other._obj = nullptr;
    }

    ~GObject()
    {
        Release();
    }

    void Swap(GObject &other)
    {
        std::swap(_obj, other._obj);
    }

    void Reset(PointerType object = nullptr)
    {
        if (_obj != object)
        {
            Release();
            _obj = object;
            AddRef();
        }
    }

    void Release()
    {
        if (_obj)
        {
            if (_obj->RefCount() <= 1)
            {
                Graphics::Release(_obj, GObjectType<T>::Value);
            }
            else
            {
                _obj->UnRef();
            }
            _obj = nullptr;
        }
    }

    PointerType Get() const
    {
        return _obj;
    }

    PointerType operator->() const
    {
        return _obj;
    }

    operator PointerType() const
    {
        return _obj;
    }

    operator Ref<T>() const
    {
        return Ref<T>(_obj);
    }

    explicit operator bool() const
    {
        return _obj != nullptr;
    }

    bool operator!() const
    {
        return !_obj;
    }

    GObject &operator=(decltype(nullptr))
    {
        Release();
        return *this;
    }

    GObject &operator=(PointerType object)
    {
        Reset(object);
        return *this;
    }

    GObject &operator=(const Ref<T> &ref)
    {
        T *p = (T *)ref;
        if (_obj != p)
        {
            Release();
            _obj = p;
            AddRef();
        }
        return *this;
    }

    template <class U, class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    GObject &operator=(const Ref<U> &ref)
    {
        T *p = (T *)(U *)ref;
        if (_obj != p)
        {
            Release();
            _obj = p;
            AddRef();
        }
        return *this;
    }

    GObject &operator=(const GObject &other)
    {
        if (_obj != other._obj)
        {
            Release();
            _obj = other._obj;
            AddRef();
        }
        return *this;
    }

    template <class U>
    GObject &operator=(const GObject<U> &other)
    {
        static_assert(std::is_convertible_v<U *, T *>);
        if (_obj != other._obj)
        {
            Release();
            _obj = other._obj;
            AddRef();
        }
        return *this;
    }

    GObject &operator=(GObject &&other) noexcept
    {
        if (this != &other)
        {
            Release();
            _obj = other._obj;
            other._obj = nullptr;
        }
        return *this;
    }

    template <class U>
    GObject &operator=(GObject<U> &&other) noexcept
    {
        static_assert(std::is_convertible_v<U *, T *>);
        Release();
        _obj = other._obj;
        other._obj = nullptr;
        return *this;
    }

private:
    void AddRef()
    {
        if (_obj)
        {
            _obj->AddRef();
        }
    }

    PointerType _obj;
};

namespace GCreate
{

inline GObject<Texture> CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels = 1, uint16_t arrayLayers = 1, TextureType type = TextureType::None)
{
    return GObject<Texture>{ Graphics::GetDevice()->CreateTexture(format, width, height, mipLevels, arrayLayers, type) };
}

inline GObject<Texture> CreateTexture(const String &filepath, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread())
{
    return GObject<Texture>{ Graphics::CreateTexture(filepath, asyncComputeThread) };
}

inline GObject<Texture> CreateTexture(const Picture &picture, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread())
{
    return GObject<Texture>{ Graphics::CreateTexture(picture, asyncComputeThread) };
}

inline GObject<Texture> CreateTexture(Format format, uint32_t width, uint32_t height, uint32_t stride, const void *data, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread(), uint32_t mipLevels = 0)
{
    return GObject<Texture>{ Graphics::CreateTexture(format, width, height, stride, data, asyncComputeThread, mipLevels) };
}

inline GObject<Sampler> CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation = CompareOperation::Never, float minLod = 0.0f, float maxLod = 16.0f)
{
    return GObject<Sampler>{ Graphics::GetDevice()->CreateSampler(filter, addressMode, compareOperation, minLod, maxLod) };
}

inline GObject<DescriptorSet> CreateDescriptorSet(Pipeline *pipeline)
{
    return GObject<DescriptorSet>{ Graphics::GetDevice()->CreateDescriptorSet(pipeline) };
}

inline GObject<Pipeline> CreateComputePipeline(Shader *shader)
{
    return GObject<Pipeline>{ Graphics::GetDevice()->CreateComputePipeline(shader) };
}

}

}
