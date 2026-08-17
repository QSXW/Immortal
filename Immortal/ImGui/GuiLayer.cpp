#include "GuiLayer.h"

#include "ImGuizmo.h"
#include "ImGuiNotify.hpp"
#include "imgui_impl_immortal.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>

#include "Framework/Application.h"
#include "Render/Graphics.h"

#include "String/LanguageSettings.h"
#include "FileSystem/Stream.h"
#include "Shared/Async.h"
#include "Widget/Widget.h"
#include "FileSystem/FileSystem.h"

#ifdef _WIN32
#include <backends/imgui_impl_win32.h>
#include "Graphics/Window/DirectWindow.h"
#endif
#include <backends/imgui_impl_glfw.h>

struct GLFWwindow;
extern "C" void glfwMakeContextCurrent(GLFWwindow *handle);

namespace Immortal
{

#ifdef _WIN32
    static const std::string SystemFontPath = { "C:\\Windows\\Fonts\\" };
#endif

FontContext GuiLayer::NotoSans;
FontContext GuiLayer::SimSun;

GuiLayer *GuiLayer::This = nullptr;

static uint64_t TotalFrame     = 0;
static double TotalFrameRate = 0;

namespace
{

struct WindowLayoutState
{
    std::filesystem::path path;
    std::filesystem::path backupPath;
};

WindowLayoutState &GetWindowLayoutState()
{
    static WindowLayoutState state;
    return state;
}

bool IsUsefulIniData(const std::string &data)
{
    return data.size() > 16 && data.find('[') != std::string::npos && data.find(']') != std::string::npos;
}

bool ReadUsefulIniFile(const std::filesystem::path &path, std::string &data)
{
    data.clear();
    if (path.empty())
    {
        return false;
    }

    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec)
    {
        return false;
    }

    const uintmax_t size = std::filesystem::file_size(path, ec);
    if (ec || size <= 16)
    {
        return false;
    }

    std::ifstream stream{ path, std::ios::binary };
    if (!stream.is_open())
    {
        return false;
    }

    data.assign(
        std::istreambuf_iterator<char>{ stream },
        std::istreambuf_iterator<char>{});
    return IsUsefulIniData(data);
}

bool WriteFileAtomically(const std::filesystem::path &path, const char *data, size_t size)
{
    if (path.empty() || !data || size == 0)
    {
        return false;
    }

    std::error_code ec;
    const std::filesystem::path parent = path.parent_path();
    if (!parent.empty())
    {
        std::filesystem::create_directories(parent, ec);
        if (ec)
        {
            LOG::WARN("Failed to create ImGui layout directory {}: {}", parent.string(), ec.message());
            return false;
        }
    }

    std::filesystem::path temporaryPath = path;
    temporaryPath += ".tmp";
    std::filesystem::remove(temporaryPath, ec);
    ec.clear();

    {
        std::ofstream stream{ temporaryPath, std::ios::binary | std::ios::trunc };
        if (!stream.is_open())
        {
            LOG::WARN("Failed to open temporary ImGui layout file {}", temporaryPath.string());
            return false;
        }

        stream.write(data, static_cast<std::streamsize>(size));
        stream.flush();
        if (!stream.good())
        {
            LOG::WARN("Failed to write temporary ImGui layout file {}", temporaryPath.string());
            return false;
        }
    }

    std::filesystem::remove(path, ec);
    ec.clear();
    std::filesystem::rename(temporaryPath, path, ec);
    if (ec)
    {
        LOG::WARN("Failed to replace ImGui layout file {}: {}", path.string(), ec.message());
        std::filesystem::remove(temporaryPath, ec);
        return false;
    }

    return true;
}

bool SaveWindowLayoutToPath(const std::filesystem::path &path, bool updateBackup)
{
    WindowLayoutState &layout = GetWindowLayoutState();
    size_t size = 0;
    const char *data = ImGui::SaveIniSettingsToMemory(&size);
    if (!data || !IsUsefulIniData(std::string{ data, size }))
    {
        LOG::WARN("Skipped saving ImGui layout because generated ini data is empty");
        return false;
    }

    if (!WriteFileAtomically(path, data, size))
    {
        return false;
    }

    if (updateBackup && !layout.backupPath.empty())
    {
        WriteFileAtomically(layout.backupPath, data, size);
    }

    return true;
}

bool SaveWindowLayoutIfNeeded(bool force)
{
    WindowLayoutState &layout = GetWindowLayoutState();
    if (layout.path.empty())
    {
        return false;
    }

    ImGuiIO &io = ImGui::GetIO();
    if (!force && !io.WantSaveIniSettings)
    {
        return true;
    }

    const bool saved = SaveWindowLayoutToPath(layout.path, true);
    if (saved)
    {
        io.WantSaveIniSettings = false;
    }
    return saved;
}

}

