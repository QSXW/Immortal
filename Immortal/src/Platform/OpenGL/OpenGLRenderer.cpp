#include "impch.h"
#include "OpenGLRenderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal {

	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         LOG::ERR(message); return;
		case GL_DEBUG_SEVERITY_MEDIUM:       LOG::ERR(message); return;
		case GL_DEBUG_SEVERITY_LOW:          LOG::WARN(message); return;
		case GL_DEBUG_SEVERITY_NOTIFICATION: LOG::INFO(message); return;
		}

		SLASSERT(false && "Unknown severity level!");
	}

	void OpenGLRenderer::Init()
	{
#ifdef IM_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glFrontFace(GL_CCW);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glfwWindowHint(GLFW_SAMPLES, 4);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_STENCIL_TEST);

	}

	void OpenGLRenderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRenderer::SetClearColor(const Vector::Vector4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRenderer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void OpenGLRenderer::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->GetVertexBuffers()[0]->Map();
		vertexArray->Map();
		vertexArray->GetIndexBuffer()->Map();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->Count();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

}
