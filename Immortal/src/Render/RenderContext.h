#pragma once

#include "ImmortalCore.h"

namespace Immortal {

	class IMMORTAL_API RenderContext
	{
	public:
		struct Description
		{
			void *WindowHandle;
			const char *ApplicationName;
		};

	public:
		static Unique<RenderContext> Create(Description &desc);

	public:
		RenderContext() { }

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;

		virtual const char *GraphicsRenderer() const { return graphicsRenderer.c_str(); }
		virtual const char *DriverVersion() const { return driverVersion.c_str(); }

	public:
		RenderContext(const RenderContext &) = delete;
		RenderContext(RenderContext &&) = delete;
		RenderContext &operator=(const RenderContext &) = delete;
		RenderContext &operator=(RenderContext &&) = delete;
	
	protected:
		std::string graphicsRenderer{};
		std::string driverVersion{};
	};

}
