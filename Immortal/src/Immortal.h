#pragma once

#include <cstdio>

#include <glad/glad.h>

#include <imgui.h>
#include "imgui_internal.h"

#include "ImmortalCore.h"
#include "Framework/Application.h"
#include "Framework/Layer.h"
#include "Framework/Log.h"
#include "ImGui/GuiLayer.h"
#include "ImGui/ImGuizmo.h"
#include "ImGui/ui.h"

#include "Framework/Input.h"

#include "Utils/PlatformUtils.h"

#include "Image/Colorspace.h"

#include "Render/Shader.h"
#include "Render/Texture.h"
#include "Render/VertexArray.h"
#include "Render/Framebuffer.h"
#include "Render/Render.h"
#include "Render/OrthographicCamera.h"
#include "Render/OrthographicCameraController.h"
#include "Render/Renderer.h"
#include "Render/Renderer2D.h"
#include "Render/Frame.h"
#include "Render/Mesh.h"

#include "Sync/Semaphore.h"
#include "Sync/SemaphorePool.h"

#include "Editor/EditorCamera.h"

#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Scene/ScriptableObject.h"
#include "Scene/ObserverCamera.h"

#include "Serializer/SceneSerializer.h"

#include "String/ChineseString.h"
