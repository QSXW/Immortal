cmake_minimum_required(VERSION 3.16)

project(Immortal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CORE_FILES
    Immortal.h
    Core.h
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
    Framework/Layer.cpp
    Framework/Layer.h
    Framework/LayerStack.cpp
    Framework/LayerStack.h
    Framework/Timer.h
    Framework/Utils.h)

set(EDITOR_FILES
    Editor/EditorCamera.cpp
    Editor/EditorCamera.h)

set(GUI_FILES
    ImGui/GuiLayer.cpp
    ImGui/GuiLayer.h
    ImGui/imgui_impl_immortal.cpp
    ImGui/imgui_impl_immortal.h
    ImGui/Utils.h)

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

set(RENDER_FILES
    Camera.cpp
    Camera.h
    DataSet.h
    Graphics.cpp
    Graphics.h
    Mesh.cpp
    Mesh.h
    OrthographicCamera.cpp
    OrthographicCamera.h
    Render2D.cpp
    Render2D.h)
list(TRANSFORM RENDER_FILES PREPEND "Render/")

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

source_group("\\"                  FILES ${CORE_FILES}      )
source_group("Algorithm\\"         FILES ${ALGORITHM_FILES} )
source_group("Audio\\"             FILES ${AUDIO_FILES}     )
source_group("Framework\\"         FILES ${FRAMEWORK_FILES} )
source_group("Editor\\"            FILES ${EDITOR_FILES}    )
source_group("Image\\"             FILES ${IMAGE_FILES}     )
source_group("ImGui\\"             FILES ${GUI_FILES}       )
source_group("Math\\"              FILES ${MATH_FILES}      )
source_group("Memory\\"            FILES ${MEMORY_FILES}    )
source_group("FileSystem\\"        FILES ${FS_FILES}        )
source_group("Physics\\"           FILES ${PHYSICS_FILES}   )
source_group("Render\\"            FILES ${RENDER_FILES}    )
source_group("Scene\\"             FILES ${SCENE_FILES}     )
source_group("Script\\"            FILES ${SCRIPT_FILES}    )
source_group("Serializer\\"        FILES ${SERIALIZER_FILES})
source_group("String\\"            FILES ${STRING_FILES}    )
source_group("Sync\\"              FILES ${SYNC_FILES}      )
source_group("Helper\\"            FILES ${HELPER_FILES}    )
source_group("Widget\\"            FILES ${WIDGET_FILES}    )
source_group("Net\\"               FILES ${NET_FILES}       )

set(PROJECT_FILES
    ${CORE_FILES}
    ${ALGORITHM_FILES}
    ${AUDIO_FILES}
    ${FRAMEWORK_FILES}
    ${EDITOR_FILES}
    ${FS_FILES}
    ${IMAGE_FILES}
    ${GUI_FILES}
    ${INTERFACE_FILES}
    ${MATH_FILES}
    ${MEMORY_FILES}
    ${IO_FILES}
    ${PHYSICS_FILES}
    ${RENDER_FILES}
    ${SCENE_FILES}
    ${SCRIPT_FILES}
    ${SERIALIZER_FILES}
    ${STRING_FILES}
    ${SYNC_FILES}
    ${HELPER_FILES}
    ${WIDGET_FILES}
    ${NET_FILES}
)

source_group("\\" FILES ${PROJECT_FILES})

list(TRANSFORM PROJECT_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

add_subdirectory(Shared)
add_subdirectory(Graphics)
add_subdirectory(Vision)

add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})

target_sources(${PROJECT_NAME} PUBLIC ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(slapi)

target_link_libraries(${PROJECT_NAME} PUBLIC
    slapi
    external
    imgui
    ImmortalGraphics
    ImmortalVision)
