#pragma once

#include "Graphics/Texture.h"
#include "ImGui/GuiLayer.h"

namespace Immortal
{

struct WImageResource
{
	WImageResource(Image *image = nullptr, const ImVec2 &uv0 = {}, const ImVec2 &uv1 = { 1, 1 }) :
		image{ image },
		uv{ uv0, uv1 }
	{

	}

	void Resource(Image *_image, const ImVec2 &uv0, const ImVec2 &uv1)
	{
		image = _image;
		uv._0 = uv0,
		uv._1 = uv1;
	}

	void Flip()
	{
		std::swap(uv._0, uv._1);
	}

	Image *image;
	struct
	{
		ImVec2 _0 = { 0, 0 };
		ImVec2 _1 = { 1, 1 };
	} uv;
};

}
