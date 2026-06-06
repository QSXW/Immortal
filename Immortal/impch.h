#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <limits>

#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <list>

#include "Core.h"
#include "Shared/Log.h"
#include "Algorithm/LightArray.h"
#include "Algorithm/LightVector.h"
#include "Algorithm/Rotate.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/RF.h"
#include "FileSystem/Stream.h"

#include "Filter/DisplayOrientationFilter.h"
#include "Filter/Filter.h"
#include "Filter/GaussianBlurFilter.h"
#include "Filter/LogCurveFilter.h"
#include "Filter/Lut3DFilter.h"
#include "Filter/ScaleFilter.h"
#include "Filter/Transfer.h"

#include "Helper/Arguments.h"
#include "Helper/json.h"
#include "Helper/nlohmann_json.h"
#include "Helper/Platform.h"

#include "Framework/Application.h"
#include "Framework/Layer.h"
#include "Framework/LayerStack.h"
#include "Framework/Timer.h"
#include "Framework/Utils.h"

#include "ImGui/GuiLayer.h"
#include "ImGui/Utils.h"

#include "Shared/IObject.h"

#include "Math/Math.h"
#include "Math/Vector.h"

#include "Memory/Allocator.h"
#include "Memory/Memory.h"
#include "Memory/MemoryAllocator.h"

#include "Net/LTP.h"
#include "Net/Socket.h"
#include "Net/TCP.h"

#include "Scene/Component.h"
#include "Scene/entt.hpp"
#include "Scene/FilterGraphComponent.h"
#include "Scene/GameObject.h"
#include "Scene/Object.h"
#include "Scene/ObserverCamera.h"
#include "Scene/Scene.h"
#include "Scene/SceneCamera.h"
#include "Scene/VideoPlayerComponent.h"

#include "Script/ScriptEngine.h"
#include "Serializer/SceneSerializer.h"
#include "String/LanguageSettings.h"
#include "Sync/Semaphore.h"

#include "Widget/MenuBar.h"
#include "Widget/Resource.h"
#include "Widget/WFileDialog.h"
#include "Widget/Widget.h"
#include "Widget/WImageButton.h"

#include "Vision/Image.h"
