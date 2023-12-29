#pragma once

#include <array>

#include "Common.h"
#include "Render/Shader.h"
#include "PipelineLayout.h"
#include "Buffer.h"
#include "Descriptor.h"
#include "Handle.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class IMMORTAL_API Shader : public SuperShader, public Handle<VkShaderModule>
{
public:
    using Super = SuperShader;
    VKCPP_SWAPPABLE(Shader)

public:
    Shader();

    Shader(Device *device, const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint);

    virtual ~Shader() override;

    VkShaderModule Load(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint);

    VkShaderModule Load(const std::string &filename, Stage stage);

    VkPipelineShaderStageCreateInfo CreateStage(VkShaderModule module, VkShaderStageFlagBits stage);

public:
    const std::vector<VkDescriptorSetLayoutBinding> &GetDescriptorSetLayoutBinding() const
    {
        return descriptorSetLayoutBindings;
    }

    const std::vector<VkPushConstantRange> &GetPushConstantRanges() const
    {
        return pushConstantRanges;
    }

    VkShaderStageFlagBits GetStage() const
    {
        return stage;
    }

    const char *GetEntryPoint() const
    {
        return entryPoint.c_str();
    }

    void Swap(Shader &other)
    {
        Handle::Swap(other);
        std::swap(device,                      other.device                     );
        std::swap(pushConstantRanges,          other.pushConstantRanges         );
        std::swap(descriptorSetLayoutBindings, other.descriptorSetLayoutBindings);
        std::swap(stage,                       other.stage                      );
    }
 
protected:
	VkShaderModule CreateModuleBySpriv(const std::vector<uint8_t> &spirv, VkShaderStageFlagBits stage);

protected:
    Device *device;

    std::vector<VkPushConstantRange> pushConstantRanges;

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

    std::string entryPoint;

    VkShaderStageFlagBits stage;
};

}
}
