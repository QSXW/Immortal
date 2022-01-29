#pragma once

#include "Core.h"

namespace Immortal
{

template <class T>
class Delegate
{

};

template <class F>
class Delegate<F()>
{
    using Handle = void*;
    using Callee = F (*)();
    using InternalCallee = void(*)(Handle);
    using Stub = std::pair<Handle, InternalCallee>;
    using Deletor = InternalCallee;

    template <Callee func>
    static void FunctionStub(Handle h)
    {
        return (func)();
    }

    template <class C, F (C::*func)()>
    static void ClassMethodStub(Handle h)
    {
        return (static_cast<C*>(h)->*func)();
    }

    template <class C>
    static void Release(Handle h)
    {
        delete static_cast<C*>(h);
    }

public:
    Delegate() :
        stub{ nullptr, nullptr },
        deletor(nullptr)
    {

    }

    ~Delegate()
    {
        if (stub.first)
        {
            deletor(stub.first);
            stub.first = nullptr;
        }
    }

    template <Callee func>
    void Map(void)
    {
        stub.first  = nullptr;
        stub.second = &Delegate::FunctionStub<func>;
    }

    template <class C, F (C::*func)()>
    void Map(C* instance)
    {
        stub.first  = instance;
        stub.second = &Delegate::ClassMethodStub<C, func>;
        deletor     = &Delegate::Release<C>;
    }

    void Invoke() const
    {
        return stub.second(stub.first);
    }

private:
    Stub stub;
    Deletor deletor;
};

template <class F, class A0>
class Delegate<F(A0)>
{

};

template <class F, class A0, class A1>
class Delegate<F(A0, A1)>
{

};

}
