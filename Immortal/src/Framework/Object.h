#pragma once

#include "ImmortalCore.h"

namespace Immortal
{

class IMMORTAL_API Object
{
public:
    using HandleType = uint64_t;
    using ValueType  = uint64_t;

    enum class Type
    {
        Mesh,
        Texture,
        Audio,
        Script,
        None,
        Other,
        Missing
    };

public:
    Object::Type Type{ Object::Type::None };
    HandleType Handle;

public:
    virtual ~Object() { }

    virtual bool operator==(const Object &other) const
    {
        return this->Handle == other.Handle;
    }

    virtual bool operator!=(const Object &other) const
    {
        return !(*this == other);
    }

public:
    void __IncreaseRefCount() const NOEXCEPT { _M_refcount++; }
    void __DecreaseRefCount() const NOEXCEPT { _M_refcount--; }
    constexpr ValueType RefCount() const NOEXCEPT { return _M_refcount; }

private:
    mutable volatile ValueType _M_refcount = 0;
};

}
