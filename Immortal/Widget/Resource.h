#pragma once

#include "Render/Texture.h"
#include "ImGui/GuiLayer.h"

namespace Immortal
{

struct WImageResource
{
	Image *image;
	struct
	{
		ImVec2 _0 = { 0, 0 };
		ImVec2 _1 = { 1, 1 };
	} uv;
};

}
