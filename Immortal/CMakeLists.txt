cmake_minimum_required(VERSION 3.16)

project(Immortal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CORE_FILES
    Immortal.h
    Core.h
    Format.h)

set(FRAMEWORK_FILES
    Framework/Application.cpp
    Framework/Application.h
    Framework/DLLLoader.cpp
    Framework/DLLLoader.h
    Framework/Input.cpp
    Framework/Input.h
    Framework/KeyCodes.h
    Framework/Layer.cpp
    Framework/Layer.h
    Framework/LayerStack.cpp
    Framework/LayerStack.h
    Framework/Log.cpp
    Framework/Log.h
    Framework/Timer.h
    Framework/Utils.h
    Framework/Window.cpp
    Framework/Window.h
    Framework/Device.h
    Framework/Async.cpp
    Framework/Async.h)

set(EDITOR_FILES
    Editor/EditorCamera.cpp
    Editor/EditorCamera.h)

set(EVENT_FILES
    Event/ApplicationEvent.h
    Event/Event.h
    Event/KeyEvent.h
    Event/MouseEvent.h)

set(IMAGE_FILES
)

set(GUI_FILES
    ImGui/GuiLayer.cpp
    ImGui/GuiLayer.h
    ImGui/ImGuiBuild.cpp
    ImGui/ImGuizmo.cpp
    ImGui/ImGuizmo.h
    ImGui/Utils.h)

set(INTERFACE_FILES
   Interface/Delegate.cpp
   Interface/Delegate.h
   Interface/IObject.h)

set(MATH_FILES
   Math/Math.h
   Math/Vector.cpp
   Math/Vector.h)

set(MEMORY_FILES
    Memory/Allocator.h
    Memory/Memory.cpp
    Memory/Memory.h
    )

set(FS_FILES
    FileSystem/FileSystem.cpp
    FileSystem/FileSystem.h)

set(PHYSICS_FILES
    Physics/Physics.cpp
    Physics/Physics.h)

set(D3D12_FILES
    Platform/D3D12/Barrier.h
    Platform/D3D12/Buffer.cpp
    Platform/D3D12/Buffer.h
    Platform/D3D12/CommandAllocator.h
    Platform/D3D12/CommandBuffer.cpp
    Platform/D3D12/CommandBuffer.h
    Platform/D3D12/CommandPool.cpp
    Platform/D3D12/CommandPool.h
    Platform/D3D12/Common.h
    Platform/D3D12/DescriptorHeap.cpp
    Platform/D3D12/DescriptorHeap.h
    Platform/D3D12/Descriptor.cpp
    Platform/D3D12/Descriptor.h
    Platform/D3D12/Device.cpp
    Platform/D3D12/Device.h
    Platform/D3D12/Fence.cpp
    Platform/D3D12/Fence.h
    Platform/D3D12/GuiLayer.cpp
    Platform/D3D12/GuiLayer.h
    Platform/D3D12/Pipeline.cpp
    Platform/D3D12/Pipeline.h
    Platform/D3D12/Queue.cpp
    Platform/D3D12/Queue.h
    Platform/D3D12/RenderContext.cpp
    Platform/D3D12/RenderContext.h
    Platform/D3D12/Renderer.cpp
    Platform/D3D12/Renderer.h
    Platform/D3D12/RenderTarget.cpp
    Platform/D3D12/RenderTarget.h
    Platform/D3D12/Resource.h
    Platform/D3D12/RootSignature.h
    Platform/D3D12/Swapchain.cpp
    Platform/D3D12/Swapchain.h
    Platform/D3D12/Shader.cpp
    Platform/D3D12/Shader.h
    Platform/D3D12/Texture.cpp
    Platform/D3D12/Texture.h)

