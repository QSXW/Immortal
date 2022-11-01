/**
 * Copyright (C) 2021-2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include <imgui.h>
#include <imgui_internal.h>

#include "Core.h"
#include "Math/Math.h"
#include "Math/Vector.h"
#include "Interface/IObject.h"
#include "Framework/Timer.h"
#include "ImGui/GuiLayer.h"
#include "String/LanguageSettings.h"
#include "Helper/PlatformUtils.h"
#include "Resource.h"

namespace Immortal
{

static inline ImVec2 operator + (const ImVec2 &a, const ImVec2 &b)
{
    return { a.x + b.x, a.y + b. y };
}

static inline ImVec2 &operator+=(ImVec2 &a, const ImVec2 &b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

enum class Theme
{
    Text                  = ImGuiCol_Text,
    TextDisabled          = ImGuiCol_TextDisabled,
    WindowBg              = ImGuiCol_WindowBg,
    ChildBg               = ImGuiCol_ChildBg,
    PopupBg               = ImGuiCol_PopupBg,
    Border                = ImGuiCol_Border,
    BorderShadow          = ImGuiCol_BorderShadow,
    FrameBg               = ImGuiCol_FrameBg,
    FrameBgHovered        = ImGuiCol_FrameBgHovered,
    FrameBgActive         = ImGuiCol_FrameBgActive,
    TitleBg               = ImGuiCol_TitleBg,
    TitleBgActive         = ImGuiCol_TitleBgActive,
    TitleBgCollapsed      = ImGuiCol_TitleBgCollapsed,
    MenuBarBg             = ImGuiCol_MenuBarBg,
    ScrollbarBg           = ImGuiCol_ScrollbarBg,
    ScrollbarGrab         = ImGuiCol_ScrollbarGrab,
    ScrollbarGrabHovered  = ImGuiCol_ScrollbarGrabHovered,
    ScrollbarGrabActive   = ImGuiCol_ScrollbarGrabActive,
    CheckMark             = ImGuiCol_CheckMark,
    SliderGrab            = ImGuiCol_SliderGrab,
    SliderGrabActive      = ImGuiCol_SliderGrabActive,
    Button                = ImGuiCol_Button,
    ButtonHovered         = ImGuiCol_ButtonHovered,
    ButtonActive          = ImGuiCol_ButtonActive,
    Header                = ImGuiCol_Header,
    HeaderHovered         = ImGuiCol_HeaderHovered,
    HeaderActive          = ImGuiCol_HeaderActive,
    Separator             = ImGuiCol_Separator,
    SeparatorHovered      = ImGuiCol_SeparatorHovered,
    SeparatorActive       = ImGuiCol_SeparatorActive,
    ResizeGrip            = ImGuiCol_ResizeGrip,
    ResizeGripHovered     = ImGuiCol_ResizeGripHovered,
    ResizeGripActive      = ImGuiCol_ResizeGripActive,
    Tab                   = ImGuiCol_Tab,
    TabHovered            = ImGuiCol_TabHovered,
    TabActive             = ImGuiCol_TabActive,
    TabUnfocused          = ImGuiCol_TabUnfocused,
    TabUnfocusedActive    = ImGuiCol_TabUnfocusedActive,
    DockingPreview        = ImGuiCol_DockingPreview,
    DockingEmptyBg        = ImGuiCol_DockingEmptyBg,
    PlotLines             = ImGuiCol_PlotLines,
    PlotLinesHovered      = ImGuiCol_PlotLinesHovered,
    PlotHistogram         = ImGuiCol_PlotHistogram,
    PlotHistogramHovered  = ImGuiCol_PlotHistogramHovered,
    TableHeaderBg         = ImGuiCol_TableHeaderBg,
    TableBorderStrong     = ImGuiCol_TableBorderStrong,
    TableBorderLight      = ImGuiCol_TableBorderLight,
    TableRowBg            = ImGuiCol_TableRowBg,
    TableRowBgAlt         = ImGuiCol_TableRowBgAlt,
    TextSelectedBg        = ImGuiCol_TextSelectedBg,
    DragDropTarget        = ImGuiCol_DragDropTarget,
    NavHighlight          = ImGuiCol_NavHighlight,
    NavWindowingHighlight = ImGuiCol_NavWindowingHighlight,
    NavWindowingDimBg     = ImGuiCol_NavWindowingDimBg,
    ModalWindowDimBg      = ImGuiCol_ModalWindowDimBg,
    MaxCount
};

static constexpr float WInherit = -1.0f;

struct WPadding
{
    float left   = 0;
    float right  = 0;
    float top    = 0;
    float bottom = 0;
};

enum class WAlignMode
{
    None,
    VCenter  = BIT(0),
    HCenter  = BIT(1),
    HVCenter = BITS(VCenter, HCenter),
};

SL_DEFINE_BITWISE_OPERATION(WAlignMode, uint32_t)

#define WIDGET_ \
    float renderWidth  = 0;      \
    float renderHeight = 0;      \
    ImVec2 position    = {0, 0};

class Widget;

struct WidgetState
{
    bool isHovered;
    bool isFocused;
    bool isActive;
};

/** Widget Lock
 *
 *  Used to limit the widget scope. Identifier is necessary for constructor
 */
