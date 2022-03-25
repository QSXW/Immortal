#pragma once

#include <imgui.h>

#include "Framework/Layer.h"

#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

#define DEFINE_CPP_STRING_API(FN_NAME, ...) \
    template <class...  Args> \
    static inline bool FN_NAME(const std::string &label, Args &&...args) \
    { \
        return ImGui::FN_NAME(label.c_str(), std::forward<Args>(args)...); \
    }

namespace ImGui
{

template <class T>
inline constexpr ImVec4 ConvertColor(T r, T g, T b, T a)
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

    ~GuiLayer();

    void OnUpdate() { };

    virtual void OnAttach() override;

    virtual void OnEvent(Event &e) override;

    virtual void OnGuiRender() override;

    inline virtual void OnDetach() override
    {
        ImGui::DestroyContext();
    }

    void Begin()
    {
        ImGui::NewFrame();

        static bool p_open             = true;
        static bool optionalPadding    = false;
        static bool optionalFullScreen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        if (optionalFullScreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }
        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        {
            window_flags |= ImGuiWindowFlags_NoBackground;
        }
        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!optionalPadding)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        }
        /* Dock place */
        ImGui::Begin("DockSpace Demo", &p_open, window_flags);

        if (!optionalPadding)
        {
            ImGui::PopStyleVar();
        }
        if (optionalFullScreen)
        {
            ImGui::PopStyleVar(2);
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle &style = ImGui::GetStyle();

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
    }

    void End()
    {
        ImGui::End(); /* Dockspace */
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
    bool blockEvents = true;

    float time = 0.0f;
};

using SuperGuiLayer = GuiLayer;
}
