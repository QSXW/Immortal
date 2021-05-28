#pragma once

#include <cstdio>


#include <imgui.h>
#include "imgui_internal.h"

#include <glad/glad.h>

#include "ImmortalCore.h"
#include "Immortal/Core/Application.h"
#include "Immortal/Core/Layer.h"
#include "Immortal/Core/Log.h"
#include "Immortal/ImGui/GuiLayer.h"
#include "Immortal/ImGui/ImGuizmo.h"
#include "Immortal/Core/Input.h"

#include "Immortal/Utils/PlatformUtils.h"

#include "Immortal/Image/Colorspace.h"

#include "Immortal/Render/Shader.h"
#include "Immortal/Render/Texture.h"
#include "Immortal/Render/VertexArray.h"
#include "Immortal/Render/Framebuffer.h"
#include "Immortal/Render/RenderCommand.h"
#include "Immortal/Render/OrthographicCamera.h"
#include "Immortal/Render/OrthographicCameraController.h"
#include "Immortal/Render/Renderer.h"
#include "Immortal/Render/Renderer2D.h"
#include "Immortal/Render/Frame.h"
#include "Immortal/Render/Mesh.h"

#include "Immortal/Editor/EditorCamera.h"

#include "Immortal/Scene/Entity.h"
#include "Immortal/Scene/Scene.h"
#include "Immortal/Scene/ScriptableObject.h"
#include "Immortal/Scene/ObserverCamera.h"

#include "Immortal/Serializer/SceneSerializer.h"

#include "Immortal/String/ChineseString.h"