struct WidgetLock
{
    WidgetLock(void *id)
    {
        ImGui::PushID(id);
    }

    ~WidgetLock()
    {
        ImGui::PopID();
    }

    WidgetLock(const WidgetLock &) = delete;
    WidgetLock &operator=(const WidgetLock &) = delete;

    WidgetLock(WidgetLock &&) = delete;
    WidgetLock &operator=(WidgetLock &&) = delete;
};

#define WIDGET_SET_PROPERTY(U, L, T) \
	WidgetType *U(T _##L)            \
	{                                \
		L = _##L;                    \
		return this;                 \
	}                                \
                                     \
	T U() const                      \
	{                                \
		return L;                    \
	}

#define PUSH_PADDING ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {padding.right, padding.bottom}); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {padding.right, padding.bottom});
#define POP_PADDING ImGui::PopStyleVar(2);

#define WIDGET_SET_PROPERTIES(W) \
    using WidgetType = W;        \
    WIDGET_SET_PROPERTY(Width,        width,        float) \
    WIDGET_SET_PROPERTY(Height,       height,       float) \
    WIDGET_SET_PROPERTY(RenderWidth,  renderWidth,  float) \
    WIDGET_SET_PROPERTY(RenderHeight, renderHeight, float) \
                                                         \
    WidgetType *PaddingLeft(float left)                  \
    {                                                    \
        padding.left = left;                             \
        return this;                                     \
    }                                                    \
                                                         \
    WidgetType *PaddingRight(float right)                \
    {                                                    \
        padding.right = right;                           \
        return this;                                     \
    }                                                    \
                                                         \
    WidgetType *PaddingTop(float top)                    \
    {                                                    \
        padding.top = top;                               \
        return this;                                     \
    }                                                    \
                                                         \
    WidgetType *PaddingBottom(float bottom)              \
    {                                                    \
        padding.bottom = bottom;                         \
        return this;                                     \
    }                                                    \
                                                         \
    WidgetType *Padding(const Vector4 &_padding)         \
    {                                                    \
        padding.top    = _padding.x;                     \
        padding.right  = _padding.y;                     \
        padding.bottom = _padding.z;                     \
        padding.left   = _padding.w;                     \
        return this;                                     \
    }                                                    \
                                                         \
    WidgetType *Id(const std::string &id)                \
    {                                                    \
        Identify2WidgetTracker[id]   = this;             \
        Widget2IdentifyTracker[this] = id;               \
        return this;                                     \
    }                                                    \
                                                         \
    const std::string &Id()                              \
	{                                                    \
        static std::string nullRef;                      \
        auto result = Widget2IdentifyTracker.find(this); \
        if (result != Widget2IdentifyTracker.end())      \
        {                                                \
            return result->second;                       \
        }                                                \
        return nullRef;                                  \
	}                                                    \
                                                         \
    WidgetType *Anchors(const Widget *widget)            \
    {                                                    \
        anchored = true;                                 \
        anchors.fill   = widget;                         \
        anchors.top    = widget;                         \
        anchors.bottom = widget;                         \
        anchors.left   = widget;                         \
        anchors.right  = widget;                         \
        return this;                                     \
    }                                                    \
                                                         \
    WidgetType *AnchorsTop(const Widget *widget)         \
    {                                                    \
        anchored = true;                                 \
        anchors.top = widget;                            \
        return this;                                     \
    }                                                    \
                                                         \
    WidgetType *AnchorsBottom(const Widget *widget)      \
    {                                                    \
        anchored = true;                                 \
        anchors.bottom = widget;                         \
        return this;                                     \
    }                                                    \
                                                         \
    WidgetType *AnchorsLeft(const Widget *widget)        \
	{                                                    \
		anchored = true;                                 \
		anchors.left = widget;                           \
		return this;                                     \
	}                                                    \
                                                         \
    WidgetType *AnchorsRight(const Widget *widget)       \
    {                                                    \
        anchored = true;                                 \
        anchors.right = widget;                          \
        return this;                                     \
    }                                                    \
                                                         \
    WidgetType *Resize(const Vector2 &size)              \
    {                                                    \
        width  = size.x;                                 \
        height = size.y;                                 \
        return this;                                     \
    }

#define WIDGET_PROPERTY_TEXT                           \
public:                                                \
    const std::string &Text() const                    \
    {                                                  \
        return text;                                   \
    }                                                  \
                                                       \
    WidgetType *Text(const std::string &_text)         \
    {                                                  \
	    if (!GuiLayer::IsLanguage(Language::English))  \
	    {                                              \
		    text = Byte2UTF8(WordsMap::Get(_text));    \
	    }                                              \
	    else                                           \
	    {                                              \
		    text = Byte2UTF8(_text);                   \
	    }                                              \
                                                       \
	    return this;                                   \
    }                                                  \
protected:                                             \
    std::string text;

#define WIDGET_PROPERTY_COLOR                                      \
public:                                                            \
    WIDGET_SET_PROPERTY(Color, color, const ImVec4 &)              \
    WidgetType *Color(uint32_t rgba)                               \
    {                                                              \
        color = ImGui::RGBA32(rgba);                               \
        return this;                                               \
    }                                                              \
                                                                   \
    WidgetType *Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)  \
	{                                                              \
		color = ImGui::RGBA32(r, g, b, a);                         \
		return this;                                               \
	}                                                              \
                                                                   \
