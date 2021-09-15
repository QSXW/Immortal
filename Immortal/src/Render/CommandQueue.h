#pragma once

#include "ImmortalCore.h"

#include <deque>

namespace Immortal
{
template <class F>
class IMMORTAL_API CommandQueue
{
public:
    using Callee = void(*)(void);

    void Enqueue(Callee func)
    {
        _M_commandQueue->push_back(func);
    }

    void Execute()
    {
        while (!_M_commandQueue.empty())
        {
            auto func = _M_commandQueue.pop_back();
            func();
        }
    }

private:
    std::deque<F> _M_commandQueue;
};

}

