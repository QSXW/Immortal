#pragma once

#include "Core.h"
#include <atomic>

namespace Immortal
{

struct IMMORTAL_API IObject
{
public:
    uint32_t AddRef()
    {
        return ++ref;
    }

    uint32_t UnRef()
    {
        return --ref;
    }

    uint32_t RefCount() const
    {
        return ref;
    }

private:
    std::atomic<uint32_t> ref = 0;
};

template <class T>
class Ref
{
public:
    using ObjectType = T;
    using PointerType = ObjectType*;

public:
    Ref() :
        _obj{ nullptr }
    {
        static_assert(std::is_base_of_v<IObject, T>);
    }

    template<class U>
    Ref(U *other) :
        _obj{ other }
    {
        static_assert(std::is_base_of_v<IObject, U>);
        AddRef();
    }

    Ref(Ref &&other) :
        _obj{ nullptr }
    {
        THROWIF(&other == this, SError::SelfAssignment);
        Swap(other);
    }

    template <class U>
    Ref(Ref<U> &&other) :
        _obj{ nullptr }
    {
        other.Swap(*this);
    }

    Ref(const Ref &other) :
        _obj{ other._obj }
    {
        AddRef();
    }

    template <class U>
    Ref(const Ref<U> &other) :
        _obj{ dynamic_cast<PointerType>(other._obj) }
    {
        AddRef();
    }

    Ref(decltype(nullptr)) :
        _obj{ nullptr }
    {

    }

    ~Ref()
    {
        UnRef();
    }

    void Swap(Ref &&other)
    {
        THROWIF(&other == this, SError::SelfAssignment);
        std::swap(_obj, other._obj);
    }

    void Swap(Ref &other)
    {
        THROWIF(&other == this, SError::SelfAssignment);
        std::swap(_obj, other._obj);
    }

    Ref &operator=(decltype(nullptr))
    {
        UnRef();
        return *this;
    }

    Ref &operator=(Ref &&other)
    {
        Ref(std::move(other)).Swap(*this);
        return *this;
    }

    Ref &operator=(const Ref &other)
    {
        if (other._obj != _obj)
        {
            Ref(other).Swap(*this);
        }
        return *this;
    }

    operator T*() const
    {
        return _obj;
    }

    void Swap(T *other)
    {
        Ref(other).Swap(*this);
    }

    void Reset(T *other = nullptr)
    {
        UnRef();
        Ref(other).Swap(*this);
    }

    T *operator->() const
    {
        return _obj;
    }

    template <class U>
    U *As() noexcept
    {
        return (U *)_obj;
    }

    T *Get() const
    {
        return _obj;
    }

    template <class U>
    U *InterpretAs() const
    {
        if constexpr ((std::is_base_of_v<T, U> || std::is_base_of_v<U, T>) && !std::is_class_v<IObject>)
        {
            return dynamic_cast<U*>(_obj);
        }
        else
        {
            return (U *)_obj;
        }
    }
    
    template <class U>
    U *GetPrimitive()
    {
        return (U *)((uint8_t *)_obj + sizeof(IObject));
    }

    template <class U>
    U &DeRef() const
    {
        return *InterpretAs<U>();
    }

    T *const* GetAddress() const
    {
        return &_obj;
    }

protected:
    void AddRef() const
    {
        if (_obj)
        {
            _obj->AddRef();
        }
    }

    uint32_t UnRef()
    {
        uint32_t ref = 0;
        if (_obj && (ref = _obj->UnRef()) == 0)
        {
            delete _obj;
        }

        _obj = nullptr;
        return ref;
    }

public:
    ObjectType *_obj;
};

template <class T>
class URef
{
public:
    URef() :
        _obj{ nullptr }
    {

    }

    URef(T *other) :
        _obj{ other }
    {

    }

    URef(URef &&other) :
        _obj{ nullptr }
    {
        other.Swap(*this);
    }

    template <class U>
    URef(URef<U> &&other) :
        _obj{ nullptr }
    {
        other.Swap(*this);
    }

    URef &operator=(URef &&other)
    {
        URef(std::move(other)).Swap(*this);
        return *this;
    }

    template <class U>
    URef &operator=(URef<U> &&other)
    {
        URef(std::move(other)).Swap(*this);
        return *this;
    }

    ~URef()
    {
        Release();
    }

    void Swap(URef &other)
    {
        std::swap(other._obj, _obj);
    }

    void Reset(T *other)
    {
        URef(other).Swap(*this);
    }

    void Reset()
    {
        Release();
    }

    void Reset(decltype(nullptr))
    {
        Release();
    }

    void Release()
    {
        if (_obj)
        {
            delete _obj;
            _obj = nullptr;
        }
    }

    T *Get() const
    {
        return _obj;
    }

    operator T*() const
    {
        return _obj;
    }

    bool operator!() const
    {
        return !_obj;
    }

    T *operator->() const
    {
        return _obj;
    }

    URef &operator=(T *other)
    {
        URef(other).Swap(*this);
        return *this;
    }

    template <class U>
    U *InterpretAs() const
    {
        if constexpr (std::is_base_of_v<T, U> || std::is_base_of_v<U, T>)
        {
            return dynamic_cast<U*>(_obj);
        }
        else
        {
            return (U *)_obj;
        }
    }

    template <class U>
    U &DeRef() const
    {
        return *InterpretAs<U>();
    }

    URef(const URef &) = delete;

    URef &operator=(const URef &) = delete;

protected:
    T *_obj;
};

}