GuiLayer::GuiLayer(Device *device, Queue *queue, Window *window, Swapchain *swapchain) :
    Layer{ "Immortal Graphics User Interface Layer" },
    dockspace{ new WDockerSpace{} },
    device{ device },
    queue{ queue },
    swapchain{ swapchain },
    window{ window },
    platformSpecficWindow{}
{
    This = this;

    themeEditor = new WWindow;
}

GuiLayer::~GuiLayer()
{
	SLASSERT(!platformSpecficWindow.NewFrame && "OnDetach isn't called!");
    ImGui::DestroyContext();
    This = nullptr;

    LOG::INFO("Rendered {} frame(s), Avarage Frame Rate: {}", TotalFrame, TotalFrameRate / TotalFrame);
}

ImFont *AddFontFromImage(const String &path, float size_pixels, const ImFontConfig *font_cfg_template, const ImWchar *glyph_ranges)
{
    Picture picture = Vision::Read(path);
    if (!picture)
    {
		LOG::ERR("Failed to read font {}", path);
		return nullptr;
    }

	size_t pos = path.ReverseFind('.');
	String dataPath = path.Substring(0, pos + 1);
	dataPath += "dat";

	Stream stream{dataPath, Stream::Mode::Read};
	std::vector<uint8_t> data;
	if (!stream.Readable())
	{
		LOG::ERR("Failed to read font data for {}", path);
		return nullptr;
	}
	stream.Read(data);
	ImGuiIO &io = ImGui::GetIO();
	return io.Fonts->AddFontFromImageAndGlyphData(picture.GetData(), picture.GetWidth(), picture.GetHeight(), data.data(), data.size(), size_pixels, font_cfg_template, glyph_ranges);
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
    //io.ConfigFlags  |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowMinSize.x      = MinWindowSizeX;
    style.WindowMinSize.y      = MinWindowSizeY;
    style.WindowBorderSize     = 0.0f;
    style.ScrollbarRounding    = 0.0f;
    style.ScrollbarSize        = 16.0f;
    style.DockingSeparatorSize = 1.2f;
	style.TabRounding            = 0.0f;
	style.TabBorderSize          = 0.0f;
	style.TabBarBorderSize       = 0.0f;
	style.TabBarOverlineSize     = 2.0f;
	style.AntiAliasedLines       = true;
	style.AntiAliasedLinesUseTex = true;
	style.AntiAliasedFill        = true;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

#ifndef IMGUI_DISABLE_SDF
	style.WindowShadowSize = 0;
	style.FrameShadowSize  = 0;
    style.FontShadowSize   = 0;
#endif

    unscaledStyle = style;
    unscaledStyleReady = true;

    io.DisplaySize.x = window->GetWidth();
    io.DisplaySize.y = window->GetHeight();
	ImGui_ImplImmortal_Init(device, window, queue, swapchain, 3, ImGuiBackendFlags_DefaultDesktop);

    decltype(&ImGui_ImplGlfw_NewFrame) NewWindowFrame;
    decltype(&ImGui_ImplGlfw_Shutdown) ShutDownWindow;

#ifdef _WIN32
    if (window->GetType() == WindowType::Win32)
    {
         ImGui_ImplWin32_Init(window->GetBackendHandle());
         platformSpecficWindow.NewFrame = ImGui_ImplWin32_NewFrame;
         platformSpecficWindow.ShutDown = ImGui_ImplWin32_Shutdown;
    }
    else
#endif
    {
        GLFWwindow *glfwWindow = (GLFWwindow *)window->GetBackendHandle();
        if (device->GetBackendAPI() == BackendAPI::OpenGL)
        {
            ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
        }
        else if (device->GetBackendAPI() == BackendAPI::Vulkan)
        {
            ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);
        }
        else if (device->GetBackendAPI() == BackendAPI::Metal)
        {
            ImGui_ImplGlfw_InitForOther(glfwWindow, true);
        }
        platformSpecficWindow.NewFrame = ImGui_ImplGlfw_NewFrame;
        platformSpecficWindow.ShutDown = ImGui_ImplGlfw_Shutdown;
    }
}

