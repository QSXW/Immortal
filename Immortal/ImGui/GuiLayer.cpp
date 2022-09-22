#include "GuiLayer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "ImGuizmo.h"

#include "Framework/Application.h"
#include "Render/Render.h"

#include "String/LanguageSettings.h"
#include "FileSystem/Stream.h"
#include "Framework/Async.h"
#include "Widget/Widget.h"

namespace Immortal
{

#ifdef WINDOWS
    static const std::string SystemFontPath = { "C:\\Windows\\Fonts\\" };
#endif

FontContext GuiLayer::NotoSans;
FontContext GuiLayer::SimSun;

GuiLayer *GuiLayer::__instance = nullptr;

GuiLayer::GuiLayer() :
    Layer{ "Immortal Graphics User Interface Layer" },
    dockspace{ new WDockerSpace{} }
{
	__instance = this;

    themeEditor = new WWindow;
	themeEditor
	    ->Connect([this] {
		    UpdateTheme();
	    });
}

GuiLayer::~GuiLayer()
{
    ImGui::DestroyContext();
	__instance = nullptr;
}

void GuiLayer::OnAttach()
{
#ifdef _DEBUG
    IMGUI_CHECKVERSION();
#endif

    ImGui::CreateContext();

    if (!LoadTheme())
    {
        SetTheme();
    }

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags  |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowMinSize.x = MinWindowSizeX;
    style.WindowMinSize.y = MinWindowSizeY;
    style.WindowBorderSize = 0.0f;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    Profiler p{ "Loading DemiLight File" };
    NotoSans.Demilight = io.Fonts->AddFontFromFileTTF(
        "Assets/Fonts/NotoSansCJKsc-Light.otf",
        20,
        nullptr,
        io.Fonts->GetGlyphRangesChineseSimplifiedCommon()
        );

    NotoSans.Bold = io.Fonts->AddFontFromFileTTF(
        "Assets/Fonts/NotoSansCJKsc-Bold.otf",
	    20,
        nullptr,
        io.Fonts->GetGlyphRangesChineseSimplifiedCommon()
        );

#ifdef WINDOWS
    SimSun.Regular = io.Fonts->AddFontFromFileTTF(
	    std::string{SystemFontPath + std::string{"Simsun.ttc"}}.c_str(),
	    16,
        nullptr,
        io.Fonts->GetGlyphRangesChineseSimplifiedCommon()
        );
#else
    SimSun.Regular = NotoSans.Demilight;
#endif
}

void GuiLayer::OnDetach()
{
	ImGui::DestroyContext();
}

void GuiLayer::SetTheme()
{
    ImGuiStyle *style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.89f, 0.89f, 0.89f, 1.13f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.1058f, 0.1058f, 0.1058f, 0.1058f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.90f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

void GuiLayer::OnEvent(Event &e)
{
    if (blockEvents)
    {
        ImGuiIO &io = ImGui::GetIO();
        e.Handled |= e.IsInCategory(Event::Category::Mouse) & io.WantCaptureMouse;
        e.Handled |= e.IsInCategory(Event::Category::Keyboard) & io.WantCaptureKeyboard;
    }
}

void GuiLayer::UpdateTheme()
{
     ImGuiStyle *style = &ImGui::GetStyle();
     ImVec4* colors = style->Colors;

     ImGui::Begin("Theme Editor");

 #define XX(x) ImGui::ColorEdit4(#x, (float*)&colors[ImGuiCol_##x])
     XX(Text);
     XX(TextDisabled);
     XX(WindowBg);
     XX(ChildBg);
     XX(PopupBg);
     XX(Border);
     XX(BorderShadow);
     XX(FrameBg);
     XX(FrameBgHovered);
     XX(FrameBgActive);
     XX(TitleBg);
     XX(TitleBgActive);
     XX(TitleBgCollapsed);
     XX(MenuBarBg);
     XX(ScrollbarBg);
     XX(ScrollbarGrab);
     XX(ScrollbarGrabHovered);
     XX(ScrollbarGrabActive);
     XX(CheckMark);
     XX(SliderGrab);
     XX(SliderGrabActive);
     XX(Button);
     XX(ButtonHovered);
     XX(ButtonActive);
     XX(Header);
     XX(HeaderHovered);
     XX(HeaderActive);
     XX(Separator);
     XX(SeparatorHovered);
     XX(SeparatorActive);
     XX(ResizeGrip);
     XX(ResizeGripHovered);
     XX(ResizeGripActive);
     XX(Tab);
     XX(TabHovered);
     XX(TabActive);
     XX(TabUnfocused);
     XX(TabUnfocusedActive);
     XX(DockingEmptyBg);
     XX(PlotLines);
     XX(PlotLinesHovered);
     XX(PlotHistogram);
     XX(PlotHistogramHovered);
     XX(TableHeaderBg);
     XX(TableBorderStrong);
     XX(TableBorderLight);
     XX(TableRowBg);
     XX(TableRowBgAlt);
     XX(TextSelectedBg);
     XX(DragDropTarget);
     XX(NavHighlight);
     XX(NavWindowingHighlight);
     XX(NavWindowingDimBg);
     XX(ModalWindowDimBg);
 #undef XX

     if (UI::Button(WordsMap::Get("Save Theme"), ImVec2{ 128.0f, 72.0f }))
     {
         GuiLayer *that = this;
         Async::Execute([&]() -> void {
             that->SaveTheme();
             });
     }

     ImGui::End();
}

void GuiLayer::Render()
{
	ImGui::PushFont(SimSun.Regular);
    dockspace->Render();
	ImGui::PopFont();

    static char title[128] = { 0 };

    const auto &io = ImGui::GetIO();

#ifdef __APPLE__
    sprintf(
        title,
        "%s (Graphics API: %s, Physical Device: %s)",
        Application::Name(),
        Render::Api(),
        Render::GraphicsRenderer()
    );
#else
    sprintf(
        title,
        "%s (Graphics API: %s, Physical Device: %s) %.3f ms/frame (%.1f FPS)",
        Application::Name(),
        Render::Api(),
        Render::GraphicsRenderer(),
        1000.0f / io.Framerate,
        io.Framerate
    );
#endif

    Application::SetTitle(title);
}

void GuiLayer::AddChild(Widget *widget)
{
	dockspace->AddChild(widget);
}

static inline std::string ThemePath = { "Assets/json/theme.json" };

namespace ns
{

void to_json(JSON::SuperJSON &j, const ImVec4 &v)
{
    j = JSON::SuperJSON{
        { "r", v.x },
        { "g", v.y },
        { "b", v.z },
        { "a", v.w },
    };
}

void from_json(const JSON::SuperJSON &j, ImVec4 &v)
{
    j.at("r").get_to(v.x);
    j.at("g").get_to(v.y);
    j.at("b").get_to(v.z);
    j.at("a").get_to(v.w);
}

}

bool GuiLayer::LoadTheme()
{
    auto json = JSON::Parse(ThemePath);

    ImGuiStyle *style = &ImGui::GetStyle();
    ImVec4 *colors = style->Colors;

#define XX(x) ns::from_json(json[#x], colors[ImGuiCol_##x]);
    XX(Text);
    XX(TextDisabled);
    XX(WindowBg);
    XX(ChildBg);
    XX(PopupBg);
    XX(Border);
    XX(BorderShadow);
    XX(FrameBg);
    XX(FrameBgHovered);
    XX(FrameBgActive);
    XX(TitleBg);
    XX(TitleBgActive);
    XX(TitleBgCollapsed);
    XX(MenuBarBg);
    XX(ScrollbarBg);
    XX(ScrollbarGrab);
    XX(ScrollbarGrabHovered);
    XX(ScrollbarGrabActive);
    XX(CheckMark);
    XX(SliderGrab);
    XX(SliderGrabActive);
    XX(Button);
    XX(ButtonHovered);
    XX(ButtonActive);
    XX(Header);
    XX(HeaderHovered);
    XX(HeaderActive);
    XX(Separator);
    XX(SeparatorHovered);
    XX(SeparatorActive);
    XX(ResizeGrip);
    XX(ResizeGripHovered);
    XX(ResizeGripActive);
    XX(Tab);
    XX(TabHovered);
    XX(TabActive);
    XX(TabUnfocused);
    XX(TabUnfocusedActive);
    XX(DockingEmptyBg);
    XX(PlotLines);
    XX(PlotLinesHovered);
    XX(PlotHistogram);
    XX(PlotHistogramHovered);
    XX(TableHeaderBg);
    XX(TableBorderStrong);
    XX(TableBorderLight);
    XX(TableRowBg);
    XX(TableRowBgAlt);
    XX(TextSelectedBg);
    XX(DragDropTarget);
    XX(NavHighlight);
    XX(NavWindowingHighlight);
    XX(NavWindowingDimBg);
    XX(ModalWindowDimBg);
#undef XX

    colors[ImGuiCol_Tab]                = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.90f);
    colors[ImGuiCol_TabHovered]         = colors[ImGuiCol_HeaderHovered];
    // colors[ImGuiCol_TabActive]          = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]       = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_NavHighlight]       = colors[ImGuiCol_HeaderHovered];

    return true;
}

bool GuiLayer::SaveTheme()
{
    ImGuiStyle *style = &ImGui::GetStyle();
    ImVec4 *colors = style->Colors;

    JSON::SuperJSON json;
#define XX(x) ns::to_json(json[#x], colors[ImGuiCol_##x]);
    XX(Text);
    XX(TextDisabled);
    XX(WindowBg);
    XX(ChildBg);
    XX(PopupBg);
    XX(Border);
    XX(BorderShadow);
    XX(FrameBg);
    XX(FrameBgHovered);
    XX(FrameBgActive);
    XX(TitleBg);
    XX(TitleBgActive);
    XX(TitleBgCollapsed);
    XX(MenuBarBg);
    XX(ScrollbarBg);
    XX(ScrollbarGrab);
    XX(ScrollbarGrabHovered);
    XX(ScrollbarGrabActive);
    XX(CheckMark);
    XX(SliderGrab);
    XX(SliderGrabActive);
    XX(Button);
    XX(ButtonHovered);
    XX(ButtonActive);
    XX(Header);
    XX(HeaderHovered);
    XX(HeaderActive);
    XX(Separator);
    XX(SeparatorHovered);
    XX(SeparatorActive);
    XX(ResizeGrip);
    XX(ResizeGripHovered);
    XX(ResizeGripActive);
    XX(Tab);
    XX(TabHovered);
    XX(TabActive);
    XX(TabUnfocused);
    XX(TabUnfocusedActive);
    XX(DockingEmptyBg);
    XX(PlotLines);
    XX(PlotLinesHovered);
    XX(PlotHistogram);
    XX(PlotHistogramHovered);
    XX(TableHeaderBg);
    XX(TableBorderStrong);
    XX(TableBorderLight);
    XX(TableRowBg);
    XX(TableRowBgAlt);
    XX(TextSelectedBg);
    XX(DragDropTarget);
    XX(NavHighlight);
    XX(NavWindowingHighlight);
    XX(NavWindowingDimBg);
    XX(ModalWindowDimBg);
#undef XX

    Stream stream{ ThemePath, Stream::Mode::Write };
    if (!stream.Writable())
    {
        return false;
    }
    stream.Write(json.dump(4));

    return true;
}

}
