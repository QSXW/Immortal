#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include "Core.h"
#include "Framework/Layer.h"
#include "Shared/IObject.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Graphics/LightGraphics.h"
#include "String/IString.h"

#define DEFINE_CPP_STRING_API(FN_NAME, ...) \
    template <class...  Args> \
    static inline bool FN_NAME(const std::string &label, Args &&...args) \
    { \
        return ImGui::FN_NAME(label.c_str(), std::forward<Args>(args)...); \
    }


#define kDragDropProxyDirectoryEntry "@DirectoryEntry"

namespace Iconfont
{

//#define kSkipNext     "\xee\xa4\x81"
//#define kSkipPrevious "\xee\xa4\x82"

//#define kPlayArrow            "\xee\xa4\x84"
//#define kPause                "\xee\xa4\x82"
//#define kPreviousFrame        "\xee\xa4\x83"
//#define kNextFrame            "\xee\xa4\x8c"
//#define kLineStartCircle      "\xee\xa4\x8a"
//#define kLineEndCircle        "\xee\xa4\x8b"
//#define kArrowForward         "\xee\xa4\x80"
//#define kArrowBack            "\xee\xa4\x81"
//#define kArrowUpward          "\xee\xa4\x89"
//#define kArrowLeft            "\xee\xa4\x85"
//#define kArrowRight           "\xee\xa4\x87"
//#define kArrowDown            "\xee\xa4\x88"
//#define kArrowUp              "\xee\xa4\x86"
//#define ICON_ARROW_FORWARD    "\xee\xa4\x83"
//#define kMiddlePoint          "\xee\xa4\x84"
//#define kWifi1Bar             "\xee\xa4\x9e"
//#define kLink                 "\xee\xa4\x9d"
//#define kLinkOff              "\xee\xa4\x9c"
//#define kStream               "\xee\xa4\xa0"
//#define kPhotoCamera          "\xee\xa4\xa1"
//#define kRadioButtonUnchecked "\xee\xa4\xa2"
//#define kCamera               "\xee\xa4\xa3"
//#define kHistoryToggleOff     "\xee\xa4\xa4"
//#define kStarFill             "\xee\xa4\xa5"
//#define kKidStarFill          "\xee\xa4\xa6"
//#define kKidStar              "\xee\xa4\xa7"
//#define kStar                 "\xee\xa4\xa8"
//#define kCheckBox             "\xee\xa4\xa9"
//#define kCheckBoxOutline      "\xee\xa4\xaa"


#define kAdd                      "\xee\x80\x80"
#define kArrowBack                "\xee\x80\x81"
#define kArrowForward             "\xee\x80\x82"
#define kArrowUpward              "\xee\x80\x83"
#define kCamera                   "\xee\x80\x84"
#define kCameraFill               "\xee\x80\x85"
#define kCheckBoxFill             "\xee\x80\x86"
#define kCheckBoxOutlineBlankFill "\xee\x80\x87"
#define kClose                    "\xee\x80\x88"
#define kDescription              "\xee\x80\x89"
#define kDot                      "\xee\x80\x8a"
#define kExposureFill             "\xee\x80\x8b"
#define kFileExportFill           "\xee\x80\x8c"
#define kFileOpenFill             "\xee\x80\x8d"
#define kFilePngFill              "\xee\x80\x8e"
#define kFolder                   "\xee\x80\x8f"
#define kFolderOpenFill           "\xee\x80\x90"
#define kHistoryToggleOffFill     "\xee\x80\x91"
#define kKeyboardArrowDown        "\xee\x80\x92"
#define kKeyboardArrowLeft        "\xee\x80\x93"
#define kKeyboardArrowRight       "\xee\x80\x94"
#define kKeyboardArrowUp          "\xee\x80\x95"
#define kKidStar                  "\xee\x80\x96"
#define kKidStarFill              "\xee\x80\x97"
#define kLineEndCircle            "\xee\x80\x98"
#define kLineStartCircle          "\xee\x80\x99"
#define kLink                     "\xee\x80\x9a"
#define kLinkOff                  "\xee\x80\x9b"
#define kLockFill                 "\xee\x80\x9c"
#define kLockOpenFill             "\xee\x80\x9d"
#define kMicFill                  "\xee\x80\x9e"
#define kMicOffFill               "\xee\x80\x9f"
#define kMimoDisconnect           "\xee\x80\xa0"
#define kPanorama                 "\xee\x80\xa1"
#define kPause                    "\xee\x80\xa2"
#define kPhotoCamera              "\xee\x80\xa3"
#define kPhotoCameraFill          "\xee\x80\xa4"
#define kPlayArrow                "\xee\x80\xa5"
#define kRadioButtonUncheckedFill "\xee\x80\xa6"
#define kSkipNext                 "\xee\x80\xa7"
#define kSkipPrevious             "\xee\x80\xa8"
#define kSpeed025                 "\xee\x80\xa9"
#define kSpeed02x                 "\xee\x80\xaa"
#define kSpeed05x                 "\xee\x80\xab"
#define kSpeed05                  "\xee\x80\xac"
#define kSpeed075                 "\xee\x80\xad"
#define kSpeed07x                 "\xee\x80\xae"
#define kSpeed125                 "\xee\x80\xaf"
#define kSpeed12x                 "\xee\x80\xb0"
#define kSpeed12                  "\xee\x80\xb1"
#define kSpeed15x                 "\xee\x80\xb2"
#define kSpeed15                  "\xee\x80\xb3"
#define kSpeed175                 "\xee\x80\xb4"
#define kSpeed17x                 "\xee\x80\xb5"
#define kSpeed2x                  "\xee\x80\xb6"
#define kStar                     "\xee\x80\xb7"
#define kStarFill                 "\xee\x80\xb8"
#define kStream                   "\xee\x80\xb9"
#define kVisibilityFill           "\xee\x80\xba"
#define kVisibilityOffFill        "\xee\x80\xbb"
#define kWifi1Bar                 "\xee\x80\xbc"
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

struct WindowCursorSwitcher
{
	WindowCursorSwitcher(const ImVec2 &pos)
    {
		_pos = ImGui::GetCursorScreenPos();
		_size = ImGui::GetWindowSize();
		ImGui::SetCursorScreenPos(pos);
    }

