#pragma once

namespace Immortal
{

class Device
{
public:
    Device() { }

    virtual ~Device() { }

    virtual void *Handle() = 0;
};

}
