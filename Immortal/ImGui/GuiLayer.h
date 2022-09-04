#pragma once

#include <imgui.h>

#include "Framework/Layer.h"

#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Widget/Widget.h"

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

class IMMORTAL_API WLayer : public Widget
{
public:
    WLayer(Widget *parent = nullptr) :
        Widget{ parent }
    {

    }
};

class RenderContext;
class IMMORTAL_API GuiLayer : public Layer
{
public:
    static constexpr float MinWindowSizeX = 320.0f;
    static constexpr float MinWindowSizeY = 36.0f;

    static FontContext NotoSans;

    static FontContext SimSun;

public:
    GuiLayer();

    virtual ~GuiLayer();

    void OnUpdate() override { }

    virtual void OnAttach() override;

    virtual void OnEvent(Event &e) override;

    virtual void OnGuiRender() override;

    inline virtual void OnDetach() override
    {
        ImGui::DestroyContext();
    }

    void AddLayer(Layer *layer);

    void Begin() override
    {
        ImGui::NewFrame();  
    }

    void End() override
    {
        ImGui::Render();
    }

    void BlockEvent(bool block)
    {
        blockEvents = block;
    }

    void SetTheme();

    bool LoadTheme();

    bool SaveTheme();

    ImFont *DemiLight()
    {
        return NotoSans.Demilight;
    }

    ImFont *Bold()
    {
        return NotoSans.Bold;
    }

    void UpdateTheme();

private:
    std::vector<MonoRef<WLayer>> layers;

    Ref<WDockerSpace> dockspace;
    
    bool blockEvents = true;

    float time = 0.0f;
};

using SuperGuiLayer = GuiLayer;
}
