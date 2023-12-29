cmake_minimum_required(VERSION 3.16)

project(Immortal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CORE_FILES
    Immortal.h
    Core.h
    Format.h
    impch.cpp
    impch.h)

set(ALGORITHM_FILES
    Algorithm/LightArray.h
    Algorithm/LightVector.h
    Algorithm/Rotate.h)

set(AUDIO_FILES
    AudioRenderContext.cpp
    AudioRenderContext.h
    AudioSource.cpp
    AudioSource.h
    Device.cpp
    Device.h)

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
    ImGui/imgui_impl_immortal.cpp
    ImGui/imgui_impl_immortal.h
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
    Allocator.h
    Memory.cpp
    Memory.h
    MemoryAllocator.cpp
    MemoryAllocator.h
    MemoryResource.cpp
    MemoryResource.h)

list(TRANSFORM MEMORY_FILES PREPEND "Memory/")

set(FS_FILES
    FileSystem/FileSystem.cpp
    FileSystem/FileSystem.h
    FileSystem/RF.h
    FileSystem/Stream.h)

set(PHYSICS_FILES
    Physics/Physics.cpp
    Physics/Physics.h)

set(D3D_FILES
    Platform/D3D/Interface.h
    Platform/D3D/Instance.cpp
    Platform/D3D/Instance.h
    Platform/D3D/PhysicalDevice.cpp
    Platform/D3D/PhysicalDevice.h)

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

list(TRANSFORM D3D11_FILES PREPEND "Platform/D3D11/")

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

list(TRANSFORM D3D12_FILES PREPEND "Platform/D3D12/")

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

list(TRANSFORM OPENGL_FILES PREPEND "Platform/OpenGL/")

set(VULKAN_FILES
   vk_mem_alloc.h
   volk.h
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
   RenderPass.h)

list(TRANSFORM VULKAN_FILES PREPEND "Platform/Vulkan/")

set(WINDOWS_FILES
    GLFWWindow.cpp
    GLFWWindow.h
    GLFWInput.cpp
    GLFWInput.h)

set(RENDER_FILES
    AccelerationStructure.h
    AsyncCompute.cpp
    AsyncCompute.h
    Buffer.cpp
    Buffer.h
    Camera.cpp
    Camera.h
    CommandBuffer.h
    DataSet.h
    Descriptor.h
    DescriptorSet.h
    Device.h
    DirectXShaderCompiler.cpp
    DirectXShaderCompiler.h
    GPUEvent.h
    Frame.cpp
    Frame.h
    GLSLCompiler.cpp
    GLSLCompiler.h
    Instance.cpp
    Instance.h
    LightGraphics.h
    RenderTarget.h
    Mesh.cpp
    Mesh.h
    OrthographicCamera.cpp
    OrthographicCamera.h
    PhysicalDevice.h
    Pipeline.h
    Queue.cpp
    Queue.h
    Graphics.cpp
    Graphics.h
    Render2D.cpp
    Render2D.h
    Sampler.h
    Shader.h
    Swapchain.h
    Texture.cpp
    Texture.h
    Types.h
    WindowCapture.h)

list(TRANSFORM RENDER_FILES PREPEND "Render/")

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
    Scene/SceneCamera.h
    Scene/ScriptComponent.cpp
    Scene/SpriteRendererComponent.cpp
    Scene/VideoComponent.cpp)

set(SCRIPT_FILES
    Script/ScriptEngine.cpp
    Script/ScriptEngine.h)

set(SERIALIZER_FILES
    Serializer/SceneSerializer.cpp
    Serializer/SceneSerializer.h)

set(STRING_FILES
    String/IString.h
    String/LanguageSettings.cpp
    String/LanguageSettings.h)

set(SYNC_FILES
    Semaphore.h)

set(HELPER_FILES
    Arguments.cpp
    Arguments.h
    nlohmann_json.h
    json.h
    Platform.h)

set(WIDGET_FILES
    MenuBar.h
    Resource.h
    WFileDialog.h
    Widget.cpp
    Widget.h
    WImageButton.h
    WSlider.h)

list(TRANSFORM WIDGET_FILES PREPEND "Widget/")

set(NET_FILES
    Socket.h
    TCP.h
    LTP.h)

