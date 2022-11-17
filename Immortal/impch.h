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

#include "slapi/slapi.h"
#include "slapi/slcast.h"
#include "slapi/slcore.h"
#include "slapi/slcpuid.h"

#include "Core.h"
#include "Format.h"

#include "Algorithm/LightArray.h"
#include "Algorithm/LightVector.h"
#include "Algorithm/Rotate.h"

#ifdef __linux__
#include "Audio/ALSA.h"
#endif
#include "Audio/AudioRenderContext.h"
#include "Audio/AudioSource.h"
#include "Audio/Device.h"
#ifdef _WIN32
#include "Audio/WASAPI.h"
#endif

#include "Event/ApplicationEvent.cpp"
#include "Event/ApplicationEvent.h"
#include "Event/Event.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/RF.h"
#include "FileSystem/Stream.h"

#include "Helper/Arguments.h"
#include "Helper/json.h"
#include "Helper/nlohmann_json.h"
#include "Helper/Platform.h"

#include "Framework/Application.h"
#include "Framework/Async.h"
#include "Framework/Device.h"
#include "Framework/DLLLoader.h"
#include "Framework/Input.h"
#include "Framework/KeyCodes.h"
#include "Framework/Layer.h"
#include "Framework/LayerStack.h"
#include "Framework/Log.h"
#include "Framework/Timer.h"
#include "Framework/Utils.h"
#include "Framework/Window.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/ImGuizmo.h"
#include "ImGui/GuiLayer.h"
#include "ImGui/Utils.h"

#include "Interface/Delegate.h"
#include "Interface/IObject.h"

#include "Math/Math.h"
#include "Math/Vector.h"

#include "Memory/Allocator.h"
#include "Memory/Memory.h"
#include "Memory/MemoryAllocator.h"

#include "Net/LTP.h"
#include "Net/Socket.h"
#include "Net/TCP.h"

#ifdef _WIN32
#include "Platform/D3D/Interface.h"

#include "Platform/D3D11/Buffer.h"
#include "Platform/D3D11/Common.h"
#include "Platform/D3D11/Descriptor.h"
#include "Platform/D3D11/Device.h"
#include "Platform/D3D11/Fence.h"
#include "Platform/D3D11/GuiLayer.h"
#include "Platform/D3D11/Image.h"
#include "Platform/D3D11/Pipeline.h"
#include "Platform/D3D11/RenderContext.h"
#include "Platform/D3D11/RenderTarget.h"
#include "Platform/D3D11/Sampler.h"
#include "Platform/D3D11/Shader.h"
#include "Platform/D3D11/Swapchain.h"
#include "Platform/D3D11/Texture.h"

#include "Platform/D3D12/Barrier.h"
#include "Platform/D3D12/Buffer.h"
#include "Platform/D3D12/CommandAllocator.h"
#include "Platform/D3D12/CommandList.h"
#include "Platform/D3D12/Common.h"
#include "Platform/D3D12/Descriptor.h"
#include "Platform/D3D12/DescriptorHeap.h"
#include "Platform/D3D12/Device.h"
#include "Platform/D3D12/Fence.h"
#include "Platform/D3D12/GuiLayer.h"
#include "Platform/D3D12/Pipeline.h"
#include "Platform/D3D12/Queue.h"
#include "Platform/D3D12/RenderContext.h"
#include "Platform/D3D12/RenderTarget.h"
#include "Platform/D3D12/Resource.h"
#include "Platform/D3D12/RootSignature.h"
#include "Platform/D3D12/Shader.h"
#include "Platform/D3D12/Swapchain.h"
#include "Platform/D3D12/Texture.h"
#include "Platform/D3D12/VideoCommon.h"
#include "Platform/D3D12/VideoDecodeCommandList.h"
#include "Platform/D3D12/VideoDecoder.h"
#include "Platform/D3D12/VideoDecoderHeap.h"
#include "Platform/D3D12/VideoDevice.h"
#endif

#include "Platform/OpenGL/Buffer.h"
#include "Platform/OpenGL/CommandBuffer.h"
#include "Platform/OpenGL/Common.h"
#include "Platform/OpenGL/Descriptor.h"
#include "Platform/OpenGL/Framebuffer.h"
#include "Platform/OpenGL/GLFormat.h"
#include "Platform/OpenGL/GuiLayer.h"
#include "Platform/OpenGL/Pipeline.h"
#include "Platform/OpenGL/RenderContext.h"
#include "Platform/OpenGL/RenderTarget.h"
#include "Platform/OpenGL/Shader.h"
#include "Platform/OpenGL/Texture.h"
#include "Platform/OpenGL/VertexArray.h"

