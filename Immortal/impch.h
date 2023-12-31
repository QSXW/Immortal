#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

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

#ifdef __linux__
#include "Audio/ALSA.h"
#endif
#include "Audio/AudioRenderContext.h"
#include "Audio/AudioSource.h"
#include "Audio/Device.h"
#ifdef _WIN32
#include "Audio/WASAPI.h"
#endif

#include "FileSystem/FileSystem.h"
#include "FileSystem/RF.h"
#include "FileSystem/Stream.h"

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

#include "Interface/Delegate.h"
#include "Interface/IObject.h"

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
#include "Scene/GameObject.h"
#include "Scene/Object.h"
#include "Scene/ObserverCamera.h"
#include "Scene/Scene.h"
#include "Scene/SceneCamera.h"
#include "Script/ScriptEngine.h"
#include "Serializer/SceneSerializer.h"
#include "String/LanguageSettings.h"
#include"Sync/Semaphore.h"

#include "Vision/Audio/WAV.h"
#include "Vision/Common/Animator.h"
#include "Vision/Common/Animator.h"
#include "Vision/Common/BitTracker.h"
#include "Vision/Common/Checksum.h"
#include "Vision/Common/Error.h"
#include "Vision/Common/NetworkAbstractionLayer.h"
#include "Vision/Common/SamplingFactor.h"

#include "Vision/Demux/FFDemuxer.h"
#include "Vision/Demux/IVFDemuxer.h"

#include "Vision/External/stb_image.h"

#include "Vision/Image/BMP.h"
#include "Vision/Image/BMP.h"
#include "Vision/Image/Helper.h"
#include "Vision/Image/Image.h"
#include "Vision/Image/JPEG.h"
#include "Vision/Image/MFXJpegCodec.h"
#include "Vision/Image/OpenCVCodec.h"
#include "Vision/Image/PPM.h"
#include "Vision/Image/Raw.h"
#include "Vision/Image/STBCodec.h"

#include "Vision/Interface/Codec.h"
#include "Vision/Interface/Demuxer.h"
#include "Vision/Interface/MFXCodec.h"

#include "Vision/Processing/ColorSpace.h"

#include "Vision/Video/DAV1DCodec.h"
#include "Vision/Video/FFCodec.h"
#include "Vision/Video/HEVC.h"
#include "Vision/Video/Video.h"

#ifdef _WIN32
#include "Vision/Video/D3D12/HEVCCodec.h"
#endif

#include "Vision/Video/Vulkan/HEVC.h"

#include "Widget/MenuBar.h"
#include "Widget/Resource.h"
#include "Widget/WFileDialog.h"
#include "Widget/Widget.h"
#include "Widget/WImageButton.h"
