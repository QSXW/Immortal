cmake_minimum_required(VERSION 3.16)

set (CMAKE_VERBOSE_MAKEFILE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("ImmortalGraphics" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(D3D_FILES
    Interface.h
    Instance.cpp
    Instance.h
    PhysicalDevice.cpp
    PhysicalDevice.h)
list(TRANSFORM D3D_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/D3D/")

set(D3D11_FILES
    Buffer.cpp
    Buffer.h
    CommandBuffer.cpp
    CommandBuffer.h
    Common.h
    Descriptor.h
    DescriptorSet.cpp
    DescriptorSet.h
    Device.cpp
    Device.h
    Fence.cpp
    Fence.h
    GPUEvent.cpp
    GPUEvent.h
    Handle.h
    Image.cpp
    Image.h
    Instance.cpp
    Instance.h
    PhysicalDevice.cpp
    PhysicalDevice.h
    Pipeline.cpp
    Pipeline.h
    Queue.cpp
    Queue.h
    RenderTarget.cpp
    RenderTarget.h
    Swapchain.cpp
    Swapchain.h
    Sampler.cpp
    Sampler.h
    Shader.cpp
    Shader.h
    Texture.cpp
    Texture.h
    WindowCapture.cpp
    WindowCapture.h)

list(TRANSFORM D3D11_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/D3D11/")

set(D3D12_FILES
    Barrier.h
    AccelerationStructure.cpp
    AccelerationStructure.h
    Buffer.cpp
    Buffer.h
    CommandAllocator.cpp
    CommandAllocator.h
    CommandBuffer.cpp
    CommandBuffer.h
    CommandList.cpp
    CommandList.h
    Common.h
    DescriptorHeap.cpp
    DescriptorHeap.h
    DescriptorPool.cpp
    DescriptorPool.h
    Descriptor.cpp
    Descriptor.h
    DescriptorSet.cpp
    DescriptorSet.h
    Device.cpp
    Device.h
    Event.cpp
    Event.h
    Fence.cpp
    Fence.h
    Handle.h
    Instance.cpp
    Instance.h
    PhysicalDevice.cpp
    PhysicalDevice.h
    Pipeline.cpp
    Pipeline.h
    Queue.cpp
    Queue.h
    RenderTarget.cpp
    RenderTarget.h
    Resource.h
    RootSignature.h
    Swapchain.cpp
    Swapchain.h
    Sampler.cpp
    Sampler.h
    Shader.cpp
    Shader.h
    Texture.cpp
    Texture.h
    VideoCommon.h
    VideoDecodeCommandList.h
    VideoDecoder.cpp
    VideoDecoder.h
    VideoDecoderHeap.h
    VideoDevice.cpp
    VideoDevice.h)

list(TRANSFORM D3D12_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/D3D12/")

set(OPENGL_FILES
    Buffer.cpp
    Buffer.h
    CommandBuffer.cpp
    CommandBuffer.h
    Common.h
    Descriptor.h
    DescriptorSet.cpp
    DescriptorSet.h
    Device.cpp
    Device.h
    GLFormat.h
    GPUEvent.cpp
    GPUEvent.h
    Instance.cpp
    Instance.h
    PhysicalDevice.cpp
    PhysicalDevice.h
    RenderTarget.cpp
    RenderTarget.h
    Pipeline.cpp
    Pipeline.h
    Queue.cpp
    Queue.h
    Sampler.cpp
    Sampler.h
    Shader.cpp
    Shader.h
    Swapchain.cpp
    Swapchain.h
    Texture.cpp
    Texture.h
    VertexArray.h)

list(TRANSFORM OPENGL_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/")

set(VULKAN_FILES
   AccelerationStructure.cpp
   AccelerationStructure.h
   Attachment.h
   Barrier.h
   Buffer.cpp
   Buffer.h
   Common.cpp
   Common.h
   Device.cpp
   Device.h
   CommandBuffer.cpp
   CommandBuffer.h
   CommandPool.cpp
   CommandPool.h
   Descriptor.h
   DescriptorPool.cpp
   DescriptorPool.h
   DescriptorSet.cpp
   DescriptorSet.h
   FencePool.cpp
   FencePool.h
   Framebuffer.cpp
   Framebuffer.h
   GPUEvent.cpp
   GPUEvent.h
   Handle.h
   Image.cpp
   Image.h
   ImageView.cpp
   ImageView.h
   Instance.cpp
   Instance.h
   Pipeline.cpp
   Pipeline.h
   PipelineLayout.h
   PhysicalDevice.cpp
   PhysicalDevice.h
   Queue.cpp
   Queue.h
   RenderTarget.cpp
   RenderTarget.h
   SemaphorePool.cpp
   SemaphorePool.h
   Swapchain.cpp
   Swapchain.h
   Sampler.cpp
   Sampler.h
   Semaphore.h
   Shader.cpp
   Shader.h
   Submitter.h
   Surface.cpp
   Surface.h
   Texture.cpp
   Texture.h
   TimelineCommandBuffer.cpp
   TimelineCommandBuffer.h
   TimelineCommandPool.cpp
   TimelineCommandPool.h
   VideoSession.cpp
   VideoSession.h
   RenderPass.cpp
   RenderPass.h
   VmaUsage.cpp
   VmaUsage.h
   volk.h)

list(TRANSFORM VULKAN_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Vulkan/")

set(METAL_FILES
    AccelerationStructure.cpp
    AccelerationStructure.h
    Buffer.cpp
    Buffer.h
    CommandBuffer.cpp
    CommandBuffer.h
    Common.h
    DescriptorSet.cpp
    DescriptorSet.h
    Device.cpp
    Device.h
    GPUEvent.cpp
    GPUEvent.h
    Handle.h
    Instance.cpp
    Instance.h
    PhysicalDevice.cpp
    PhysicalDevice.h
    Pipeline.cpp
    Pipeline.h
    Queue.cpp
    Queue.h
    RenderTarget.cpp
    RenderTarget.h
    Sampler.cpp
    Sampler.h
    Shader.cpp
    Shader.h
    Swapchain.mm
    Swapchain.h
    Texture.cpp
    Texture.h)

list(TRANSFORM METAL_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Metal/")

set(WINDOW_FILES
    GLFWWindow.cpp
    GLFWWindow.h
    GLFWInput.cpp
    GLFWInput.h)

set(GRAPHICS_FILES
    AccelerationStructure.h
    AsyncCompute.cpp
    AsyncCompute.h
    Buffer.cpp
    Buffer.h
    CommandBuffer.h
    Descriptor.h
    DescriptorSet.h
    Device.h
    DirectXShaderCompiler.cpp
    DirectXShaderCompiler.h
    GPUEvent.h
    Format.cpp
    Format.h
    GLSLCompiler.cpp
    GLSLCompiler.h
    Instance.cpp
    Instance.h
    LightGraphics.h
    RenderTarget.h
    PhysicalDevice.h
    Pipeline.h
    Queue.cpp
    Queue.h
    GraphicsPrecompiledHeader.h
    Input.cpp
    Input.h
    Sampler.h
    Shader.h
    Swapchain.h
    Texture.cpp
    Texture.h
    Types.h
    Window.cpp
    Window.h
    WindowCapture.h)

list(TRANSFORM GRAPHICS_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

if (WIN32)
    list(APPEND WINDOW_FILES
        DirectWindow.cpp
        DirectWindow.h
        NativeInput.cpp
        NativeInput.h)
endif()
list(TRANSFORM WINDOW_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Window/")

set(EVENT_FILES
    ApplicationEvent.h
    Event.h
    KeyCodes.h
    KeyEvent.h
    MouseEvent.h)

list(TRANSFORM EVENT_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Event/")

source_group("\\"          FILES ${GRAPHICS_FILES})
source_group("\\Event\\"   FILES ${EVENT_FILES}   )
source_group("\\D3D\\"     FILES ${D3D_FILES}     )
source_group("\\D3D11\\"   FILES ${D3D11_FILES}   )
source_group("\\D3D12\\"   FILES ${D3D12_FILES}   )
source_group("\\Vulkan\\"  FILES ${VULKAN_FILES}  )
source_group("\\Metal\\"   FILES ${METAL_FILES}   )
source_group("\\OpenGL\\"  FILES ${OPENGL_FILES}  )
source_group("\\Windows\\" FILES ${WINDOW_FILES}  )

add_library(${PROJECT_NAME} STATIC
    ${GRAPHICS_FILES}
    ${EVENT_FILES}
    $<$<PLATFORM_ID:Windows>:${D3D_FILES}>
    $<$<PLATFORM_ID:Windows>:${D3D11_FILES}>
    $<$<PLATFORM_ID:Windows>:${D3D12_FILES}>
    $<$<PLATFORM_ID:Darwin>:${METAL_FILES}>
    ${VULKAN_FILES}
    ${OPENGL_FILES}
    ${WINDOW_FILES})

if (NOT DISABLE_VULKAN_BACKEND)
    set(VULKAN_DEPENDENCIES glslang glslang-default-resource-limits
        SPIRV spirv-cross-glsl)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw ${VULKAN_DEPENDENCIES} $<$<PLATFORM_ID:Darwin>:spirv-cross-msl>
    PUBLIC
    slapi
    glad
    Vulkan-Headers GPUOpen::VulkanMemoryAllocator
    $<$<PLATFORM_ID:Darwin>:metal-cpp>
    DirectXShaderCompilerHeaders
    ImmortalShared
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(${PROJECT_NAME} PUBLIC "VK_ENABLE_BETA_EXTENSIONS")

if (NOT APPLE)
    target_precompile_headers(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/GraphicsPrecompiledHeader.h>)
endif()
