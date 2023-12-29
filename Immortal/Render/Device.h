#pragma once

#include "Core.h"
#include "Format.h"
#include "Interface/IObject.h"

namespace Immortal
{

class Queue;
class Buffer;
class Texture;
class Window;
class Swapchain;
class CommandBuffer;
class Sampler;
class Shader;
class GPUEvent;
class GraphicsPipeline;
class ComputePipeline;
class RenderTarget;
class Pipeline;
class Texture;
class DescriptorSet;
class RenderTarget;
class IMMORTAL_API Device
{
public:
    virtual ~Device() = default;

    /**
     * @brief Get the backend handle
     * 
     * Vulkan: VkDevice
     * OpenGL: A pointer to GLFWwindow
     * D3D11:  A pointer to ID3D11Device
     * D3D12:  A pointer to ID3D12Device
     */
    virtual Anonymous GetBackendHandle() const = 0;

    /**
     * @brief Get the backend API Vulkan/OpenGL/D3D11/D3D12/Metal...
     */
    virtual BackendAPI GetBackendAPI() = 0;

    /**
     * @brief create command queue
     * The graphics queue is available for executing render/compute/copy commands
     * The compute queue is available for compute/copy commands
     * The Transfer queue is only available for copy commands
     * The VideoDecoding queue is availale for video decoding commands
     * The VideoEncoding queue is availale for video encoding commands
     */
    virtual Queue *CreateQueue(QueueType type, QueuePriority priority = QueuePriority::Normal) = 0;

    /**
     * @brief Create a command buffer for commands recording
     * If the D3D12 backend is used. The type of the queue must be set the same as
     * the queue that executing this command buffer.
     */
    virtual CommandBuffer *CreateCommandBuffer(QueueType type = QueueType::Graphics) = 0;

    /**
     * @brief Create a swapchain
     * @param             queue The queue that used to submit the commmand buffer that renders something into the render target the this swapchain
     * @param window      The pointer to the Window for surface creation
     * @param format      The render target format of the swapchain
     * @param bufferCount The number of the back buffer 
     * @param mode         VerticalSync or None
     */
    virtual Swapchain *CreateSwapchain(Queue *queue, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode) = 0;

    /**
     * @brief Create a sampler
     */
    virtual Sampler *CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation = CompareOperation::Never, float minLod = 0.0f, float maxLod = 1.0f) = 0;
    
    /**
     * @brief Create a shader
     * @param name       The name of the specific shader source, which is useful for error logging
     * @param state      The stage for the specific shader source
     * @param source     The shader source code
     * @param entryPoint The entry point of the shader program
     */
    virtual Shader *CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint) = 0;
    
    /**
     * @brief Create a graphics pipeline
     */
    virtual GraphicsPipeline *CreateGraphicsPipeline() = 0;

    /**
	 * @brief Create a compute pipeline
     * @param shader The shader that is loaded into the compute pipeline
	 */
    virtual ComputePipeline *CreateComputePipeline(Shader *shader) { return nullptr; }

    /**
     * @brief Create a buffer
     * @param size The size of the buffer
     * @param type The type of the buffer
     */
    virtual Buffer *CreateBuffer(size_t size, BufferType type) = 0;

    /**
	 * @brief Create a texture
	 */
    virtual Texture *CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels = 1, uint16_t arrayLayers = 1, TextureType type = TextureType::None) = 0;

    /**
     * @brief Create a descriptor set, which is used to bind all of the resources that the pipeline will used at once
     */
    virtual DescriptorSet *CreateDescriptorSet(Pipeline *pipeline) = 0;

    /**
     * @brief Create a gpu event for synchoronization.
     */
	virtual GPUEvent *CreateGPUEvent(const std::string &name = {}) = 0;


    /**
     * @brief Create a render target
     */
    virtual RenderTarget *CreateRenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat = {}) = 0;
};

using SuperDevice = Device;

}
