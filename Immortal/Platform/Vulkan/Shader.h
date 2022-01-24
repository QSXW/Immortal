#pragma once

#include <array>

#include "Common.h"
#include "Render/Shader.h"
#include "Render/GLSLCompiler.h"
#include "PipelineLayout.h"
#include "Buffer.h"
#include "Descriptor.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Shader : public SuperShader
{
public:
    using Super = SuperShader;

public:
    Shader(Device *device, const std::string &filename, Type type = Type::Graphics);

    ~Shader();

    VkShaderModule Load(const std::string &filename, Stage stage);

    auto &Stages()
    {
        return stages;
    }

    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<PipelineLayout, T>())
        {
            return pipelineLayout;
        }
        if constexpr (IsPrimitiveOf<VkDescriptorSetLayout, T>())
        {
            return descriptorSetLayout;
        }
    }

    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<DescriptorSetUpdater, T>())
        {
            return &descriptorSetUpdater;
        }
    }

    const std::vector<VkDescriptorPoolSize> &PoolSize() const
    {
        return poolSizes;
    }
 
private:
    void Reflect(const std::string &source);

    void SetupDescriptorSetLayout(Stage stage);

    void Setup();

private:
    Device *device{ nullptr };

    std::vector<VkShaderModule> modules;

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    std::vector<Shader::Resource> resources;

    PipelineLayout pipelineLayout;

    VkDescriptorSetLayout descriptorSetLayout;

    DescriptorSetUpdater descriptorSetUpdater;

    std::vector<VkDescriptorPoolSize> poolSizes;

    std::vector<VkPushConstantRange> pushConstantRanges;

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
};

}
}
