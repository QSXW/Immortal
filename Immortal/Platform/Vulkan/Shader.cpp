#include "Shader.h"

#include <stack>

#include "FileSystem/FileSystem.h"
#include "Render/GLSLCompiler.h"
#include "Device.h"
#include "Media/RF.h"

namespace Immortal
{
namespace Vulkan
{

void CacheSpirv(const std::string &path, const std::string &shaderName, const std::string &src, const std::vector<uint32_t> &spirv)
{
    std::string filename = std::to_string(std::hash<std::string>{}(shaderName)) + std::string{ ".spirv" };

    if (!FileSystem::Path::Exsits(path))
    {
        FileSystem::MakeDirectory(path);
    }

    RF rf{ FileSystem::Path::Join(path, filename), Stream::Mode::Write };

    if (!rf.Writable())
    {
        return;
    }

    size_t srcHash = std::hash<std::string>{}(src);
    rf.Append(&srcHash);
    rf.Append(spirv);
    rf.Write();
}

bool ReadSpirv(const std::string &path, const std::string &shaderName, const std::string &src, std::vector<uint32_t> &spirv)
{
    std::string filename = std::to_string(std::hash<std::string>{}(shaderName)) + std::string{ ".spirv" };

    if (!FileSystem::Path::Exsits(path))
    {
        FileSystem::MakeDirectory(path);
    }

    RF rf{ FileSystem::Path::Join(path, filename), Stream::Mode::Read };
    if (!rf.Readable())
    {
        return false;
    }

    auto &chunks = rf.Read();
    if (chunks.size() != 2)
    {
        LOG::WARN("Cached spirv file corrupted!");
        return false;
    }
    auto hash = *rcast<const size_t *>(chunks[0].ptr);
    if (hash != std::hash<std::string>{}(src))
    {
        return false;
    }

    spirv.resize(chunks[1].size / sizeof(uint32_t));
    memcpy(spirv.data(), chunks[1].ptr, chunks[1].size);

    return true;
}

static inline VkPipelineShaderStageCreateInfo CreateStage(VkShaderModule module, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo createInfo{};
    createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage  = stage;
    createInfo.module = module;
    createInfo.pName  = "main";
    return createInfo;
}

Shader::Shader(Device *d, const std::string &filename, Shader::Type type) :
    Super{ type },
    device { d }
{   
    stages.reserve(2);
    if (type == Shader::Type::Graphics)
    {
        VkShaderModule vert = Load(filename + ".vert", Shader::Stage::Vertex);
        VkShaderModule frag = Load(filename + ".frag", Shader::Stage::Fragment);

        modules.push_back(vert);
        modules.push_back(frag);
        stages.resize(2);
        stages[0] = CreateStage(vert, VK_SHADER_STAGE_VERTEX_BIT);
        stages[1] = CreateStage(frag, VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    else
    {
        VkShaderModule comp = Load(filename + ".comp", Shader::Stage::Compute);
        modules.push_back(comp);
        stages.resize(1);
        stages[0] = CreateStage(comp, VK_SHADER_STAGE_COMPUTE_BIT);
    }
    Setup();
}

Shader::~Shader()
{
    for (auto &m : modules)
    {
        device->Destory(m);
    }
    device->Destory(descriptorSetLayout);
}

VkShaderModule Shader::Load(const std::string &filename, Shader::Stage stage)
{
    constexpr const char *tmpPath = "tmp/";

    GLSLCompiler compiler{};

    auto src = FileSystem::ReadString(filename);

    std::vector<uint32_t> spirv;
    std::string error;

    if (!ReadSpirv(tmpPath, filename, src, spirv))
    {
        if (!GLSLCompiler::Src2Spirv(Shader::API::Vulkan, stage, src.size(), src.data(), "main", spirv, error))
        {
            LOG::FATAL("Failed to compiler Shader => {0}\n", error.c_str());
            return VK_NULL_HANDLE;
        }
        CacheSpirv(tmpPath, filename, src, spirv);
    }

    GLSLCompiler::Reflect(spirv, resources);

    VkShaderModule shaderModule{ VK_NULL_HANDLE };
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode    = spirv.data();

    Check(vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule));

    // Reflect(src);
    SetupDescriptorSetLayout(stage);
    resources.clear();

    return shaderModule;
}

#define SKIP_IF(c, d, preprocess) if ((c) == (d)) \
    { \
        preprocess; \
        continue; \
    }

#define BREAK_IF(c, d, preprocess) if ((c) == (d)) \
    { \
        preprocess; \
        break; \
    }

inline size_t ParseKeyValue(const char *ptr, Shader::Resource &resource)
{
    char buffer[64] = { 0 };
    size_t i = 0, j = 0;

    while (ptr[i])
    {
        SKIP_IF(ptr[i], ' ', i++);

        if (ptr[i] == '=')
        {
            buffer[j++] = '\0';
            i++;
            continue;
        }

        if (ptr[i] == ',' || ptr[i] == ')')
        {
            buffer[j] = '\0';
            j = 0;
            if (Equals(buffer, "set"))
            {
                resource.set = std::atoi(buffer + 4);
            }
            else if (Equals(buffer, "location"))
            {
                resource.location = std::atoi(buffer + 9);
            }
            else if (Equals(buffer, "binding"))
            {
                resource.binding = std::atoi(buffer + 8);
            }
            else if (Equals(buffer, "push_constant"))
            {
                resource.type |= Shader::Resource::Type::PushConstant;
            }
            BREAK_IF(ptr[i], ')', );
            i++;
            continue;
        }

        buffer[j++] = ptr[i++];
    }

    return i;
}

inline size_t ParseStructure(const char *ptr, Shader::Resource &resource)
{
    size_t i = 0, j = 0;
    char buffer[64] = { 0 };

    std::vector<InputElement> elements;
    
    InputElement e;
    while (ptr[i] != '}')
    {
        SKIP_IF(ptr[i], '\t', i++);
        SKIP_IF(ptr[i], '\r', i++);
        SKIP_IF(ptr[i], '\n', i++);
        if (ptr[i] == ' ')
        {
            if (j > 0)
            {
                buffer[j] = '\0';
                if (e.format != Format::None)
                {
                    e.format = Shader::GetResourceFormat(buffer);
                }
            }
            j = 0;
            i++;
            continue;
        }
        if (ptr[i] == ';')
        {
            buffer[j] = '\0';
            e.name = buffer;
            elements.emplace_back(e);
            j = 0;
            i++;
            continue;
        }

        buffer[j++] = ptr[i++];
    }

    InputElementDescription desc{ std::move(elements) };
    resource.size = desc.Stride();

    return i;
}

inline void GetType(const char *word, Shader::Resource &resource)
{
    auto type = Shader::GetResourceType(std::string{ word });
    if (type != Shader::Resource::Type::None)
    {
        resource.type = resource.type | type;
    }
}

inline void GetFormat(const char *word, Shader::Resource &resource)
{
    auto format = Shader::GetResourceFormat(std::string{ word });
    if (format != Format::None)
    {
        resource.format = format;
    }
}

inline size_t ParseLayout(const char *ptr, Shader::Resource &resource)
{
    size_t i = 0, j = 0;
    std::stack<uint8_t> tokens;
    char buffer[128] = { 0 };
    bool isStructure = false;

    while (*ptr)
    {
        if (ptr[i] == ' ')
        {
            if (j > 0)
            {
                GetType(buffer, resource);
                GetFormat(buffer, resource);
            }
            j = 0;
            i++;
            continue;
        }
        SKIP_IF(ptr[i], '\n', i++);
        SKIP_IF(ptr[i], '\r', i++);
        if (ptr[i] == '(')
        {
            tokens.push(ptr[i++]);
            i += ParseKeyValue(ptr + i, resource);
            continue;
        }
        if (ptr[i] == '{')
        {
            isStructure = true;
            buffer[j] = '\0';
            resource.name = buffer;
            tokens.push(ptr[i++]);
            i += ParseStructure(ptr + i, resource);
            continue;
        }
        if (ptr[i] == ')')
        {
            if (tokens.top() == '(')
            {
                tokens.pop();
            }
            i++;
            continue;
        }
        if (ptr[i] == '}')
        {
            if (tokens.top() == '{')
            {
                tokens.pop();
            }
            i++;
            continue;
        }
        if (ptr[i] == '[')
        {
            i++;
            j = 0;
            while (ptr[i] != ']')
            {
                buffer[j++] = ptr[i++];
            }
            buffer[j] = '\0';
            resource.count = std::atoi(buffer);
        }
        BREAK_IF(ptr[i], ';', buffer[j++] = '\0'; i++; if (!isStructure) { resource.name = buffer; });
        buffer[j++] = ptr[i++];
    }

    if (!tokens.empty())
    {
        LOG::ERR("Failed to reflect the shader resources");
    }

    return i;
}

void Shader::Reflect(const std::string &source)
{
    const std::string layout = std::string{ "layout" };
    std::vector<VkPushConstantRange> ranges{};

    size_t i = 0;
    while ((i = source.find(layout, i)) != std::string::npos)
    {
        resources.resize(resources.size() + 1);
        auto &resource = resources.back();
        resource.count = 1;

        i += layout.size();
        i += ParseLayout(source.data() + i, resource);
    }
}

void Shader::SetupDescriptorSetLayout(Stage stage)
{
    for (auto &resource : resources)
    {
        if (resource.type & Resource::Type::PushConstant)
        {
            if (resource.size >= Limit::PushConstantMaxSize)
            {
                LOG::WARN("Using a push constants size exceeded {0} bytes, "
                    "which is not recommended by all vendor devices", Limit::PushConstantMaxSize);
            }
            pushConstantRanges.emplace_back(VkPushConstantRange{ (VkShaderStageFlags)stage, 0, resource.size });
        }
        else if (resource.type & Resource::Type::Uniform)
        {
            VkDescriptorSetLayoutBinding bindingInfo{};
            bindingInfo.binding            = resource.binding;
            bindingInfo.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindingInfo.descriptorCount    = resource.count;
            bindingInfo.stageFlags         = (VkShaderStageFlags)stage;
            bindingInfo.pImmutableSamplers = nullptr;

            VkWriteDescriptorSet writeDescriptor{};
            writeDescriptor.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptor.pNext           = nullptr;
            writeDescriptor.descriptorCount = resource.count;
            writeDescriptor.dstBinding      = resource.binding;

            if (resource.type & Resource::Type::ImageSampler)
            {
                bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writeDescriptor.pImageInfo = nullptr;
            }
            if (resource.type & Resource::Type::ImageStorage)
            {
                bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writeDescriptor.pImageInfo = nullptr;
            }
            if (bindingInfo.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                writeDescriptor.pBufferInfo = nullptr;
            }
            writeDescriptor.descriptorType = bindingInfo.descriptorType;

            descriptorSetLayoutBindings.emplace_back(std::move(bindingInfo));
            descriptorSetUpdater.Emplace(resource.name, std::move(writeDescriptor));
        }
    }
}

void Shader::Setup()
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = U32(descriptorSetLayoutBindings.size());
    layoutInfo.pBindings    = descriptorSetLayoutBindings.data();

    Check(device->Create(&layoutInfo, nullptr, &descriptorSetLayout));
    pipelineLayout = PipelineLayout{
        *device, 1, &descriptorSetLayout,
        pushConstantRanges.empty() ? nullptr : pushConstantRanges.data(),
        U32(pushConstantRanges.size())
    };

    pushConstantRanges.clear();
    descriptorSetLayoutBindings.clear();
}

}
}
