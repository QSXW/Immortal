cmake_minimum_required(VERSION 3.21)

project(Immortal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CORE_FILES
    src/Immortal.h
    src/ImmortalCore.h)

set(FRAMEWORK_FILES
    src/Framework/Application.cpp
    src/Framework/Application.h
    src/Framework/Core.cpp
    src/Framework/Main.h
    src/Framework/Input.h
    src/Framework/KeyCodes.h
    src/Framework/Layer.cpp
    src/Framework/Layer.h
    src/Framework/LayerStack.cpp
    src/Framework/LayerStack.h
    src/Framework/Log.cpp
    src/Framework/Log.h
    src/Framework/Math.h
    src/Framework/Object.h
    src/Framework/Timer.h
    src/Framework/Utils.h
    src/Framework/Vector.cpp
    src/Framework/Vector.h
    src/Framework/Window.cpp
    src/Framework/Window.h
    src/Framework/Device.h
    src/Framework/ThreadManager.cpp
    src/Framework/ThreadManager.h)

set(EDITOR_FILES
    src/Editor/EditorCamera.cpp
    src/Editor/EditorCamera.h)

set(EVENT_FILES
    src/Event/ApplicationEvent.cpp
    src/Event/ApplicationEvent.h
    src/Event/Event.h
    src/Event/KeyEvent.h
    src/Event/MouseEvent.h)

set(IMAGE_FILES
)

set(GUI_FILES
    src/ImGui/GuiLayer.cpp
    src/ImGui/GuiLayer.h
    src/ImGui/ImGuiBuild.cpp
    src/ImGui/ImGuizmo.cpp
    src/ImGui/ImGuizmo.h
    src/ImGui/ui.h)

set(INTERFACE_FILES
   src/Interface/Delegate.cpp
   src/Interface/Delegate.h)

set(FS_FILES
    src/FileSystem/FileSystem.cpp
    src/FileSystem/FileSystem.h)

set(PHYSICS_FILES
    src/Physics/Physics.cpp
    src/Physics/Physics.h)

set(D3D12_FILES
    src/Platform/D3D12/Barrier.h
    src/Platform/D3D12/CommandAllocator.h
    src/Platform/D3D12/CommandBuffer.cpp
    src/Platform/D3D12/CommandBuffer.h
    src/Platform/D3D12/CommandPool.cpp
    src/Platform/D3D12/CommandPool.h
    src/Platform/D3D12/Common.h
    src/Platform/D3D12/DescriptorPool.cpp
    src/Platform/D3D12/DescriptorPool.h
    src/Platform/D3D12/Descriptor.cpp
    src/Platform/D3D12/Descriptor.h
    src/Platform/D3D12/Device.cpp
    src/Platform/D3D12/Device.h
    src/Platform/D3D12/FencePool.cpp
    src/Platform/D3D12/FencePool.h
    src/Platform/D3D12/Framebuffer.cpp
    src/Platform/D3D12/Framebuffer.h
    src/Platform/D3D12/Image.cpp
    src/Platform/D3D12/Image.h
    src/Platform/D3D12/ImageView.cpp
    src/Platform/D3D12/ImageView.h
    src/Platform/D3D12/Instance.cpp
    src/Platform/D3D12/Instance.h
    src/Platform/D3D12/PhysicalDevice.cpp
    src/Platform/D3D12/PhysicalDevice.h
    src/Platform/D3D12/Queue.cpp
    src/Platform/D3D12/Queue.h
    src/Platform/D3D12/RenderContext.cpp
    src/Platform/D3D12/RenderContext.h
    src/Platform/D3D12/Renderer.cpp
    src/Platform/D3D12/Renderer.h
    src/Platform/D3D12/RenderFrame.cpp
    src/Platform/D3D12/RenderFrame.h
    src/Platform/D3D12/RenderTarget.cpp
    src/Platform/D3D12/RenderTarget.h
    src/Platform/D3D12/SemaphorePool.cpp
    src/Platform/D3D12/SemaphorePool.h
    src/Platform/D3D12/Swapchain.cpp
    src/Platform/D3D12/Swapchain.h
    src/Platform/D3D12/Shader.cpp
    src/Platform/D3D12/Shader.h
    src/Platform/D3D12/Texture.cpp
    src/Platform/D3D12/Texture.h
    src/Platform/D3D12/GuiLayer.cpp
    src/Platform/D3D12/GuiLayer.h)