protected:                                                         \
    ImVec4 color;

#define WIDGET_PROPERTY_ALIGN                     \
public:                                           \
	WIDGET_SET_PROPERTY(Align, align, WAlignMode) \
                                                  \
protected:                                        \
	WAlignMode align = WAlignMode::None;

struct WAnchors
{
    const Widget *fill   = nullptr;
	const Widget *left   = nullptr;
	const Widget *right  = nullptr;
	const Widget *top    = nullptr;
	const Widget *bottom = nullptr;
};

class IMMORTAL_API Widget : public IObject
{
public:
    WIDGET_SET_PROPERTIES(Widget)

    static std::unordered_map<std::string, Widget *> Identify2WidgetTracker;
	static std::unordered_map<Widget *, std::string> Widget2IdentifyTracker;


public:
    Widget(Widget *parent = nullptr) :
        parent{}
    {
        AddParent(parent);
        Connect([&]() {
            for (auto &child : children)
            {
                child->RealRender();
            }
            });
    }

    Widget *AddParent(Widget *other)
    {
        if (other)
        {
            other->AddChild(this);
        }

        return other;
    }

    Widget *AddChild(Widget *child)
    {
        if (child)
        {
            child->parent = this;
            children.emplace_back(child);
        }

        return child;
    }

    Widget *AddChildren(std::initializer_list<Widget *> &&widgets)
    {
        for (auto &w : widgets)
        {
            AddChild(w);
        }

        return this;
    }

    Widget *operator[](std::initializer_list<Widget *> &&widgets)
    {
        return AddChildren(std::move(widgets));
    }

    Widget *Wrap(std::initializer_list<Widget *> &&widgets)
    {
        return AddChildren(std::move(widgets));
    }

    void RealRender()
    {
        render();
    }

    void Render()
    {
        SLASSERT(!parent && "Widget::Render{ Only the root node could emit the render functions }");
        RealRender();
    }

    Vector2 Size() const
    {
        return Vector2{ renderWidth, renderHeight };
    }

    ImVec2 Position() const
    {
		return position;
    }

    template <class F>
    Widget *Connect(F f)
    {
        render = f;
        return this;
    }

    template <class T>
    requires std::is_base_of_v<Widget, T>
    T *Query(const std::string &id)
    {
        auto widget = Identify2WidgetTracker.find(id);
        if (widget != Identify2WidgetTracker.end())
        {
			return dynamic_cast<T *>(widget->second);
        }

		return nullptr;
    }

