#pragma once

#include "Graphics/Texture.h"
#include "ImGui/GuiLayer.h"

namespace Immortal
{

struct WImageResource
{
	WImageResource(Ref<Texture> image = nullptr, const ImVec2 &uv0 = {}, const ImVec2 &uv1 = {1, 1}) :
		image{ image },
		uv{ uv0, uv1 }
	{

	}

	~WImageResource()
	{
		Graphics::ReleaseResource(image);
	}

	void Resource(Ref<Texture> _image, const ImVec2 &uv0, const ImVec2 &uv1)
	{
		Graphics::ReleaseResource(image);
		image = _image;
		uv._0 = uv0,
		uv._1 = uv1;
	}

	void Flip()
	{
		std::swap(uv._0, uv._1);
	}

	Ref<Texture> image;
	struct
	{
		ImVec2 _0 = { 0, 0 };
		ImVec2 _1 = { 1, 1 };
	} uv;
};

}