set(VULKAN_FILES
   src/Platform/Vulkan/vk_mem_alloc.h
   src/Platform/Vulkan/volk.h
   src/Platform/Vulkan/Buffer.cpp
   src/Platform/Vulkan/Buffer.h
   src/Platform/Vulkan/Common.cpp
   src/Platform/Vulkan/Common.h
   src/Platform/Vulkan/Device.cpp
   src/Platform/Vulkan/Device.h
   src/Platform/Vulkan/CommandBuffer.cpp
   src/Platform/Vulkan/CommandBuffer.h
   src/Platform/Vulkan/CommandPool.cpp
   src/Platform/Vulkan/CommandPool.h
   src/Platform/Vulkan/DescriptorPool.cpp
   src/Platform/Vulkan/DescriptorPool.h
   src/Platform/Vulkan/DescriptorSet.h
   src/Platform/Vulkan/FencePool.cpp
   src/Platform/Vulkan/FencePool.h
   src/Platform/Vulkan/Framebuffer.cpp
   src/Platform/Vulkan/Framebuffer.h
   src/Platform/Vulkan/GuiLayer.cpp
   src/Platform/Vulkan/GuiLayer.h
   src/Platform/Vulkan/Image.cpp
   src/Platform/Vulkan/Image.h
   src/Platform/Vulkan/ImageView.cpp
   src/Platform/Vulkan/ImageView.h
   src/Platform/Vulkan/Instance.cpp
   src/Platform/Vulkan/Instance.h
   src/Platform/Vulkan/Pipeline.cpp
   src/Platform/Vulkan/Pipeline.h
   src/Platform/Vulkan/PipelineLayout.h
   src/Platform/Vulkan/PhysicalDevice.cpp
   src/Platform/Vulkan/PhysicalDevice.h
   src/Platform/Vulkan/Queue.cpp
   src/Platform/Vulkan/Queue.h
   src/Platform/Vulkan/RenderContext.cpp
   src/Platform/Vulkan/RenderContext.h
   src/Platform/Vulkan/Renderer.cpp
   src/Platform/Vulkan/Renderer.h
   src/Platform/Vulkan/RenderFrame.cpp
   src/Platform/Vulkan/RenderFrame.h
   src/Platform/Vulkan/RenderTarget.cpp
   src/Platform/Vulkan/RenderTarget.h
   src/Platform/Vulkan/SemaphorePool.cpp
   src/Platform/Vulkan/SemaphorePool.h
   src/Platform/Vulkan/Swapchain.cpp
   src/Platform/Vulkan/Swapchain.h
   src/Platform/Vulkan/Sampler.cpp
   src/Platform/Vulkan/Sampler.h
   src/Platform/Vulkan/Shader.cpp
   src/Platform/Vulkan/Shader.h
   src/Platform/Vulkan/Texture.cpp
   src/Platform/Vulkan/Texture.h
   src/Platform/Vulkan/RenderPass.cpp
   src/Platform/Vulkan/RenderPass.h)

set(OPENGL_FILES
    src/Platform/OpenGL/Buffer.cpp
    src/Platform/OpenGL/Buffer.h
    src/Platform/OpenGL/Common.h
    src/Platform/OpenGL/Framebuffer.cpp
    src/Platform/OpenGL/Framebuffer.h
    src/Platform/OpenGL/GuiLayer.cpp
    src/Platform/OpenGL/GuiLayer.h
    src/Platform/OpenGL/RenderContext.cpp
    src/Platform/OpenGL/RenderContext.h
    src/Platform/OpenGL/Renderer.cpp
    src/Platform/OpenGL/Renderer.h
    src/Platform/OpenGL/Shader.cpp
    src/Platform/OpenGL/Shader.h
    src/Platform/OpenGL/Texture.cpp
    src/Platform/OpenGL/Texture.h
    src/Platform/OpenGL/VertexArray.cpp
    src/Platform/OpenGL/VertexArray.h)

set(WINDOWS_FILES
    src/Platform/Windows/DirectWindow.cpp
    src/Platform/Windows/DirectWindow.h
    src/Platform/Windows/GLFWWindow.cpp
    src/Platform/Windows/GLFWWindow.h
    src/Platform/Windows/WindowPlatformUtils.cpp
    src/Platform/Windows/WindowsInput.cpp
    src/Platform/Windows/WindowsInput.h)

