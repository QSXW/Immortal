workspace "Immortal"
	architecture "x64"
	startproject "ImmortalEditor"

	configurations
	{
		"Debug",
		"Release",
		"Distributable"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Immortal/3rdparty/GLFW/include"
IncludeDir["Glad"] = "Immortal/3rdparty/Glad/include"
IncludeDir["ImGui"] = "Immortal/3rdparty/imgui"
IncludeDir["glm"] = "Immortal/3rdparty/glm"
IncludeDir["opencv"] = "Immortal/3rdparty/opencv"
IncludeDir["yaml"] = "Immortal/3rdparty/yaml-cpp/include"
IncludeDir["assimp"] = "Immortal/3rdparty/assimp/include"
IncludeDir["mono"] = "Immortal/3rdparty/mono/include"

group "Dependencies"
	include "Immortal/3rdparty/GLFW"
	include "Immortal/3rdparty/Glad"
	include "Immortal/3rdparty/imgui"
	include "Immortal/3rdparty/yaml-cpp"

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
		"%{prj.name}/3rdparty/spdlog/include",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.opencv}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.mono}"
	}

	links
	{
		"GLFW",
		"Glad",
		"opengl32.lib",
		"ImGui",
		"yaml-cpp",
		"bin/%{outputdir}/opencv_world452d",
		"Immortal/3rdparty/assimp/bin/Debug/assimp-vc141-mtd.lib",
		"Immortal/3rdparty/mono/lib/Debug/mono-2.0-sgen.lib"
	}

	flags
	{
		"MultiProcessorCompile"
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

	filter "configurations:Release"
		defines "IM_RELEASE"
		buildoptions "/MD"
		optimize "on"

	filter "configurations:Distributable"
		defines "IM_DISTRUTABLE"
		buildoptions "/MD"
		optimize "on"

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
		"Immortal/3rdparty/spdlog/include",
		"Immortal/src",
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

	filter "configurations:Distributable"
		buildoptions "/MD"
		defines "IM_DISTRUTABLE"

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
		"Immortal/3rdparty/spdlog/include",
		"Immortal/src",
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

	filter "configurations:Distributable"
		buildoptions "/MD"
		defines "IM_DISTRUTABLE"

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
		"Immortal/3rdparty/spdlog/include",
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

	filter "configurations:Distributable"
		buildoptions "/MD"
		defines "IM_DISTRUTABLE"