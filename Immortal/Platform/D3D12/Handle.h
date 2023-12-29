#pragma once

namespace Immortal
{
namespace D3D12
{

class Device;
class NonDispatchableHandle
{
public:
    NonDispatchableHandle(Device *device = nullptr) :
        device{ device }
    {

    }

    Device *GetDevice() const
    {
        return device;
    }

    void Swap(NonDispatchableHandle &other)
    {
		std::swap(device, other.device);
    }
    
protected:
    Device *device;
};

}
}