set(VULKAN_FILES
   Platform/Vulkan/vk_mem_alloc.h
   Platform/Vulkan/volk.h
   Platform/Vulkan/Attachment.h
   Platform/Vulkan/Barrier.h
   Platform/Vulkan/Buffer.cpp
   Platform/Vulkan/Buffer.h
   Platform/Vulkan/Common.cpp
   Platform/Vulkan/Common.h
   Platform/Vulkan/Device.cpp
   Platform/Vulkan/Device.h
   Platform/Vulkan/CommandBuffer.cpp
   Platform/Vulkan/CommandBuffer.h
   Platform/Vulkan/CommandPool.cpp
   Platform/Vulkan/CommandPool.h
   Platform/Vulkan/Descriptor.h
   Platform/Vulkan/DescriptorPool.cpp
   Platform/Vulkan/DescriptorPool.h
   Platform/Vulkan/DescriptorSet.cpp
   Platform/Vulkan/DescriptorSet.h
   Platform/Vulkan/FencePool.cpp
   Platform/Vulkan/FencePool.h
   Platform/Vulkan/Framebuffer.cpp
   Platform/Vulkan/Framebuffer.h
   Platform/Vulkan/GuiLayer.cpp
   Platform/Vulkan/GuiLayer.h
   Platform/Vulkan/Image.cpp
   Platform/Vulkan/Image.h
   Platform/Vulkan/ImageView.cpp
   Platform/Vulkan/ImageView.h
   Platform/Vulkan/Instance.cpp
   Platform/Vulkan/Instance.h
   Platform/Vulkan/Pipeline.cpp
   Platform/Vulkan/Pipeline.h
   Platform/Vulkan/PipelineLayout.h
   Platform/Vulkan/PhysicalDevice.cpp
   Platform/Vulkan/PhysicalDevice.h
   Platform/Vulkan/Queue.cpp
   Platform/Vulkan/Queue.h
   Platform/Vulkan/RenderContext.cpp
   Platform/Vulkan/RenderContext.h
   Platform/Vulkan/Renderer.cpp
   Platform/Vulkan/Renderer.h
   Platform/Vulkan/RenderTarget.cpp
   Platform/Vulkan/RenderTarget.h
   Platform/Vulkan/SemaphorePool.cpp
   Platform/Vulkan/SemaphorePool.h
   Platform/Vulkan/Swapchain.cpp
   Platform/Vulkan/Swapchain.h
   Platform/Vulkan/Sampler.cpp
   Platform/Vulkan/Sampler.h
   Platform/Vulkan/Shader.cpp
   Platform/Vulkan/Shader.h
   Platform/Vulkan/Texture.cpp
   Platform/Vulkan/Texture.h
   Platform/Vulkan/VideoSession.cpp
   Platform/Vulkan/VideoSession.h
   Platform/Vulkan/RenderPass.cpp
   Platform/Vulkan/RenderPass.h)

set(OPENGL_FILES
    Platform/OpenGL/Buffer.cpp
    Platform/OpenGL/Buffer.h
    Platform/OpenGL/Common.h
    Platform/OpenGL/Descriptor.h
    Platform/OpenGL/Framebuffer.cpp
    Platform/OpenGL/Framebuffer.h
    Platform/OpenGL/GuiLayer.cpp
    Platform/OpenGL/GuiLayer.h
    Platform/OpenGL/RenderContext.cpp
    Platform/OpenGL/RenderContext.h
    Platform/OpenGL/Renderer.cpp
    Platform/OpenGL/Renderer.h
    Platform/OpenGL/RenderTarget.cpp
    Platform/OpenGL/RenderTarget.h
    Platform/OpenGL/Shader.cpp
    Platform/OpenGL/Shader.h
    Platform/OpenGL/Pipeline.cpp
    Platform/OpenGL/Pipeline.h
    Platform/OpenGL/Texture.cpp
    Platform/OpenGL/Texture.h
    Platform/OpenGL/VertexArray.h)

set(WINDOWS_FILES
    Platform/Windows/GLFWWindow.cpp
    Platform/Windows/GLFWWindow.h
    Platform/Windows/GLFWInput.cpp
    Platform/Windows/GLFWInput.h
    Platform/Windows/WindowPlatformUtils.cpp)

set(WIN32_WINDOES_FILES
    Platform/Windows/DirectWindow.cpp
    Platform/Windows/DirectWindow.h
    Platform/Windows/NativeInput.cpp
    Platform/Windows/NativeInput.h)

set(WINDOWS_FILES
    ${WINDOWS_FILES}
    $<$<C_COMPILER_ID:MSVC>:${WIN32_WINDOES_FILES}>)

