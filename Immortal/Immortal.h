#pragma once

#include <cstdio>

#include <imgui.h>
#include <imgui_internal.h>

#include "Audio/Device.h"

#include "Core.h"
#include "Config.h"

#include "FileSystem/FileSystem.h"

#include "Framework/Application.h"

#include "Shared/Async.h"
#include "Shared/Log.h"
#include "Graphics/LightGraphics.h"

#include "Helper/Arguments.h"

#include "ImGui/GuiLayer.h"
#include "ImGui/Utils.h"

#include "Shared/IObject.h"

#include "Math/Math.h"
#include "Math/Vector.h"

#include "Memory/Memory.h"

#include "Vision/Image.h"
#include "Vision/Video/Video.h"

#include "Helper/Platform.h"
#include "Helper/json.h"

#include "Render/Graphics.h"
#include "Render/OrthographicCamera.h"
#include "Render/Render2D.h"
#include "Render/Mesh.h"

#include "Sync/Semaphore.h"

#include "Editor/EditorCamera.h"

#include "Script/ScriptEngine.h"

#include "Scene/Object.h"
#include "Scene/Scene.h"
#include "Scene/Component.h"
#include "Scene/GameObject.h"
#include "Scene/ObserverCamera.h"

#include "Serializer/SceneSerializer.h"

#include "String/LanguageSettings.h"

#include "Widget/Widget.h"
#include "Widget/WFileDialog.h"
#include "Widget/WImageButton.h"
#include "Widget/WSlider.h"
#include "Widget/MenuBar.h"

#include "Net/Socket.h"
#include "Net/TCP.h"
#include "Net/LTP.h"

#include "Physics/Physics.h"
