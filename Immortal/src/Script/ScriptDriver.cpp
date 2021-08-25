#include "impch.h"
#include "ScriptDriver.h"
#pragma once

// Docs: https://www.mono-project.com/docs/advanced/embedding/
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

namespace Immortal
{
	HMODULE NativeScriptDriver::_M_moudle = nullptr;

	MonoDomain *ScriptDriver::Domain = nullptr;
	std::string Compiler::Log;
	int Compiler::Status = -1;

	std::string Linker::Log;
	int Linker::Status = -1;

	std::string Compiler::CompileOptions = {
			"cl.exe "
			"/JMC "
			"/ifcOutput \"ScriptsCore/bin-int/\""
			"/GS "
			"/W3 "
			"/Zc:wchar_t "
			"/ZI "
			"/Gm- "
			"/Od "
			"/Fd\"ScriptsCore/bin-int/\" "
			"/Zc:inline "
			"/fp:precise "
			"/D IMMORTAL_PLATFORM_WINDOWS "
			"/D IM_DEBUG "
			"/D _WINDLL "
			"/D _UNICODE "
			"/D UNICODE "
			"/errorReport:prompt "
			"/WX- "
			"/Zc:forScope "
			"/RTC1 "
			"/Gd "
			"/MDd "
			"/std:c++17 "
			"/FC "
			"/Fa\"ScriptsCore/bin-int/\" "
			"/EHsc "
			"/nologo "
			"/Fo\"ScriptsCore/bin-int/\" "
			"/Fp\"ScriptsCore/bin-int/\" "
			"/diagnostics:column "
			"/I\"std/include/\" "
			"/I\"std/crt/src/\" "
			"/I\"std/WindowsSDK/10.0.19041.0/shared/\" "
			"/I\"std/WindowsSDK/10.0.19041.0/ucrt/\" "
			"/I\"std/WindowsSDK/10.0.19041.0/um/\" "
			"/I\"std/WindowsSDK/10.0.19041.0/winrt/\" "
			"/I\"D:/Code/C/Immortal/Immortal/3rdparty/glm\" "
			"/I\"D:/Code/C/Immortal/Immortal/3rdparty/spdlog/include\" "
			"/I\"D:/Code/C/Immortal/Immortal/3rdparty/assimp/include\" "
			"/I\"D:/Code/C/Immortal/Immortal/src\" "
	};

	std::string Compiler::LinkOptions = {
		"/link "
		"/OUT:\"ScriptsCore/bin/Runtime.dll\" "
		"/MANIFEST "
		"/NXCOMPAT "
		//"/PDB:\"ScriptsCore/bin/Runtime.pdb\" "
		"/DYNAMICBASE "
		"kernel32.lib "
		"user32.lib "
		"gdi32.lib "
		"winspool.lib "
		"comdlg32.lib "
		"advapi32.lib "
		"shell32.lib "
		"ole32.lib "
		"oleaut32.lib "
		"uuid.lib "
		"odbc32.lib "
		"odbccp32.lib "
		"/DLL "
		"lib/GLFW.lib "
		"lib/Glad.lib "
		"lib/ImGui.lib "
		"lib/yaml-cpp.LIB "
		"lib/Immortal.lib "
		"/IMPLIB:\"ScriptsCore/bin/Runtime.lib\" "
		"/DEBUG:NONE "
		"/DLL "
		"/MACHINE:X64 "
		"/INCREMENTAL "
		//"/PGD:\"../bin/Debug-windows-x86_64/RuntimeTest.pgd\" "
		"/SUBSYSTEM:WINDOWS "
		"/MANIFESTUAC:\"level='asInvoker'uiAccess='false'\" "
		"/ManifestFile:\"ScriptsCore/bin-int/RuntimeTest.dll.intermediate.manifest\" "
		"/LTCGOUT:\"ScriptsCore/bin-int/Runtime.iobj\" "
		"/ERRORREPORT:PROMPT "
		"/NOLOGO "
		"/TLBID:1 "
	};

	void ScriptDriver::On(const std::string & assemblyPath)
	{
		InitMono();
	}

	void ScriptDriver::Off()
	{

	}

	void ScriptDriver::InitMono()
	{
		if (!Domain)
		{
			mono_set_assemblies_path("mono/lib");
			Domain = mono_jit_init("Immortal");
			Domain = mono_domain_create_appdomain("ImmortalApp", nullptr);
		}
	}

	void NativeScriptDriver::Off()
	{

	}

	static std::string GbkToUtf8(const char *src_str)
	{
		int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[(size_t)len + 1];
		memset(wstr, 0, (size_t)len + 1);
		MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		char* str = new char[(size_t)len + 1];
		memset(str, 0, (size_t)len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
		std::string tempStr = str;
		if (wstr) delete[] wstr;
		if (str) delete[] str;
		return tempStr;
	}

	[[nodiscard]]
	Compiler::Flag NativeScriptDriver::Compile(const std::string &workspace, const std::vector<std::string> &scripts)
	{
		constexpr int bufferSize = 512;
		char   psBuffer[bufferSize];
		FILE   *pPipe;
		Compiler::Flag ret = Compiler::Flag::Failed;

		std::string command = Compiler::CompileOptions + std::string("/I\"") + workspace + "\" ";
		for (auto &s : scripts)
		{
			command += s + " ";
		}
		command.append(Compiler::LinkOptions);

		Compiler::Log.append(UNICODE8("ø™ º±‡“Î: \n"));
		if ((pPipe = _popen(command.c_str(), "rt")) == NULL)
		{
			Compiler::Log.append("There is some problem in the pipeline between Immortal and MSCV.");
			return Compiler::Flag::Failed;
		}

		while (fgets(psBuffer, bufferSize, pPipe))
		{
			Compiler::Log.append(GbkToUtf8(psBuffer));
		}

		if (feof(pPipe))
		{
			Compiler::Status = _pclose(pPipe);
			Compiler::Log.append(UNICODE8("cl.exe ∑µªÿ◊¥Ã¨: "));
			Compiler::Log.append(std::to_string(Compiler::Status));
			Compiler::Log.append("\n");
		}
		else
		{
			LOG::FATAL("Error: Failed to read the pipe to the end.\n");
		}

		switch(Compiler::Status)
		{
		case 0:
			ret = Compiler::Flag::Succeed;
			LOG::INFO(Compiler::Log);
			Compiler::Log.append(UNICODE8("±‡“Î≥…π¶\n"));
			break;
		case 1:
			ret = Compiler::Flag::NotFound;
		case 2:
			ret = Compiler::Flag::InvalidParameter;
		default:
			Compiler::Log.append(UNICODE8("±‡“Î ß∞‹\n"));
			break;
		}
		return ret;
	}

}