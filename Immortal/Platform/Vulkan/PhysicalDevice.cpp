#include "PhysicalDevice.h"

namespace Immortal
{
namespace Vulkan
{

PhysicalDevice::PhysicalDevice(Instance *instance, VkPhysicalDevice physicalDevice) :
    handle{ physicalDevice },
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

PhysicalDevice::PhysicalDevice(PhysicalDevice &&other) :
    PhysicalDevice{}
{
    other.Swap(*this);
}

PhysicalDevice &PhysicalDevice::operator =(PhysicalDevice &&other)
{
    PhysicalDevice(std::move(other)).Swap(*this);
    return *this;
}

void PhysicalDevice::Swap(PhysicalDevice &other)
{
    std::swap(handle,                        other.handle                       );
    std::swap(instance,                      other.instance                     );
    std::swap(QueueFamilyProperties,         other.QueueFamilyProperties        );
    std::swap(ExtensionFeatures,             other.ExtensionFeatures            );
    std::swap(Features,                      other.Features                     );
    std::swap(Properties,                    other.Properties                   );
    std::swap(MemoryProperties,              other.MemoryProperties             );
    std::swap(RequestedFeatures,             other.RequestedFeatures            );
    std::swap(LastRequestedExtensionFeature, other.LastRequestedExtensionFeature);
    std::swap(HighPriorityGraphicsQueue,     other.HighPriorityGraphicsQueue    );
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
