
group "Dependencies"
    include "3rdparty/GLFW"
    include "3rdparty/Glad"
    include "3rdparty/imgui"

VulkanSdk = os.getenv("VULKAN_SDK")

ThirdPartyDir = "%{wks.location}/3rdparty";

IncludeDir = {}
IncludeDir["spdlog"] = "%{ThirdPartyDir}/spdlog/include"
IncludeDir["GLFW"]   = "%{ThirdPartyDir}/GLFW/include"
IncludeDir["Glad"]   = "%{ThirdPartyDir}/Glad/include"
IncludeDir["ImGui"]  = "%{ThirdPartyDir}/imgui"
IncludeDir["glm"]    = "%{ThirdPartyDir}/glm"
IncludeDir["opencv"] = "%{ThirdPartyDir}/opencv"
IncludeDir["assimp"] = "%{ThirdPartyDir}/assimp/include"
IncludeDir["mono"]   = "%{ThirdPartyDir}/mono/include"
IncludeDir["Vulkan"] = "%{ThirdPartyDir}/Vulkan/include"

LibraryDir = {}
LibraryDir["VulkanSDK"]      = "%{ThirdPartyDir}/Vulkan/Lib/Release"
LibraryDir["VulkanSDKDebug"] = "%{ThirdPartyDir}/Vulkan/Lib/Debug"
LibraryDir["Opencv"]         = "%{ThirdPartyDir}/opencv/Lib";

Library = {}

Library["OpencvDebug"]   = "%{LibraryDir.Opencv}/Debug/opencv_world452d.lib"
Library["OpencvRelease"] = "%{LibraryDir.Opencv}/Release/opencv_world452.lib"

Library["Vulkan"]      = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["mono"] = "%{ThirdPartyDir}/mono/lib/Debug/mono-2.0-sgen.lib"

Library["AssimpDebug"]   = "%{ThirdPartyDir}/assimp/bin/Debug/assimp-vc141-mtd.lib"
Library["AssimpRelease"] = "%{ThirdPartyDir}/assimp/bin/Release/assimp-vc141-mtd.lib"

Library["ShadercDebug"]        = "%{LibraryDir.VulkanSDKDebug}/shaderc_sharedd.lib"
Library["SPIRVToolsDebug"]     = "%{LibraryDir.VulkanSDKDebug}/SPIRV-Toolsd.lib"
Library["SPIRVCrossDebug"]     = "%{LibraryDir.VulkanSDKDebug}/spirv-cross-cored.lib"
Library["SPIRVCrossGLSLDebug"] = "%{LibraryDir.VulkanSDKDebug}/spirv-cross-glsld.lib"

Library["ShadercRelease"]        = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRVCrossRelease"]     = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRVCrossGLSLRelease"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