set(RENDER_FILES
    Render/Buffer.h
    Render/Camera.cpp
    Render/Camera.h
    Render/CommandBuffer.cpp
    Render/CommandBuffer.h
    Render/DataSet.h
    Render/Descriptor.h
    Render/Environment.cpp
    Render/Environment.h
    Render/Frame.cpp
    Render/Frame.h
    Render/RenderTarget.h
    Render/Mesh.cpp
    Render/Mesh.h
    Render/OrthographicCamera.cpp
    Render/OrthographicCamera.h
    Render/Pipeline.h
    Render/Queue.h
    Render/Render.cpp
    Render/Render.h
    Render/RenderContext.cpp
    Render/RenderContext.h
    Render/Renderer.cpp
    Render/Renderer.h
    Render/Render2D.cpp
    Render/Render2D.h
    Render/Shader.h
    Render/Texture.h
    Render/Types.h
    Render/GLSLCompiler.cpp
    Render/GLSLCompiler.h)

set(SCENE_FILES
    Scene/Component.cpp
    Scene/Component.h
    Scene/Object.h
    Scene/entt.hpp
    Scene/GameObject.h
    Scene/ObserverCamera.cpp
    Scene/ObserverCamera.h
    Scene/Scene.cpp
    Scene/Scene.h
    Scene/SceneCamera.cpp
    Scene/SceneCamera.h)

set(SCRIPT_FILES
    Script/ScriptEngine.cpp
    Script/ScriptEngine.h)

set(SERIALIZER_FILES
    Serializer/SceneSerializer.cpp
    Serializer/SceneSerializer.h)

set(STRING_FILES
    String/LanguageSettings.cpp
    String/LanguageSettings.h)

set(SYNC_FILES
    Sync/Semaphore.cpp
    Sync/Semaphore.h
    Sync/SemaphorePool.cpp
    Sync/SemaphorePool.h)

set(UTILS_FILES
    Utils/nlohmann_json.h
    Utils/json.h
    Utils/PlatformUtils.h)

set(WIDGET_FILES
    Widget/MenuBar.h
    Widget/Widget.h
    Widget/Viewport.h)

set(NET_FILES
    Net/Socket.h)

source_group("\\" FILES ${CORE_FILES})
source_group("Framework\\" FILES ${FRAMEWORK_FILES})
source_group("Editor\\" FILES ${EDITOR_FILES})
source_group("Event\\" FILES ${EVENT_FILES})
source_group("Image\\" FILES ${IMAGE_FILES})
source_group("ImGui\\" FILES ${GUI_FILES})
source_group("Interface\\" FILES ${INTERFACE_FILES})
source_group("Math\\" FILES ${MATH_FILES})
source_group("Memory\\" FILES ${MEMORY_FILES})
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
source_group("String\\" FILES ${STRING_FILES})
source_group("Sync\\" FILES ${SYNC_FILES})
source_group("Utils\\" FILES ${UTILS_FILES})
source_group("Widget\\" FILES ${WIDGET_FILES})
source_group("Net\\" FILES ${NET_FILES})

set(PROJECT_FILES
    ${CORE_FILES}
    ${FRAMEWORK_FILES}
    ${EDITOR_FILES}
    ${EVENT_FILES}
    ${FS_FILES}
    ${IMAGE_FILES}
    ${GUI_FILES}
    ${INTERFACE_FILES}
    ${MATH_FILES}
    ${MEMORY_FILES}
    ${IO_FILES}
    ${PHYSICS_FILES}
    ${VULKAN_FILES}
    ${OPENGL_FILES}
    ${WINDOWS_FILES}
    ${RENDER_FILES}
    ${SCENE_FILES}
    ${SCRIPT_FILES}
    ${SERIALIZER_FILES}
    ${STRING_FILES}
    ${SYNC_FILES}
    ${UTILS_FILES}
    ${WIDGET_FILES}
    ${NET_FILES}
    $<$<C_COMPILER_ID:MSVC>:${D3D12_FILES}>
)

source_group("\\" FILES ${PROJECT_FILES})

list(TRANSFORM PROJECT_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})

target_sources(${PROJECT_NAME} PUBLIC ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/impch.h>)

add_subdirectory(sl)
add_subdirectory(Media)

target_link_libraries(${PROJECT_NAME}
    slapi
    Vision
    external
)