set(RENDER_FILES
    src/Render/Buffer.cpp
    src/Render/Buffer.h
    src/Render/Camera.cpp
    src/Render/Camera.h
    src/Render/CommandBuffer.cpp
    src/Render/CommandBuffer.h
    src/Render/CommandQueue.cpp
    src/Render/CommandQueue.h
    src/Render/Environment.cpp
    src/Render/Environment.h
    src/Render/Frame.cpp
    src/Render/Frame.h
    src/Render/Format.h
    src/Render/Framebuffer.cpp
    src/Render/Framebuffer.h
    src/Render/Material.cpp
    src/Render/Material.h
    src/Render/Mesh.cpp
    src/Render/Mesh.h
    src/Render/OrthographicCamera.cpp
    src/Render/OrthographicCamera.h
    src/Render/OrthographicCameraController.cpp
    src/Render/OrthographicCameraController.h
    src/Render/Pipeline.cpp
    src/Render/Pipeline.h
    src/Render/Render.cpp
    src/Render/Render.h
    src/Render/RenderContext.cpp
    src/Render/RenderContext.h
    src/Render/Renderer.cpp
    src/Render/Renderer.h
    src/Render/Render2D.cpp
    src/Render/Render2D.h
    src/Render/Shader.cpp
    src/Render/Shader.h
    src/Render/Texture.cpp
    src/Render/Texture.h
    src/Render/VertexArray.cpp
    src/Render/VertexArray.h
    src/Render/GLSLCompiler.cpp
    src/Render/GLSLCompiler.h)

set(SCENE_FILES
    src/Scene/Component.cpp
    src/Scene/Component.h
    src/Scene/Entity.cpp
    src/Scene/Entity.h
    src/Scene/entt.hpp
    src/Scene/ObserverCamera.cpp
    src/Scene/ObserverCamera.h
    src/Scene/Scene.cpp
    src/Scene/Scene.h
    src/Scene/SceneCamera.cpp
    src/Scene/SceneCamera.h
    src/Scene/ScriptableObject.cpp
    src/Scene/ScriptableObject.h)

set(SCRIPT_FILES
    src/Script/ScriptDriver.cpp
    src/Script/ScriptDriver.h)

set(SERIALIZER_FILES
    src/Serializer/SceneSerializer.cpp
    src/Serializer/SceneSerializer.h)

set(SYNC_FILES
    src/Sync/Semaphore.cpp
    src/Sync/Semaphore.h
    src/Sync/SemaphorePool.cpp
    src/Sync/SemaphorePool.h)

set(UTILS_FILES
    src/Utils/nlohmann_json.h
    src/Utils/json.h
    src/Utils/PlatformUtils.h)

set(WIDGET_FILES
    src/Widget/MenuBar.h
    src/Widget/Widget.h
    src/Widget/Viewport.h)

set(NET_FILES
    src/Net/Socket.h)

source_group("\\" FILES ${CORE_FILES})
source_group("Framework\\" FILES ${FRAMEWORK_FILES})
source_group("Editor\\" FILES ${EDITOR_FILES})
source_group("Event\\" FILES ${EVENT_FILES})
source_group("Image\\" FILES ${IMAGE_FILES})
source_group("ImGui\\" FILES ${GUI_FILES})
source_group("Interface\\" FILES ${INTERFACE_FILES})
source_group("FileSystem\\" FILES ${FS_FILES})
source_group("Physics\\" FILES ${PHYSICS_FILES})
source_group("Platform\\D3D12\\" FILES ${D3D12_FILES})
source_group("Platform\\Vulkan\\" FILES ${VULKAN_FILES})
source_group("Platform\\OpenGL\\" FILES ${OPENGL_FILES})
source_group("Platform\\Windows\\" FILES ${WINDOWS_FILES})
source_group("Render\\" FILES ${RENDER_FILES})
source_group("Scene\\" FILES ${SCENE_FILES})
source_group("Script\\" FILES ${SCRIPT_FILES})
source_group("Serializer\\" FILES ${SERIALIZER_FILES})
source_group("Sync\\" FILES ${SYNC_FILES})
source_group("Utils\\" FILES ${UTILS_FILES})
source_group("Widget\\" FILES ${WIDGET_FILES})
source_group("Net\\" FILES ${NET_FILES})

set(PROJECT_FILES
    ${CORE_FILES}
    ${FRAMEWORK_FILES}
    ${EDITOR_FILES}
    ${EVENT_FILES}
    ${IMAGE_FILES}
    ${GUI_FILES}
    ${INTERFACE_FILES}
    ${IO_FILES}
    ${PHYSICS_FILES}
    ${D3D12_FILES}
    ${VULKAN_FILES}
    ${OPENGL_FILES}
    ${WINDOWS_FILES}
    ${RENDER_FILES}
    ${SCENE_FILES}
    ${SCRIPT_FILES}
    ${SERIALIZER_FILES}
    ${SYNC_FILES}
    ${UTILS_FILES}
    ${WIDGET_FILES}
    ${NET_FILES})

source_group("\\" FILES ${PROJECT_FILES})

add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_precompile_headers(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/impch.h>)

add_subdirectory(sl)
add_subdirectory(Media)
target_link_libraries(${PROJECT_NAME}
    slapi
    slMedia
    glm
    glad
    glfw
    # mono
    assimp
    vulkan
    spdlog::spdlog
    imgui
    opencv
    glslang
    glslang-default-resource-limits
    SPIRV)
