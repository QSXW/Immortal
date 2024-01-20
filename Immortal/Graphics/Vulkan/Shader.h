#pragma once

#include <array>

#include "Common.h"
#include "Graphics/Shader.h"
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

    void Load(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint);

    void ConstructShaderModule();

    void GetPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo *pPipelineShaderStageCreateInfo, VkPipelineShaderStageModuleIdentifierCreateInfoEXT *pIdentifierCreateInfo);

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
	void Construct();

protected:
    Device *device;
    
    std::vector<uint8_t> spirv;

    std::vector<VkPushConstantRange> pushConstantRanges;

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

    std::string entryPoint;

    VkShaderStageFlagBits stage;

    uint32_t identifierSize;

	uint8_t identifier[VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT];
};

}
}