void GuiLayer::SetUiLayoutScale(float scale)
{
    if (!ImGui::GetCurrentContext() || !unscaledStyleReady)
    {
        return;
    }

    scale = std::max(0.01f, scale);
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 colors[ImGuiCol_COUNT];
    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
        colors[i] = style.Colors[i];
    }

    const float fontSizeBase = style.FontSizeBase;
    const float fontScaleDpi = style.FontScaleDpi;
    const float nextFrameFontSizeBase = style._NextFrameFontSizeBase;
    style = unscaledStyle;
    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
        style.Colors[i] = colors[i];
    }
    style.FontSizeBase = fontSizeBase;
    style.FontScaleMain = unscaledStyle.FontScaleMain * scale;
    style.FontScaleDpi = fontScaleDpi;
    style._NextFrameFontSizeBase = nextFrameFontSizeBase;
    style.ScaleAllSizes(scale);
}

void GuiLayer::OnDetach()
{
    SaveWindowLayoutIfNeeded(true);
    platformSpecficWindow.ShutDown();
    ImGui_ImplImmortal_Shutdown();

    platformSpecficWindow = {};
}

void GuiLayer::SmoothScroll()
{
	auto &io = ImGui::GetIO();

	const float kScrollSmoothing = 8.0f;
	ImVec2 scroll = ImVec2(0.0f, 0.0f);
	if (std::abs(scrollEnergy.x) > 0.01f)
	{
		scroll.x = scrollEnergy.x * io.DeltaTime * kScrollSmoothing;
		scrollEnergy.x -= scroll.x;
	}
	else
	{
		scrollEnergy.x = 0.0f;
	}
	if (std::abs(scrollEnergy.y) > 0.01f)
	{
		scroll.y = scrollEnergy.y * io.DeltaTime * kScrollSmoothing;
		scrollEnergy.y -= scroll.y;
	}
	else
	{
		scrollEnergy.y = 0.0f;
	}

	io.MouseWheel  =  scroll.y;
	io.MouseWheelH = -scroll.x;
}

void GuiLayer::Begin()
{
    ImGuiIO &io = ImGui::GetIO();
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    ImGui_ImplImmortal_NewFrame();
	platformSpecficWindow.NewFrame();
    if (Application::This && Application::This->UsesInternalHiResUi())
    {
        // Preserve the platform density and add the internal UI render-target density.
        const float scale = Application::This->GetUiRenderScale();
        io.DisplayFramebufferScale.x *= scale;
        io.DisplayFramebufferScale.y *= scale;
    }

    // This ImGui branch does not copy DisplayFramebufferScale into the main
    // viewport. Keep the viewport, draw-data and font density on one scale.
    ImGui::GetMainViewport()->FramebufferScale = io.DisplayFramebufferScale;
    SmoothScroll();
    ImGui::NewFrame();
    ImGui::GetMainViewport()->FramebufferScale = io.DisplayFramebufferScale;

    if (pendingExternalFileDrop &&
        ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip | ImGuiDragDropFlags_SourceExtern))
    {
		FileSystem::DirectoryEntry *entry = &dragDropSources;
		ImGui::SetDragDropPayload(kDragDropProxyDirectoryEntry, &entry, sizeof(entry));
        ImGui::EndDragDropSource();
        pendingExternalFileDrop = false;
    }
}

void GuiLayer::End()
{
#ifdef _WIN32
	/** So WM_NCHITTEST can return HTCLIENT on menu items (borderless drag uses HTCAPTION in the menu strip). */
	if (ImGuiWindow *mb = ImGui::FindWindowByName("##MainMenuBar"))
	{
		const ImRect r = mb->Rect();
		const ImVec2 mp = ImGui::GetIO().MousePos;
		const bool inMenu = r.Contains(mp);
		DirectWindow::SetBorderlessCaptionPreferClient(inMenu && ImGui::IsAnyItemHovered());
	}
	else
	{
		DirectWindow::SetBorderlessCaptionPreferClient(false);
	}
#endif
    ImGui::Render();
    SaveWindowLayoutIfNeeded(false);
}

