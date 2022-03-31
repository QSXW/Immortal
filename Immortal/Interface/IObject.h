#pragma once

#include "Core.h"

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

    uint32_t Ref() const
    {
        return ref;
    }

private:
    uint32_t ref = 0;
};

template <class T>
class Ref
{
public:
    using ObjectType = T;

public:
    Ref() :
        _obj{ nullptr}
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
        _obj{ other._obj }
    {
        other._obj = nullptr;
    }

    Ref(const Ref &other) :
        _obj{ other._obj }
    {
        AddRef();
    }

    template <class U>
    Ref(const Ref<U> &other) :
        _obj{ other._obj }
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

    T* Get() const
    {
        return _obj;
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

protected:
    ObjectType *_obj;
};

template <class T>
class MonoRef
{
public:
    MonoRef() :
        _obj{ nullptr }
    {

    }

    MonoRef(T *other) :
        _obj{ other }
    {

    }

    MonoRef(MonoRef &&other) :
        _obj{ other._obj }
    {
        other.Release();
    }

    ~MonoRef()
    {
        Release();
    }

    void Swap(MonoRef &other)
    {
        THROWIF(&other == this, SError::SelfAssignment);
        std::swap(other._obj, _obj);
    }

    void Reset(T *other)
    {
        MonoRef(other).Swap(*this);
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

protected:
    T *_obj;

private:
    MonoRef(const MonoRef &) = delete;
    MonoRef& operator=(const MonoRef &) = delete;
};

}
