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

	void Resource(Image *_image, const ImVec2 &uv0, const ImVec2 &uv1)
	{
		image = _image;
		uv._0 = uv0,
		uv._1 = uv1;
	}
};

}