    void __PreCalculateSize()
    {
		if (Id() == "#player")
		{
			int i = 0;
		}

        float x = width;
        float y = height;
        if (width == WInherit)
        {
            x = parent->renderWidth;
        }
        if (height == WInherit)
        {
            y = parent->renderHeight;
        }

        if (width > 0.f && width < 1.0f)
        {
            x = parent->renderWidth * width;
        }
        if (height > 0.f && height < 1.0f)
        {
            y = parent->renderHeight * height;
        }

        if (!anchors.fill && anchored)
        {
            if (anchors.top)
            {
                SLASSERT(anchors.top == parent || anchors.top->parent == parent);
                auto top = anchors.top;
				position = ImVec2{ top->position.x, top->position.y + top->renderHeight };

                if (height == WInherit)
                {
					y -= (position.y - top->parent->position.y);
                }
            }
            if (anchors.bottom)
            {
                SLASSERT(anchors.bottom == parent || anchors.bottom->parent == parent);
                auto bottom = anchors.bottom;
				const_cast<Widget *>(bottom)->__PreCalculateSize();

				position = ImVec2{ bottom->position.x, bottom->position.y };

			    if (bottom == parent)
			    {
					bottom = this;
					position.y += parent->renderHeight;
                }

                if (height == WInherit)
                {
					position = parent->position;
					y = bottom->position.y - parent->position.y;
                }
                else
				{
					position.y -= height;
                }
				ImGuiWindow *window = ImGui::GetCurrentWindow();
				window->DC.CursorPos = position;
            }
            if (anchors.left)
            {
                SLASSERT(anchors.left == parent || anchors.left->parent == parent);
                auto left = anchors.left;
                SLASSERT(false && "Not Implemented yet");
            }
            if (anchors.right)
            {
                SLASSERT(anchors.right == parent || anchors.right->parent == parent);
                const_cast<Widget *>(anchors.right)->__PreCalculateSize();
                auto right = anchors.right;
                SLASSERT(false && "Not Implemented yet");
            }
        }
		else if (anchors.fill)
        {
			x = parent->renderWidth;
			y = parent->renderHeight;
        }

        x -= (padding.right + padding.left);
        y -= (padding.top + padding.bottom);
        renderWidth  = x;
        renderHeight = y;
    }

    void __Trampoline()
    {
        position = ImGui::GetItemRectMin();
        auto relative = position;
        for (auto &child : children)
        {
            child->RealRender();
        }
    }

    #define PUSH_WINDOW_POS(pos) ImGuiWindow *window = ImGui::GetCurrentWindow(); ImVec2 __pos = window->DC.CursorPos; window->DC.CursorPos = pos;
    #define POP_WINDOW_POS 	window->DC.CursorPos = __pos;
    void __RelativeTrampoline()
    {
		position += ImVec2{padding.left, padding.top};
        auto relative = position;
        PUSH_WINDOW_POS(relative)
        for (auto &child : children)
        {
            child->RelativeTo(relative);
            child->RealRender();
            if ((relative.x - position.x + child->renderWidth) < renderWidth)
            {
                relative.x += child->renderWidth;
            }
            else
            {
                relative.y += child->renderHeight;
            }
            window->DC.CursorPos = relative;
        }
        POP_WINDOW_POS
    }

    void __EndRender()
    {

    }

    void RelativeTo(const ImVec2 &pos)
    {
        position = pos;
    }

public:
    Widget *parent;

    std::vector<Widget *> children;

    std::function<void()> render;

	ImVec2 position    = { 0, 0 };

    float width        = WInherit;

	float height       = WInherit;

    float renderWidth  = 0;

	float renderHeight = 0;

    WPadding padding;

    WAnchors anchors;

    bool anchored = false;
};

class IMMORTAL_API WWindow : public Widget
{
public:
    WWindow();
};

class IMMORTAL_API WDemoWindow : public Widget
{
public:
    WDemoWindow(Widget *parent = nullptr) :
        Widget{ parent }
    {
        Connect([&] { ImGui::ShowDemoWindow(&isOpen); });
    }

    bool Triggle()
    {
        isOpen = !isOpen;
    }

protected:
    bool isOpen = true;
};

class IMMORTAL_API WFrame : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WFrame)
	WIDGET_PROPERTY_TEXT
	WIDGET_PROPERTY_COLOR

