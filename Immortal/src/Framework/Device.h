#pragma once

namespace Immortal
{
    class Device
    {
    public:
        Device() { }

        virtual ~Device() { }

        virtual void *Handle() = 0;

        virtual void *SuitableQueue()
        { 
            return nullptr; 
        }
    };
}
