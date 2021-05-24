workspace "Immortal"
	architecture "x64"
	startproject "ImmortalEditor"

	configurations
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Dependencies.lua"

project "Immortal"
	location "Immortal"
	kind "Staticlib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "impch.h"
	pchsource "Immortal/src/impch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/3rdparty/glm/glm/**.h",
		"%{prj.name}/3rdparty/glm/glm/**.inl"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.opencv}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.Vulkan}"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	links
	{
		"GLFW",
		"Glad",
		"opengl32.lib",
		"ImGui",
		"yaml-cpp",
		"%{Library.mono}",
		"%{Library.Vulkan}",
		"%{Library.VulkanUtils}"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"IMMORTAL_PLATFORM_WINDOWS",
			"IMMORTAL_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "IM_DEBUG"
		buildoptions "/MDd"
		symbols "on"
		links
		{
			"%{Library.OpencvDebug}",
			"%{Library.AssimpDebug}",
			"%{Library.ShadercDebug}",
			"%{Library.SPIRVCrossDebug}",
			"%{Library.SPIRVCrossGLSLDebug}",
			"%{Library.SPIRVToolsDebug}"
		}

	filter "configurations:Release"
		defines "IM_RELEASE"
		buildoptions "/MD"
		optimize "on"
		links
		{
			"%{Library.OpencvRelease}",
			"%{Library.AssimpRelease}",
			"%{Library.ShadercRelease}",
			"%{Library.SPIRVCrossRelease}",
			"%{Library.SPIRVCrossGLSLRelease}"
		}

project "ImmortalEditor"
	location "ImmortalEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Immortal",
		"Immortal/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.opencv}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}"
	}

	links
	{
		"Immortal"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"IMMORTAL_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "IM_DEBUG"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		buildoptions "/MD"
		defines "IM_RELEASE"
		optimize "on"

project "RuntimeTest"
	location "RuntimeTest"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Immortal",
		"Immortal/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.opencv}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}"
	}

	links
	{
		"Immortal"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"IMMORTAL_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "IM_DEBUG"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		buildoptions "/MD"
		defines "IM_RELEASE"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Immortal",
		"%{IncludeDir.spdlog}",
		"Immortal/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}"
	}

	links
	{
		"Immortal"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"IMMORTAL_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "IM_DEBUG"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		buildoptions "/MD"
		defines "IM_RELEASE"
		optimize "on"

project "VulkanSanbox"
	location "VulkanSanbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Immortal",
		"Immortal/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.opencv}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}"
	}

	links
	{
		"Immortal"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"IMMORTAL_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "IM_DEBUG"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		buildoptions "/MD"
		defines "IM_RELEASE"
		optimize "on"