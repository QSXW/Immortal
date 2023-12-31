#include "PhysicalDevice.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

PhysicalDevice::PhysicalDevice(Instance *instance, VkPhysicalDevice physicalDevice) :
    Handle{ physicalDevice },
    instance{ instance },
    RequestedFeatures{}
{
    GetFeatures(&Features);
    GetProperties(&Properties);
    GetMemoryProperties(&MemoryProperties);

    uint32_t count = 0;
    GetQueueFamilyProperties(&count, nullptr);
    QueueFamilyProperties.resize(count);
    GetQueueFamilyProperties(&count, QueueFamilyProperties.data());

    RequestedFeatures.textureCompressionBC       = Features.textureCompressionBC;
    RequestedFeatures.textureCompressionASTC_LDR = Features.textureCompressionASTC_LDR;
    RequestedFeatures.textureCompressionETC2     = Features.textureCompressionETC2;
}

PhysicalDevice::~PhysicalDevice()
{

}

void PhysicalDevice::Activate(Feature feature) const
{
    VkBool32 *features = (VkBool32 *)&RequestedFeatures;
    features[feature] = VK_TRUE;
}

void PhysicalDevice::Deactivate(Feature feature) const
{
    VkBool32 *features = (VkBool32 *)&RequestedFeatures;
    features[feature] = VK_FALSE;
}

}
}