if (WIN32)
    list(APPEND NET_FILES
        Win32Socket.cpp)

    list(APPEND AUDIO_FILES
        WASAPI.cpp
        WASAPI.h)

    list(APPEND HELPER_FILES
        Win32Platform.cpp)

    list(APPEND WINDOWS_FILES
        DirectWindow.cpp
        DirectWindow.h
        NativeInput.cpp
        NativeInput.h)

    list(APPEND SYNC_FILES
        Win32Semaphore.cpp)
else()
    list(APPEND NET_FILES
        UnixSocket.cpp)

    list(APPEND AUDIO_FILES
        ALSA.cpp
        ALSA.h)

    list(APPEND HELPER_FILES
        LinuxPlatform.cpp)

    list(APPEND SYNC_FILES
        UnixSemaphore.cpp)
endif()

list(TRANSFORM AUDIO_FILES   PREPEND "Audio/"           )
list(TRANSFORM HELPER_FILES  PREPEND "Helper/"          )
list(TRANSFORM NET_FILES     PREPEND "Net/"             )
list(TRANSFORM SYNC_FILES    PREPEND "Sync/"            )
list(TRANSFORM WINDOWS_FILES PREPEND "Platform/Windows/")

source_group("\\"                  FILES ${CORE_FILES}      )
source_group("Algorithm\\"         FILES ${ALGORITHM_FILES} )
source_group("Audio\\"             FILES ${AUDIO_FILES}     )
source_group("Framework\\"         FILES ${FRAMEWORK_FILES} )
source_group("Editor\\"            FILES ${EDITOR_FILES}    )
source_group("Event\\"             FILES ${EVENT_FILES}     )
source_group("Image\\"             FILES ${IMAGE_FILES}     )
source_group("ImGui\\"             FILES ${GUI_FILES}       )
source_group("Interface\\"         FILES ${INTERFACE_FILES} )
source_group("Math\\"              FILES ${MATH_FILES}      )
source_group("Memory\\"            FILES ${MEMORY_FILES}    )
source_group("FileSystem\\"        FILES ${FS_FILES}        )
source_group("Physics\\"           FILES ${PHYSICS_FILES}   )
source_group("Platform\\D3D\\"     FILES ${D3D_FILES}       )
source_group("Platform\\D3D11\\"   FILES ${D3D11_FILES}     )
source_group("Platform\\D3D12\\"   FILES ${D3D12_FILES}     )
source_group("Platform\\Vulkan\\"  FILES ${VULKAN_FILES}    )
source_group("Platform\\OpenGL\\"  FILES ${OPENGL_FILES}    )
source_group("Platform\\Windows\\" FILES ${WINDOWS_FILES}   )
source_group("Render\\"            FILES ${RENDER_FILES}    )
source_group("Scene\\"             FILES ${SCENE_FILES}     )
source_group("Script\\"            FILES ${SCRIPT_FILES}    )
source_group("Serializer\\"        FILES ${SERIALIZER_FILES})
source_group("String\\"            FILES ${STRING_FILES}    )
source_group("Sync\\"              FILES ${SYNC_FILES}      )
source_group("Helper\\"            FILES ${HELPER_FILES}    )
source_group("Widget\\"            FILES ${WIDGET_FILES}    )
source_group("Net\\"               FILES ${NET_FILES}       )

set(VISION_CORE_FILES
    Vision/Types.h)

set(VISION_AUDIO_FILES
    Vision/Audio/WAV.cpp
    Vision/Audio/WAV.h)

set(COMMON_FILES
    Vision/Common/Animator.h
    Vision/Common/BitTracker.h
    Vision/Common/Checksum.cpp
    Vision/Common/Checksum.h
    Vision/Common/Error.h
    Vision/Common/NetworkAbstractionLayer.h
    Vision/Common/SamplingFactor.h)

set(DEMUX_FILES
    Vision/Demux/FFDemuxer.cpp
    Vision/Demux/FFDemuxer.h
    Vision/Demux/IVFDemuxer.cpp
    Vision/Demux/IVFDemuxer.h)

set(VISION_INTERFACE_FILES
    Vision/Interface/Codec.h
    Vision/Interface/Demuxer.h
    Vision/Interface/MFXCodec.cpp
    Vision/Interface/MFXCodec.h)

set(LOOKUP_TABLE_FILE
    Vision/LookupTable/LookupTable.cpp
    Vision/LookupTable/LookupTable.h)