    WindowCursorSwitcher(const ImVec2 &pos, const ImVec2 &size) :
	    WindowCursorSwitcher{ pos }
    {
		ImGui::SetWindowSize(size);
    }

    ~WindowCursorSwitcher()
    {
		ImGui::SetCursorScreenPos(_pos);
		ImGui::SetWindowSize(_size);
    }

    ImVec2 _pos;
	ImVec2 _size;
};

struct FontStack
{
public:
    FontStack(ImFont *font, float scale = 1.0f) :
	    font{font},
	    lastScale{ font->Scale }
    {
		font->Scale = scale;
		ImGui::PushFont(font);
    }

    ~FontStack()
    {
		ImGui::PopFont();
		font->Scale = lastScale;
		ImGui::SetCurrentFont(font);
    }

    ImFont *font;
	float lastScale;
};

struct FontSizeStack
{
public:
	FontSizeStack(ImFont *font, float fontSize) :
	    fontStack{ font, fontSize / font->FontSize }
	{
	}

    FontSizeStack(float fontSize) :
	    FontSizeStack{ ImGui::GetFont(), fontSize }
    {

    }

	~FontSizeStack()
	{

	}
    
	FontStack fontStack;
};

struct DisabledWhen
{
public:
	DisabledWhen(bool condition = true)
	{
		ImGui::BeginDisabled(condition);
	}

	~DisabledWhen()
	{
		ImGui::EndDisabled();
	}
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

    void SubmitRenderDrawCommands(CommandBuffer *commandBuffer, GPUEvent *gpuEvent, uint64_t syncValue);

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
    
    static void SaveWindowLayout(const String &path = {});

protected:
	void __Begin()
	{
		ImGui::NewFrame();
	}

	void __End()
	{
		ImGui::Render();
	}

    void SmoothScroll();

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

	ImVec2 scrollEnergy = ImVec2(0.0f, 0.0f);
};

using SuperGuiLayer = GuiLayer;

namespace Interface
{
    using GuiLayer = SuperGuiLayer;
}

}
