#pragma once

#include <string>
#include <optional>

#include "ImmortalCore.h"

namespace Immortal {

	class FileDialogs
	{
	public:
		enum class Type : UINT32
		{
			None = 0X2E000000U,
			FBX  = 0X2E666278U,
			JPEG = 0X2E6A7067U,
			PNG  = 0X2E706E67U
		};

	public:
		static std::optional<std::string> OpenFile(const char *filter);
		static std::optional<std::string> SaveFile(const char *filter);
		static char ImageFilter[];
				
	};
}