set(IMAGE_CODEC_FILES
    Vision/Image/Helper.h
    Vision/Image/BMP.cpp
    Vision/Image/BMP.h
    Vision/Image/Image.h
    Vision/Image/JPEG.cpp
    Vision/Image/JPEG.h
    Vision/Image/STBCodec.cpp
    Vision/Image/STBCodec.h
    Vision/Image/MFXJpegCodec.cpp
    Vision/Image/MFXJpegCodec.h
    Vision/Image/PPM.cpp
    Vision/Image/PPM.h
    Vision/Image/Raw.cpp
    Vision/Image/Raw.h
    Vision/Image/OpenCVCodec.cpp
    Vision/Image/OpenCVCodec.h)

set(VIDEO_CODEC_FILES
    Vision/Video/DAV1DCodec.cpp
    Vision/Video/DAV1DCodec.h
    Vision/Video/FFCodec.cpp
    Vision/Video/FFCodec.h
    Vision/Video/HEVC.cpp
    Vision/Video/HEVC.h
    Vision/Video/Video.h)

set(D3D12_VIDEO_FILES
    Vision/Video/D3D12/HEVCCodec.cpp
    Vision/Video/D3D12/HEVCCodec.h)

set(VULKAN_VIDEO_FILES
    Vision/Video/Vulkan/HEVC.cpp
    Vision/Video/Vulkan/HEVC.h)

set(PROCESSING_FILES
    Vision/Processing/ColorSpace.cpp
    Vision/Processing/ColorSpace.h)

set(EXTERNAL_FILES
    Vision/External/External.cpp
    Vision/External/stb_image.h)

source_group("\\Vision\\"                FILES ${MEDIA_CORE_FILES}      )
source_group("\\Vision\\Audio\\"         FILES ${VISION_AUDIO_FILES}    )
source_group("\\Vision\\Common\\"        FILES ${COMMON_FILES}          )
source_group("\\Vision\\Demux\\"         FILES ${DEMUX_FILES}           )
source_group("\\Vision\\Interface\\"     FILES ${VISION_INTERFACE_FILES})
source_group("\\Vision\\LookupTable\\"   FILES ${LOOKUP_TABLE_FILE}     )
source_group("\\Vision\\Image\\"         FILES ${IMAGE_CODEC_FILES}     )
source_group("\\Vision\\Video\\"         FILES ${VIDEO_CODEC_FILES}     )
source_group("\\Vision\\Video\\D3D12\\"  FILES ${D3D12_VIDEO_FILES}     )
source_group("\\Vision\\Video\\Vulkan\\" FILES ${VULKAN_VIDEO_FILES}    )
source_group("\\Vision\\External\\"      FILES ${EXTERNAL_FILES}        )
source_group("\\Vision\\Processing\\"    FILES ${PROCESSING_FILES}      )

set(VISION_FILES
    ${VISION_CORE_FILES}
    ${VISION_AUDIO_FILES}
    ${COMMON_FILES}
    ${DEMUX_FILES}
    ${EXTERNAL_FILES}
    ${VISION_INTERFACE_FILES}
    ${LOOKUP_TABLE_FILE}
    ${IMAGE_CODEC_FILES}
    ${VIDEO_CODEC_FILES}
    ${VULKAN_VIDEO_FILES}
    ${PROCESSING_FILES}
    $<$<C_COMPILER_ID:MSVC>:${D3D12_VIDEO_FILES}>)

set(PROJECT_FILES
    ${CORE_FILES}
    ${ALGORITHM_FILES}
    ${AUDIO_FILES}
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
    ${OPENGL_FILES}
    ${VULKAN_FILES}
    ${WINDOWS_FILES}
    ${RENDER_FILES}
    ${SCENE_FILES}
    ${SCRIPT_FILES}
    ${SERIALIZER_FILES}
    ${STRING_FILES}
    ${SYNC_FILES}
    ${HELPER_FILES}
    ${WIDGET_FILES}
    ${NET_FILES}
    ${VISION_FILES}
    $<$<PLATFORM_ID:Windows>:${D3D_FILES}>
    $<$<PLATFORM_ID:Windows>:${D3D11_FILES}>
    $<$<PLATFORM_ID:Windows>:${D3D12_FILES}>
)

source_group("\\" FILES ${PROJECT_FILES})

list(TRANSFORM PROJECT_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})

target_sources(${PROJECT_NAME} PUBLIC ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(${PROJECT_NAME} PUBLIC "VK_ENABLE_BETA_EXTENSIONS")

target_precompile_headers(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/impch.h>)

add_subdirectory(slapi)

target_link_libraries(${PROJECT_NAME} PUBLIC
    slapi
    external
)