#include "Platform/Vulkan/Attachment.h"
#include "Platform/Vulkan/Barrier.h"
#include "Platform/Vulkan/Buffer.h"
#include "Platform/Vulkan/CommandBuffer.h"
#include "Platform/Vulkan/CommandPool.h"
#include "Platform/Vulkan/Common.h"
#include "Platform/Vulkan/Descriptor.h"
#include "Platform/Vulkan/DescriptorPool.h"
#include "Platform/Vulkan/DescriptorSet.h"
#include "Platform/Vulkan/Device.h"
#include "Platform/Vulkan/FencePool.h"
#include "Platform/Vulkan/Framebuffer.h"
#include "Platform/Vulkan/GuiLayer.h"
#include "Platform/Vulkan/Image.h"
#include "Platform/Vulkan/ImageView.h"
#include "Platform/Vulkan/Instance.h"
#include "Platform/Vulkan/PhysicalDevice.h"
#include "Platform/Vulkan/Pipeline.h"
#include "Platform/Vulkan/PipelineLayout.h"
#include "Platform/Vulkan/Queue.h"
#include "Platform/Vulkan/RenderContext.h"
#include "Platform/Vulkan/RenderPass.h"
#include "Platform/Vulkan/RenderTarget.h"
#include "Platform/Vulkan/Sampler.h"
#include "Platform/Vulkan/SemaphorePool.h"
#include "Platform/Vulkan/Shader.h"
#include "Platform/Vulkan/Submitter.h"
#include "Platform/Vulkan/Swapchain.h"
#include "Platform/Vulkan/Texture.h"
#include "Platform/Vulkan/Timeline.h"
#include "Platform/Vulkan/VideoSession.h"
#include "Platform/Vulkan/vk_mem_alloc.h"
#include "Platform/Vulkan/volk.h"

#ifdef _WIN32
#include "Platform/Windows/DirectWindow.h"
#include "Platform/Windows/NativeInput.h"
#endif

#include "Platform/Windows/GLFWInput.h"
#include "Platform/Windows/GLFWWindow.h"

#include "Render/Buffer.h"
#include "Render/Camera.h"
#include "Render/CommandBuffer.h"
#include "Render/DataSet.h"
#include "Render/Descriptor.h"
#include "Render/Frame.h"
#include "Render/GLSLCompiler.h"
#include "Render/Mesh.h"
#include "Render/OrthographicCamera.h"
#include "Render/Pipeline.h"
#include "Render/Queue.h"
#include "Render/Render.h"
#include "Render/Render2D.h"
#include "Render/RenderContext.h"
#include "Render/RenderTarget.h"
#include "Render/Shader.h"
#include "Render/Texture.h"
#include "Render/Types.h"

#include "Scene/Component.h"
#include "Scene/entt.hpp"
#include "Scene/GameObject.h"
#include "Scene/Object.h"
#include "Scene/ObserverCamera.h"
#include "Scene/Scene.h"
#include "Scene/SceneCamera.h"

#include "Script/ScriptEngine.h"

#include "Serializer/SceneSerializer.h"

#include "String/LanguageSettings.h"

#include"Sync/Semaphore.h"

#include "Vision/Audio/WAV.h"

#include "Vision/Common/Animator.h"
#include "Vision/Common/Animator.h"
#include "Vision/Common/BitTracker.h"
#include "Vision/Common/Checksum.h"
#include "Vision/Common/Error.h"
#include "Vision/Common/NetworkAbstractionLayer.h"
#include "Vision/Common/SamplingFactor.h"

#include "Vision/Demux/FFDemuxer.h"
#include "Vision/Demux/IVFDemuxer.h"

#include "Vision/External/stb_image.h"

#include "Vision/Image/BMP.h"
#include "Vision/Image/BMP.h"
#include "Vision/Image/Helper.h"
#include "Vision/Image/Image.h"
#include "Vision/Image/JPEG.h"
#include "Vision/Image/MFXJpegCodec.h"
#include "Vision/Image/OpenCVCodec.h"
#include "Vision/Image/PPM.h"
#include "Vision/Image/Raw.h"
#include "Vision/Image/STBCodec.h"

#include "Vision/Interface/Codec.h"
#include "Vision/Interface/Demuxer.h"
#include "Vision/Interface/MFXCodec.h"

#include "Vision/Processing/ColorSpace.h"

#include "Vision/Video/DAV1DCodec.h"
#include "Vision/Video/FFCodec.h"
#include "Vision/Video/HEVC.h"
#include "Vision/Video/Video.h"

#ifdef _WIN32
#include "Vision/Video/D3D12/HEVCCodec.h"
#endif

#include "Vision/Video/Vulkan/HEVC.h"

#include "Widget/MenuBar.h"
#include "Widget/Resource.h"
#include "Widget/Widget.h"
#include "Widget/WImageButton.h"