bool GuiLayer::LoadWindowLayout(const std::string &path)
{
	ImGuiIO &io = ImGui::GetIO();
    WindowLayoutState &layout = GetWindowLayoutState();
    layout.path = std::filesystem::path{ path };
    layout.backupPath = layout.path;
    layout.backupPath += ".bak";
    io.IniFilename = nullptr;

    std::string data;
    if (ReadUsefulIniFile(layout.path, data))
    {
        ImGui::ClearIniSettings();
        ImGui::LoadIniSettingsFromMemory(data.data(), data.size());
        std::string backupData;
        if (!ReadUsefulIniFile(layout.backupPath, backupData))
        {
            WriteFileAtomically(layout.backupPath, data.data(), data.size());
        }
        io.WantSaveIniSettings = false;
        return true;
    }

    if (ReadUsefulIniFile(layout.backupPath, data))
    {
        ImGui::ClearIniSettings();
        ImGui::LoadIniSettingsFromMemory(data.data(), data.size());
        WriteFileAtomically(layout.path, data.data(), data.size());
        io.WantSaveIniSettings = false;
        LOG::WARN("Restored ImGui layout from backup {}", layout.backupPath.string());
        return true;
    }

    io.WantSaveIniSettings = false;
    LOG::WARN("No usable ImGui layout found at {} or {}", layout.path.string(), layout.backupPath.string());
    return false;
}

bool GuiLayer::SaveWindowLayout(const String &path)
{
    if (!path.empty())
    {
        return SaveWindowLayoutToPath(std::filesystem::path{ path.c_str() }, false);
    }

    if (!GetWindowLayoutState().path.empty())
    {
        return SaveWindowLayoutIfNeeded(true);
    }

    ImGuiIO &io = ImGui::GetIO();
    if (io.IniFilename)
    {
        ImGui::SaveIniSettingsToDisk(io.IniFilename);
        return true;
    }
    return false;
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

    if (e.GetType() == Event::Type::WindowDragDrop)
    {
		WindowDragDropEvent &dragDropEvent = (WindowDragDropEvent &)e;
		size_t size = dragDropEvent.GetSize();
		if (size != 0)
        {
			LOG::DEBUG("Queuing {} dropped file(s) for the ImGui target; first path: {}", size, dragDropEvent.QueryFile(0));
			if (size > 1)
            {
				dragDropSources = {};
				auto &sub = dragDropSources.subdirectories;
				sub.resize(size);
				for (size_t i = 0; i < size; i++)
				{
					sub[i] = {dragDropEvent.QueryFile(i), FileType::RegularFile};
				}
            }
            else
            {
				dragDropSources = {
				    dragDropEvent.QueryFile(0),
				    FileType::RegularFile};
            }
			pendingExternalFileDrop = true;
        }
    }
    else if (e.GetType() == Event::Type::MouseScrolled)
    {
        MouseScrolledEvent &event = (MouseScrolledEvent &)e;
        scrollEnergy.x += event.GetOffsetX();
        scrollEnergy.y += event.GetOffsetY();
    }

    dockspace->OnEvent(e);
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

     if (ImGui::Button(Translator::Translate("Save Theme").c_str(), ImVec2{128.0f, 72.0f}))
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
    {
		FontSizeStack fontSize{ NotoSans.Bold, 18.f};
		StyleVarStack<float> styleVar1{
		    { ImGuiStyleVar_ScrollbarRounding, 0.0f},
		    { ImGuiStyleVar_ScrollbarSize,     16.0f}
        };

        StyleColorStack<uint32_t> styleColor2{
            { ImGuiCol_WindowBg,             0xff222222},
		    { ImGuiCol_ChildBg,              0xff222222},
		    { ImGuiCol_Border,               0xff3b3b3b},
            { ImGuiCol_Separator,            0xff3b3b3b},
		    { ImGuiCol_TitleBg,              0xff222222},
		    { ImGuiCol_TitleBgActive,        0xff222222},
		    { ImGuiCol_Tab,                   0xff222222},
		    { ImGuiCol_TabHovered,            0xff2d2d2d},
		    { ImGuiCol_TabSelected,           0xff222222},
		    { ImGuiCol_TabSelectedOverline,   0xffff8844},
		    { ImGuiCol_TabDimmed,             0xff222222},
		    { ImGuiCol_TabDimmedSelected,     0xff222222},
		    { ImGuiCol_TabDimmedSelectedOverline, 0xff5a5a5a},
            { ImGuiCol_ScrollbarBg,          0x0},
            { ImGuiCol_ScrollbarGrab,        0x88444444},
            { ImGuiCol_ScrollbarGrabHovered, 0xdd444444},
            { ImGuiCol_ScrollbarGrabActive,  0xdd444444},
            { ImGuiCol_Button,               0xccff8844},
		    { ImGuiCol_ButtonHovered,        0x33ff8844},
		    { ImGuiCol_ButtonActive,         0x33ff8844},
		    { ImGuiCol_Header,               0xccff8844},
		    { ImGuiCol_HeaderHovered,        0x33ff8844},
		    { ImGuiCol_HeaderActive,         0x33ff8844},
        };

		for (Widget *child : preDockspaceChildren)
		{
			if (child)
			{
				child->Draw();
			}
		}

		dockspace->Render();

        StyleVarStack<float> styleVar{
		    { ImGuiStyleVar_WindowRounding,     0.f  },
		    { ImGuiStyleVar_WindowBorderSize,   0.f  },
		};

        StyleColorStack<ImVec4> styleColor{
			{ ImGuiCol_Button,        ImVec4(0.05f, 0.05f, 0.05f, 0.f)},
			{ ImGuiCol_ButtonHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f)},
			{ ImGuiCol_ButtonActive,  ImVec4(0.20f, 0.22f, 0.23f, 1.00f)},
			{ ImGuiCol_WindowBg,      ImVec4(0.10f, 0.10f, 0.10f, 1.00f)}
        };



		ImGui::RenderNotifications();
    }

    static char title[128] = { 0 };
    std::snprintf(title, sizeof(title), "%s", Application::Name());

    const auto &io = ImGui::GetIO();
    TotalFrame++;
    TotalFrameRate += io.Framerate;
    Application::SetTitle(title);
}

