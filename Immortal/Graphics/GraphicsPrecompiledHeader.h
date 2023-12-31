#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <list>
#include <filesystem>
#include <fstream>

#include "AccelerationStructure.h"
#include "AsyncCompute.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "Descriptor.h"
#include "DescriptorSet.h"
#include "Device.h"
#include "DirectXShaderCompiler.h"
#include "Format.h"
#include "GLSLCompiler.h"
#include "GPUEvent.h"
#include "InputElement.h"
#include "Instance.h"
#include "LightGraphics.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "PipelineCache.h"
#include "Queue.h"
#include "RenderTarget.h"
#include "Sampler.h"
#include "Shader.h"
#include "Swapchain.h"
#include "Texture.h"
#include "Types.h"
#include "Window.h"
#include "WindowCapture.h"

#include "Event/ApplicationEvent.cpp"
#include "Event/ApplicationEvent.h"
#include "Event/Event.h"
#include "Event/KeyCodes.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

#ifdef _WIN32

#include <wrl/client.h>
#include <initguid.h>
#include <d3d11.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3d11shader.h>
#include <d3d12shader.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#include "D3D/Interface.h"
#include "D3D/Instance.h"
#include "D3D/PhysicalDevice.h"

#include "D3D11/Buffer.h"
#include "D3D11/CommandBuffer.h"
#include "D3D11/Common.h"
#include "D3D11/Descriptor.h"
#include "D3D11/DescriptorSet.h"
#include "D3D11/Device.h"
#include "D3D11/Fence.h"
#include "D3D11/GPUEvent.h"
#include "D3D11/Handle.h"
#include "D3D11/Image.h"
#include "D3D11/PhysicalDevice.h"
#include "D3D11/Pipeline.h"
#include "D3D11/Queue.h"
#include "D3D11/RenderTarget.h"
#include "D3D11/Sampler.h"
#include "D3D11/Shader.h"
#include "D3D11/Swapchain.h"
#include "D3D11/Texture.h"
#include "D3D11/WindowCapture.h"

#include "D3D12/AccelerationStructure.h"
#include "D3D12/Barrier.h"
#include "D3D12/Buffer.h"
#include "D3D12/CommandAllocator.h"
#include "D3D12/CommandBuffer.h"
#include "D3D12/CommandList.h"
#include "D3D12/Common.h"
#include "D3D12/Descriptor.h"
#include "D3D12/DescriptorHeap.h"
#include "D3D12/DescriptorPool.h"
#include "D3D12/DescriptorSet.h"
#include "D3D12/Device.h"
#include "D3D12/Event.h"
#include "D3D12/Fence.h"
#include "D3D12/Handle.h"
#include "D3D12/PhysicalDevice.h"
#include "D3D12/Pipeline.h"
#include "D3D12/Queue.h"
#include "D3D12/RenderTarget.h"
#include "D3D12/Resource.h"
#include "D3D12/RootSignature.h"
#include "D3D12/Sampler.h"
#include "D3D12/Shader.h"
#include "D3D12/Swapchain.h"
#include "D3D12/Texture.h"
#include "D3D12/VideoCommon.h"
#include "D3D12/VideoDecodeCommandList.h"
#include "D3D12/VideoDecoder.h"
#include "D3D12/VideoDecoderHeap.h"
#include "D3D12/VideoDevice.h"
#endif

#include "OpenGL/Buffer.h"
#include "OpenGL/CommandBuffer.h"
#include "OpenGL/Common.h"
#include "OpenGL/Descriptor.h"
#include "OpenGL/DescriptorSet.h"
#include "OpenGL/Device.h"
#include "OpenGL/GLFormat.h"
#include "OpenGL/GPUEvent.h"
#include "OpenGL/PhysicalDevice.h"
#include "OpenGL/Pipeline.h"
#include "OpenGL/Queue.h"
#include "OpenGL/RenderTarget.h"
#include "OpenGL/Sampler.h"
#include "OpenGL/Shader.h"
#include "OpenGL/Swapchain.h"
#include "OpenGL/Texture.h"
#include "OpenGL/VertexArray.h"

#include "Vulkan/AccelerationStructure.h"
#include "Vulkan/Attachment.h"
#include "Vulkan/Barrier.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/CommandPool.h"
#include "Vulkan/Common.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/DescriptorPool.h"
#include "Vulkan/DescriptorSet.h"
#include "Vulkan/Device.h"
#include "Vulkan/FencePool.h"
#include "Vulkan/Framebuffer.h"
#include "Vulkan/GPUEvent.h"
#include "Vulkan/Handle.h"
#include "Vulkan/Image.h"
#include "Vulkan/ImageView.h"
#include "Vulkan/Instance.h"
#include "Vulkan/PhysicalDevice.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/PipelineLayout.h"
#include "Vulkan/Queue.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/RenderTarget.h"
#include "Vulkan/Sampler.h"
#include "Vulkan/Semaphore.h"
#include "Vulkan/SemaphorePool.h"
#include "Vulkan/Shader.h"
#include "Vulkan/Submitter.h"
#include "Vulkan/Surface.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/Texture.h"
#include "Vulkan/TimelineCommandBuffer.h"
#include "Vulkan/TimelineCommandPool.h"
#include "Vulkan/VideoSession.h"
#include "Vulkan/vk_mem_alloc.h"
#include "Vulkan/volk.h"

#ifdef _WIN32
#include "Window/DirectWindow.h"
#include "Window/NativeInput.h"
#endif

#include "Window/GLFWInput.h"
#include "Window/GLFWWindow.h"