public:
    WFrame(Widget *parent = nullptr) :
        Widget{ parent }
    {
        Connect([&]() {
            WidgetLock lock{this};
            ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { padding.right, padding.bottom });
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { padding.right, padding.bottom });
            if (ImGui::Begin(text.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar))
            {
                auto [x, y] = ImGui::GetContentRegionAvail();
                RenderWidth(x);
                RenderHeight(y);
                ImGui::BeginChild("###");
                state.isFocused = ImGui::IsWindowFocused();
                state.isHovered = ImGui::IsWindowHovered();

                position = ImGui::GetItemRectMin();
                __RelativeTrampoline();

                ImGui::EndChild();
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();
            });
    }

    bool IsFocused() const
    {
		return state.isFocused;
    }

    bool IsHovered() const
    {
		return state.isHovered;
    }

protected:
	WidgetState state;
};

class IMMORTAL_API WRect : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WRect)
	WIDGET_PROPERTY_COLOR

public:
    WRect(Widget *v = nullptr) :
        Widget{ v }
    {
        Connect([&]() {
            WidgetLock lock{this};
            __PreCalculateSize();

            ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{padding.right, padding.bottom});
            ImGuiWindow *window = ImGui::GetCurrentWindow();
            window->DC.CursorPos = window->DC.CursorPos + ImVec2{padding.left, padding.top};
            Draw({ renderWidth, renderHeight });

            __RelativeTrampoline();

            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            });
    }

    void Draw(const ImVec2 &size)
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            return;
        }

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, 0))
        {
            return;
        }

        window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(color), 0.0f);
    }
};

class IMMORTAL_API WImage : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WImage)

public:
    WImage(Widget *v = nullptr) :
        Widget{ v }
    {
        Connect([&]() {
            WidgetLock lock{ this };

            __PreCalculateSize();
            __PreClaculateImageSize();

            ImVec2 offset = ImGui::GetWindowPos();
            ImVec2 minRegion = ImGui::GetWindowContentRegionMin();
            ImVec2 maxRegion = ImGui::GetWindowContentRegionMax();

            bounds.min = { minRegion.x + offset.x, minRegion.y + offset.y };
            bounds.max = { maxRegion.x + offset.x, maxRegion.y + offset.y };

            //state.isHovered = ImGui::IsWindowHovered();
            //state.isFocused = ImGui::IsWindowFocused();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{renderPadding.right, renderPadding.bottom});
            ImGuiWindow *window = ImGui::GetCurrentWindow();
            window->DC.CursorPos = window->DC.CursorPos + ImVec2{renderPadding.left, renderPadding.top};

            if (descriptor)
            {
                ImGui::Image(
                    (ImTextureID)descriptor,
                    {renderWidth, renderHeight}
                );
            }
            else
            {
                ImGui::Image(
                    (ImTextureID)(uint64_t) *resource.image,
                    {renderWidth, renderHeight},
                    resource.uv._0,
                    resource.uv._1
                );
            }
            ImGui::PopStyleVar();

            __RelativeTrampoline();

            });
    }

    void __PreClaculateImageSize()
    {
        if (descriptor)
        {
            return;
        }

        auto scale = (float)resource.image->Width() / (float)resource.image->Height();
        auto rscale = renderWidth / renderHeight;

        float x = renderWidth;
        float y = renderHeight;
        if (scale > rscale)
        {
            y = x * ((float) resource.image->Height() / (float) resource.image->Width());
        }
        else
        {
            x = y * scale;
        }

        float half = (renderHeight - y) * 0.5f;
		renderPadding.top = padding.top + half;
		renderPadding.bottom = padding.bottom + half;

        half = (renderWidth - x) * 0.5f;
		renderPadding.left = padding.left + half;
		renderPadding.right = padding.right + half;

        renderWidth = x;
        renderHeight = y;
    }

    WIDGET_SET_PROPERTY(Descriptor, descriptor, uint64_t)

    WidgetType *Source(const WImageResource &res)
    {
        resource = res;
        return this;
    }

    Vector2 MinBound() const
    {
        return bounds.min;
    }

    bool IsHovered() const
    {
		return false;
    }

public:
    struct
    {
        Vector2 min;
        Vector2 max;
    } bounds;

    uint64_t descriptor = 0;

    WImageResource resource;

    WPadding renderPadding;
};

class IMMORTAL_API WPopup : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WPopup)
	WIDGET_PROPERTY_TEXT
	WIDGET_PROPERTY_COLOR