void GuiLayer::SubmitRenderDrawCommands(CommandBuffer *commandBuffer, GPUEvent *gpuEvent, uint64_t syncValue)
{
    ImGui_ImplImmortal_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    // Update and Render additional Platform Windows
    auto &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
         ImGui::UpdatePlatformWindows();
         ImGui::RenderPlatformWindowsDefault();
         if (window->GetType() == WindowType::GLFW)
         {
             glfwMakeContextCurrent((GLFWwindow *)window->GetBackendHandle());
         }
    }
}

void GuiLayer::AddChild(Widget *widget)
{
    dockspace->AddChild(widget);
}

void GuiLayer::AddPreDockspaceChild(Widget *widget)
{
    if (widget)
    {
        preDockspaceChildren.emplace_back(widget);
    }
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
    if (json.is_null())
    {
		return false;
    }

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

ImFont *GuiLayer::AddFont(
    const std::string &path,
    float fontSize,
    const ImWchar *ranges,
    float glyphMinAdvanceX,
    bool mergeMode,
    ImVec2 glyphOffset)
{
	auto &io = ImGui::GetIO();
	ImFontConfig fontConfig = {};
	fontConfig.MergeMode          = mergeMode;
	fontConfig.SignedDistanceFont = true;
	fontConfig.Flags              |= ImFontFlags_NoLoadError;
	fontConfig.GlyphOffset         = glyphOffset;

    if (glyphMinAdvanceX > 0.0f)
	{
		fontConfig.PixelSnapH = true;
		fontConfig.GlyphMinAdvanceX = glyphMinAdvanceX;
	}

	if (auto font = io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize, &fontConfig, ranges))
		return font;

	LOG::ERR("Failed to load font {}", path);
	if (mergeMode)
		return nullptr;

	ImFontConfig fallbackConfig = {};
	fallbackConfig.SizePixels = fontSize;
	fallbackConfig.SignedDistanceFont = true;
	return io.Fonts->AddFontDefaultVector(&fallbackConfig);
}

}
