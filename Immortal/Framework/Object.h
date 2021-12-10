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

private:
    ValueType IncreaseRef()
    {
        return ++refCount;
    }

    ValueType DecreaseRef()
    {
        return --refCount;
    }

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

    virtual ValueType RefCount() const
    {
        return refCount;
    }

    virtual ValueType AddRef()
    {
        return IncreaseRef();
    }

    virtual ValueType Release()
    {
        DecreaseRef();
        if (refCount == 0)
        {
            delete this;
        }
        return refCount;
    }

private:
    mutable volatile ValueType refCount = 0;
};

}
