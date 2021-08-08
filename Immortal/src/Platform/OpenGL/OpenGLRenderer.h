#pragma once

#include "ImmortalCore.h"
#include "Render/RendererAPI.h"

#include <glad/glad.h>

namespace Immortal {

	class OpenGLRenderer : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const Vector::Vector4& color) override;
		virtual void Clear() override;

		virtual void EnableDepthTest() override
		{
			glEnable(GL_DEPTH_TEST);
		}

		virtual void DisableDepthTest() override
		{
			glDisable(GL_DEPTH_TEST);
		}

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
	};

}