#pragma once

#include <cstdio>

#include <glad/glad.h>

#include <imgui.h>
#include "imgui_internal.h"

#include "ImmortalCore.h"

#include "Framework/Application.h"
#include "Framework/Async.h"
#include "Framework/Layer.h"
#include "Framework/Log.h"
#include "Framework/Input.h"

#include "ImGui/GuiLayer.h"
#include "ImGui/ImGuizmo.h"
#include "ImGui/Utils.h"

#include "Utils/PlatformUtils.h"
#include "Utils/json.h"

#include "Render/DataSet.h"
#include "Render/Shader.h"
#include "Render/Texture.h"
#include "Render/RenderTarget.h"
#include "Render/Render.h"
#include "Render/OrthographicCamera.h"
#include "Render/OrthographicCameraController.h"
#include "Render/Renderer.h"
#include "Render/Render2D.h"
#include "Render/Frame.h"
#include "Render/Mesh.h"
#include "Render/Render.h"

#include "Platform/Vulkan/Common.h"
#include "Platform/Vulkan/RenderContext.h"
#include "Platform/Vulkan/GuiLayer.h"

#include "Sync/Semaphore.h"
#include "Sync/SemaphorePool.h"

#include "Editor/EditorCamera.h"

#include "Scene/Object.h"
#include "Scene/Scene.h"
#include "Scene/Component.h"
#include "Scene/GameObject.h"
#include "Scene/ObserverCamera.h"

#include "Serializer/SceneSerializer.h"

#include "String/LanguageSettings.h"

#include "Widget/Widget.h"
#include "Widget/Viewport.h"
#include "Widget/MenuBar.h"

#include "Net/Socket.h"