public:
    WPopup(Widget *parent = nullptr) :
        Widget{ parent }
    {
        Connect([&] {
            if (isOpen)
            {
                ImGui::OpenPopup(text.c_str());
            }

            ImVec4 backgroupColor = color;
            backgroupColor.w *= std::sin(factor);
            __PreCalculateSize();
            ImGui::SetNextWindowSize({ renderWidth, renderHeight });
            ImGui::PushStyleColor(ImGuiCol_PopupBg, backgroupColor);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{padding.right, padding.bottom});
            if (ImGui::BeginPopup(text.c_str(), ImGuiWindowFlags_NoMove))
            {
                factor = std::min(factor + Time::DeltaTime * 4.f, (float)(0.5f * Math::PI));
                position = ImGui::GetItemRectMin();
                __RelativeTrampoline();
                ImGui::EndPopup();
            }
            else
            {
                factor = 0;
            }
            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar(1);
        });
    }

    void Trigger(bool enable)
    {
        isOpen = enable;
    }

private:
    bool isOpen = false;

    float factor = 0.0f;
};

class IMMORTAL_API WSeparator : public Widget
{
public:
    WIDGET_PROPERTY_TEXT
    WIDGET_PROPERTY_COLOR

public:
    WSeparator(Widget *parent = nullptr) :
        Widget{ parent }
    {
        color = ImGui::RGBA32(119, 119, 119, 88);
        Connect([&] {
            ImGui::PushStyleColor(ImGuiCol_Separator, color);
            ImGui::Separator();
            ImGui::PopStyleColor();
        });
    }
};

class IMMORTAL_API WHBox : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WHBox)

public:
    WHBox(Widget *parent = nullptr) :
        Widget{parent}
    {
        Connect([this] {
            __PreCalculateSize();

            auto relative = position;
            PUSH_WINDOW_POS(relative)
            for (auto &child : children)
            {
                child->RelativeTo(relative);
                child->RealRender();
                relative.x += child->renderWidth;
                window->DC.CursorPos = relative;
            }
            POP_WINDOW_POS
        });
    }
};

class IMMORTAL_API WVBox : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WVBox)

public:
    WVBox(Widget *parent = nullptr) :
        Widget{parent}
    {
        Connect([this] {
            __PreCalculateSize();

            __RelativeTrampoline();
        });
    }
};

class IMMORTAL_API WText : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WText)
	WIDGET_PROPERTY_TEXT
	WIDGET_PROPERTY_COLOR
	WIDGET_PROPERTY_ALIGN

public:
    WText(Widget *parent = nullptr) :
        Widget{parent}
    {
        Connect([&] {
            (void)__PreCalculateSize();

            if (align & WAlignMode::VCenter)
            {
                renderHeight += padding.top + padding.bottom;
                padding.top = padding.bottom = (renderHeight - fontSize) * 0.5f;
            }
            if (align & WAlignMode::HCenter)
            {
                renderWidth += padding.left + padding.right;
                auto [x, y] = ImGui::CalcTextSize(text.c_str());
                padding.left = padding.right = (renderWidth - x) * 0.5f;
            }

            auto fontScale = fontSize / ImGui::GetFontSize();
            ImGui::SetWindowFontScale(fontScale);
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{padding.right, padding.bottom});
            ImGuiWindow *window = ImGui::GetCurrentWindow();
            window->DC.CursorPos = window->DC.CursorPos + ImVec2{padding.left, padding.top};
            ImGui::Text("%s", text.c_str());
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            ImGui::SetWindowFontScale(1.0f);
        });
    }

    WIDGET_SET_PROPERTY(FontSize, fontSize, float     )

protected:
    float fontSize = 16.0f;

    float spacing = 0;
};

class IMMORTAL_API WDockerSpace : public Widget
{
public:
    WDockerSpace() :
        Widget{ nullptr }
    {
        Connect([&]() {
            static bool isOpen = true;
            static bool optionalPadding = false;
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
            ImGui::Begin("DockSpace Demo", &isOpen, window_flags);

            if (!optionalPadding)
            {
                ImGui::PopStyleVar();
            }
            if (optionalFullScreen)
            {
                ImGui::PopStyleVar(2);
            }

            ImGuiIO &io = ImGui::GetIO();
            ImGuiStyle &style = ImGui::GetStyle();

            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }

            __Trampoline();

            ImGui::End();
            });
    }

};

}
