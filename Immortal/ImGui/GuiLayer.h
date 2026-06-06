#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include "Core.h"
#include "Framework/Layer.h"
#include "Shared/IObject.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Graphics/LightGraphics.h"

#define DEFINE_CPP_STRING_API(FN_NAME, ...) \
    template <class...  Args> \
    static inline bool FN_NAME(const std::string &label, Args &&...args) \
    { \
        return ImGui::FN_NAME(label.c_str(), std::forward<Args>(args)...); \
    }

namespace ImGui
{

template <class T>
inline ImVec4 ConvertColor(T r, T g, T b, T a)
{
    ImVec4 ret;

    constexpr decltype(ret.x) max = (T)~0;

    ret.x = r / max;
    ret.y = g / max;
    ret.z = b / max;
    ret.w = a / max;

    return ret;
}

static inline ImVec4 RGBA32(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return ConvertColor<decltype(r)>(r, g, b, a);
}

static inline ImVec4 RGBA32(uint32_t rgba)
{
	uint8_t *_rgba = (uint8_t *)&rgba;
	return ConvertColor<uint8_t>(_rgba[3], _rgba[2], _rgba[1], _rgba[0]);
}

static inline void Text(const Immortal::String &text)
{
    ImGui::Text(text.c_str());
}

static inline bool MenuItem(const std::string &label, const char *shortcut = NULL, bool selected = false, bool enabled = true)
{
    return MenuItem(label.c_str(), shortcut, selected, enabled);
}

static inline bool BeginMenu(const std::string &label, bool enabled = true)
{
    return BeginMenu(label.c_str(), enabled);
}

static inline bool Begin(const std::string &name, bool *p_open = NULL, ImGuiWindowFlags flags = 0)
{
    return Begin(name.c_str(), p_open, flags);
}

DEFINE_CPP_STRING_API(CollapsingHeader)

}

namespace UI
{

DEFINE_CPP_STRING_API(Button)

}

struct FontContext
{
    ImFont *Regular{ nullptr };
    ImFont *Medium{ nullptr };
    ImFont *Light{ nullptr };
    ImFont *Demilight{ nullptr };
    ImFont *Bold{ nullptr };
    ImFont *Black{ nullptr };
};

namespace Immortal
{

class Window;
class Device;
enum class Language
{
	Chinese,
    English,
};

template <class T>
struct StyleColorStack
{
public:
    StyleColorStack(std::initializer_list<std::pair<int, T>> &&styles) :
        size{ int(styles.size()) }
    {
        for (auto &[style, value] : styles)
        {
            ImGui::PushStyleColor(style, value);
        }
    }

    ~StyleColorStack()
    {
        ImGui::PopStyleColor(size);
    }

    int size;
};

template <class T>
struct StyleVarStack
{
public:
    StyleVarStack(std::initializer_list<std::pair<int, T>> &&styles) :
        size{ int(styles.size()) }
    {
        for (auto &[style, value] : styles)
        {
            ImGui::PushStyleVar(style, value);
        }
    }

    ~StyleVarStack()
    {
        ImGui::PopStyleVar(size);
    }

    int size;
};

struct FontStack
{
public:
    FontStack(ImFont *font, float scale = 1.0f) :
        scale{ scale },
	    lastScale{ ImGui::GetCurrentWindow()->FontWindowScale }
    {
        ImGui::PushFont(font);
		if (scale != lastScale)
        {
            ImGui::SetWindowFontScale(scale);
        }
    }

    ~FontStack()
    {
        if (scale != lastScale)
        {
			ImGui::SetWindowFontScale(lastScale);
        }
        ImGui::PopFont();
    }

    float scale;
	float lastScale;
};

class WWindow;
class WDockerSpace;
class RenderContext;
class IMMORTAL_API GuiLayer : public Layer
{
public:
    static constexpr float MinWindowSizeX = 320.0f;
    static constexpr float MinWindowSizeY = 36.0f;

    static FontContext NotoSans;

    static FontContext SimSun;

public:
    GuiLayer(Device *device, Queue *queue, Window *window, Swapchain *swapchain);

    virtual ~GuiLayer();

    virtual void OnAttach() override;

    virtual void OnEvent(Event &e) override;

    virtual void OnDetach() override;

    virtual void Begin();

    virtual void End();

    void SubmitRenderDrawCommands(CommandBuffer *commandBuffer);

    void Render();

	void AddChild(Widget *widget);

    void SetTheme();

    bool LoadTheme();

    bool SaveTheme();

    void BlockEvent(bool block)
	{
		blockEvents = block;
	}

    ImFont *DemiLight()
    {
        return NotoSans.Demilight;
    }

    ImFont *Bold()
    {
        return NotoSans.Bold;
    }

    void UpdateTheme();

    static void Inject2Dockspace(Widget *widget)
	{
		SLASSERT(This && "ImGui is not initialized yet!");
		This->AddChild(widget);
	}

    static bool IsLanguage(Language lang)
    {
		return This->language == lang;
    }

protected:
	void __Begin()
	{
		ImGui::NewFrame();
	}

	void __End()
	{
		ImGui::Render();
	}

protected:
	Device *device;

    Queue *queue;

    Swapchain *swapchain;

	Window *window;

    std::function<void()> platformSpecificWindow;

    struct
    {
		std::function<void()> NewFrame;
		std::function<void()> ShutDown;
    } platformSpecficWindow;

    Ref<WDockerSpace> dockspace;

    bool blockEvents = true;

    float time = 0.0f;

    Language language = Language::Chinese;

    static GuiLayer *This;

    URef<WWindow> themeEditor;

    ImVector<ImWchar> fontRanges;
};

using SuperGuiLayer = GuiLayer;

namespace Interface
{
    using GuiLayer = SuperGuiLayer;
}

}
