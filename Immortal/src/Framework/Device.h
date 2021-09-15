#pragma once

namespace Immortal
{

class Device
{
public:
    Device() { }

    virtual ~Device() { }

    virtual void *Handle() { return nullptr; }
};

using SuperDevice = Device;

}
