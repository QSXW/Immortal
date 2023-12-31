#pragma once

#include "Window.h"
#include "Input.h"

struct GLFWwindow;

namespace Immortal
{

class IMMORTAL_API GLFWWindow : public Window
{
public:
	GLFWWindow(Anonymous handle);

	GLFWWindow(const std::string &title, uint32_t width, uint32_t height, Window *parent = nullptr);

    virtual ~GLFWWindow() override;

    virtual uint32_t GetWidth() const override;

    virtual uint32_t GetHeight() const override;

    virtual void SetEventCallback(const EventCallbackFunc &callback) override;

    virtual Anonymous GetBackendHandle() const override;

    virtual Anonymous GetPlatformSpecificHandle() const override;

    virtual void Show() override;

    virtual void ProcessEvents() override;

    virtual void SetTitle(const std::string &title) override;

    virtual void SetIcon(const std::string &filepath) override;

protected:
	void Construct(const std::string &title, uint32_t width, uint32_t height);

    void Shutdown();

    void SelectPlatformType();

protected:
    GLFWwindow *window;

    EventCallbackFunc eventCallback;

    GLFWWindow *parent;

    uint32_t childWindow;

    bool owned;

    std::unique_ptr<Input> input;
};

}
