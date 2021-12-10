#pragma once

#include "ImmortalCore.h"

namespace Immortal {

#define IM_INLINE __forceinline

    template <class T>
    class Delegate { };

    template <class F>
    class Delegate<F()>
    {
        using Handle = void*;
        using Callee = F (*)();
        using InternalCallee = void(*)(Handle);
        using Stub = std::pair<Handle, InternalCallee>;
        using Deletor = InternalCallee;

        template <Callee func>
        static IM_INLINE void FunctionStub(Handle h)
        {
            return (func)();
        }

        template <class C, F (C::*func)()>
        static IM_INLINE void ClassMethodStub(Handle h)
        {
            return (static_cast<C*>(h)->*func)();
        }

        template <class C>
        static IM_INLINE void Release(Handle h)
        {
            delete static_cast<C*>(h);
        }

    public:
        Delegate()
            : _M_stub{ nullptr, nullptr }, _M_deletor(nullptr)
        {

        }

        ~Delegate()
        {
            if (_M_stub.first)
            {
                _M_deletor(_M_stub.first);
                _M_stub.first = nullptr;
            }
        }

        template <Callee func>
        void Map(void)
        {
            _M_stub.first = nullptr;
            _M_stub.second = &Delegate::FunctionStub<func>;
        }

        template <class C, F (C::*func)()>
        void Map(C* instance)
        {
            _M_stub.first = instance;
            _M_stub.second = &Delegate::ClassMethodStub<C, func>;
            _M_deletor = &Delegate::Release<C>;
        }

        void Invoke() const
        {
            return _M_stub.second(_M_stub.first);
        }

    private:
        Stub _M_stub;
        Deletor _M_deletor;
    };

    template <class F, class A0>
    class Delegate<F(A0)>
    {

    };

    template <class F, class A0, class A1>
    class Delegate<F(A0, A1)>
    {

    };
#undef IM_INLINE
    // using MyDelegate = Delegate<void (int, const char*)>;
}