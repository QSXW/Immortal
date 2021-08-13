#include "impch.h"

#include "GLFWWindow.h"
#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "Event/KeyEvent.h"

#include "Render/RendererAPI.h"

namespace Immortal
{
	UINT8 GLFWWindow::GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char *description)
	{
		IM_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	GLFWWindow::GLFWWindow(const WindowProps& props)
	{
		Init(props);
	}

	GLFWWindow::~GLFWWindow()
	{
		Shutdown();
	}

	void GLFWWindow::ProcessEvents()
	{
		glfwPollEvents();
	}

	/// @brief It calculates the dpi factor using the density from GLFW physical size
	/// <a href="https://www.glfw.org/docs/latest/monitor_guide.html#monitor_size">GLFW docs for dpi</a>
	float GLFWWindow::DpiFactor() const
	{
		auto primaryMonitor = glfwGetPrimaryMonitor();
		auto videoMode = glfwGetVideoMode(primaryMonitor);

		INT32 mmWidth;
		INT32 mmHeight;
		glfwGetMonitorPhysicalSize(primaryMonitor, &mmWidth, &mmHeight);

		static constexpr const float inch2mm = 25.0f;
		static constexpr const float windowBaseDesity = 96.0f;

		auto dpi = static_cast<UINT32>(videoMode->width / (mmWidth / inch2mm));
		return dpi / windowBaseDesity;
	}

	void GLFWWindow::SetTitle(const std::string &title)
	{
		mData.Title = title;
		glfwSetWindowTitle(mWindow, mData.Title.c_str());
	}

	void GLFWWindow::Init(const WindowProps& props)
	{
		mData.Title  = props.Title;
		mData.Width  = props.Width;
		mData.Height = props.Height;

		IM_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (GLFWWindowCount == 0)
		{
			glfwSetErrorCallback(GLFWErrorCallback);

			int success = glfwInit();
			IM_CORE_ASSERT(success, "Could not initialize GLFW!")
		}

		if (RendererAPI::API == RendererAPI::Type::VulKan)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}

		mWindow = glfwCreateWindow((int)props.Width, (int)props.Height, mData.Title.c_str(), nullptr, nullptr);
		++GLFWWindowCount;

		// To do list
		RenderContext::Description desc = {
			mWindow
		};
		mContext = RenderContext::Create(desc);
		mContext->Init();

		glfwSetWindowUserPointer(mWindow, &mData);
		SetVSync(true);

		/* Set callbacks */
		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow *window, int width, int height)
		{
			WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* window) {
			WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int modes)
		{
			WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			
			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow *window, int button, int action, int modes)
		{
			WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event((MouseCode)button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event((MouseCode)button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(mWindow, [](GLFWwindow *window, double xOffset, double yOffset)
		{
			WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(mWindow, [](GLFWwindow *window, double xPos, double yPos)
		{
			WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseMoveEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void GLFWWindow::Shutdown()
	{
		glfwDestroyWindow(mWindow);
		--GLFWWindowCount;

		if (GLFWWindowCount == 0)
		{
			glfwTerminate();
		}
	}

	void GLFWWindow::OnUpdate()
	{
		mContext->SwapBuffers();
		glfwPollEvents();
	}

	void GLFWWindow::SetVSync(bool enabled)
	{
		if (enabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}

		mData.Vsync = enabled;
	}

	bool GLFWWindow::IsVSync() const
	{
		return mData.Vsync;
	}

	void GLFWWindow::Clear()
	{
		glClearColor(0.1098f, 0.1098f, 0.1098f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	float GLFWWindow::Time()
	{
		return static_cast<float>(glfwGetTime());
	}

}