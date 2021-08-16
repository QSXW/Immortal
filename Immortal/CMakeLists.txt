cmake_minimum_required(VERSION 3.20)

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
    src/Framework/Assert.h
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
    src/Framework/Window.h)

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
   src/Image/ColorSpace.h
   src/Image/ColorSpace.cpp)

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

set(IO_FILES
    src/io/FileSystem.cpp
    src/io/FileSystem.h
    )

set(PHYSICS_FILES
    src/Physics/Physics.cpp
    src/Physics/Physics.h)

set(D3D12_FILES
   src/Platform/D3D12/D3D12Common.h
   src/Platform/D3D12/D3D12Framebuffer.cpp
   src/Platform/D3D12/D3D12Framebuffer.h)

set(VULKAN_FILES
   src/Platform/Vulkan/vk_mem_alloc.h
   src/Platform/Vulkan/volk.h
   src/Platform/Vulkan/vkcommon.cpp
   src/Platform/Vulkan/vkcommon.h
   src/Platform/Vulkan/Device.cpp
   src/Platform/Vulkan/Device.h
   src/Platform/Vulkan/CommandBuffer.cpp
   src/Platform/Vulkan/CommandBuffer.h
   src/Platform/Vulkan/CommandPool.cpp
   src/Platform/Vulkan/CommandPool.h
   src/Platform/Vulkan/FencePool.cpp
   src/Platform/Vulkan/FencePool.h
   src/Platform/Vulkan/VulkanFramebuffer.cpp
   src/Platform/Vulkan/VulkanFramebuffer.h
   src/Platform/Vulkan/VulkanGuiLayer.cpp
   src/Platform/Vulkan/VulkanGuiLayer.h
   src/Platform/Vulkan/Image.cpp
   src/Platform/Vulkan/Image.h
   src/Platform/Vulkan/ImageView.cpp
   src/Platform/Vulkan/ImageView.h
   src/Platform/Vulkan/Instance.cpp
   src/Platform/Vulkan/Instance.h
   src/Platform/Vulkan/PhysicalDevice.cpp
   src/Platform/Vulkan/PhysicalDevice.h
   src/Platform/Vulkan/Queue.cpp
   src/Platform/Vulkan/Queue.h
   src/Platform/Vulkan/RenderContext.cpp
   src/Platform/Vulkan/RenderContext.h
   src/Platform/Vulkan/Renderer.cpp
   src/Platform/Vulkan/Renderer.h
   src/Platform/Vulkan/RenderTarget.cpp
   src/Platform/Vulkan/RenderTarget.h
   src/Platform/Vulkan/Swapchain.cpp
   src/Platform/Vulkan/Swapchain.h)

set(OPENGL_FILES
    src/Platform/OpenGL/OpenGLBuffer.cpp
    src/Platform/OpenGL/OpenGLBuffer.h
    src/Platform/OpenGL/OpenGLFramebuffer.cpp
    src/Platform/OpenGL/OpenGLFramebuffer.h
    src/Platform/OpenGL/OpenGLGuiLayer.cpp
    src/Platform/OpenGL/OpenGLGuiLayer.h
    src/Platform/OpenGL/OpenGLRenderContext.cpp
    src/Platform/OpenGL/OpenGLRenderContext.h
    src/Platform/OpenGL/OpenGLRenderer.cpp
    src/Platform/OpenGL/OpenGLRenderer.h
    src/Platform/OpenGL/OpenGLShader.cpp
    src/Platform/OpenGL/OpenGLShader.h
    src/Platform/OpenGL/OpenGLTexture.cpp
    src/Platform/OpenGL/OpenGLTexture.h
    src/Platform/OpenGL/OpenGLVertexArray.cpp
    src/Platform/OpenGL/OpenGLVertexArray.h)

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
    src/Render/RenderCommand.cpp
    src/Render/RenderCommand.h
    src/Render/RenderContext.cpp
    src/Render/RenderContext.h
    src/Render/Renderer.cpp
    src/Render/Renderer.h
    src/Render/Renderer2D.cpp
    src/Render/Renderer2D.h
    src/Render/RendererAPI.cpp
    src/Render/RendererAPI.h
    src/Render/Shader.cpp
    src/Render/Shader.h
    src/Render/Texture.cpp
    src/Render/Texture.h
    src/Render/VertexArray.cpp
    src/Render/VertexArray.h)

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
    src/Serializer/json.h
    src/Serializer/SceneSerializer.cpp
    src/Serializer/SceneSerializer.h)

set(UTILS_FILES
    src/Utils/PlatformUtils.h)

source_group("\\" FILES ${CORE_FILES})
source_group("Framework\\" FILES ${FRAMEWORK_FILES})
source_group("Editor\\" FILES ${EDITOR_FILES})
source_group("Event\\" FILES ${EVENT_FILES})
source_group("Image\\" FILES ${IMAGE_FILES})
source_group("ImGui\\" FILES ${GUI_FILES})
source_group("Interface\\" FILES ${INTERFACE_FILES})
source_group("io\\" FILES ${IO_FILES})
source_group("Physics\\" FILES ${PHYSICS_FILES})
source_group("Platform\\D3D12\\" FILES ${D3D12_FILES})
source_group("Platform\\Vulkan\\" FILES ${VULKAN_FILES})
source_group("Platform\\OpenGL\\" FILES ${OPENGL_FILES})
source_group("Platform\\Windows\\" FILES ${WINDOWS_FILES})
source_group("Render\\" FILES ${RENDER_FILES})
source_group("Scene\\" FILES ${SCENE_FILES})
source_group("Script\\" FILES ${SCRIPT_FILES})
source_group("Serializer\\" FILES ${SERIALIZER_FILES})
source_group("Utils\\" FILES ${UTILS_FILES})

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
    ${UTILS_FILES})

source_group("\\" FILES ${PROJECT_FILES})

add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})

target_sources(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_FILES}
    )

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_precompile_headers(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/impch.h>)

target_link_libraries(${PROJECT_NAME}
    glm
    glad
    glfw
    mono
    assimp
    vulkan
    spdlog::spdlog
    imgui
    opencv)
