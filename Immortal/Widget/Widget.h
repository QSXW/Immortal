/**
 * Copyright (C) 2021-2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>
#include <imgui_internal.h>
#include <tweeny.h>

#include "Core.h"
#include "ImGui/GuiLayer.h"
#include "Math/Math.h"
#include "Math/Vector.h"
#include "Shared/IObject.h"
#include "Framework/Timer.h"
#include "String/IString.h"
#include "String/LanguageSettings.h"
#include "Resource.h"

#define EXPORT_WINDOW auto window = ImGui::GetCurrentWindow();
#define MOVEPOS(x, y) window->DC.CursorPos += { x, y }
#define MOVEX(v) window->DC.CursorPos.x += v
#define MOVEY(v) window->DC.CursorPos.y += v
#define WIMAGE(x) (ImTextureID)(void *)(x)
#define IM_ANONY "###"

namespace Immortal
{

namespace Icon
{
	static const char *Arrows[] = {
        kArrowRight,
        kArrowDown
    };
};

enum ColorStyle
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

static inline ImVec2 operator-(const ImVec2 &a, const ImVec2 &b)
{
    return { a.x - b.x, a.y - b.y };
}

static inline ImVec2 operator+(const ImVec2 &a, const ImVec2 &b)
{
    return { a.x + b.x, a.y + b.y };
}

static inline ImVec2 &operator-=(ImVec2 &a, const ImVec2 &b)
{
    a.x -= b.x;
    a.y -= b.y;

    return a;
}

static inline ImVec2 &operator+=(ImVec2 &a, const ImVec2 &b)
{
    a.x += b.x;
    a.y += b.y;

    return a;
}

inline float GetCenterAlignPosition(float avilableWidth, float itemWidth)
{
	return (avilableWidth - itemWidth) * 0.5f;
}

inline ImVec2 GetCenterAlignPosition(const ImVec2 &region, const ImVec2 &size)
{
	return {GetCenterAlignPosition(region.x, size.x), GetCenterAlignPosition(region.y, size.y)};
}

inline float GetRightAlignPosition(float avilableWidth, float itemWidth)
{
	return avilableWidth - itemWidth;
}

inline ImVec2 GetRightAlignPosition(const ImVec2 &region, const ImVec2 &size)
{
	return {GetRightAlignPosition(region.x, size.x), GetRightAlignPosition(region.y, size.y)};
}

static inline ImRect GetBoundingBox(const ImVec2 &pos, const ImVec2 &size)
{
	return {pos, pos + size};
}

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
    Right    = BIT(2),
    Left     = BIT(3)
};

SL_ENABLE_BITWISE_OPERATOR(WAlignMode)

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
template <class T>
struct WidgetLock
{
    WidgetLock(T id)
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

#define WIDGET_SET_PROPERTY_FUNC(U, L, T) \
public:                              \
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

#define WIDGET_SET_PROPERTY(U, L, T, ...)     \
    WIDGET_SET_PROPERTY_FUNC(U, L, const T &) \
protected:                                    \
    T L{__VA_ARGS__}; \

#define WIDGET_SET_POINTER(U, L, T, ...)     \
	WIDGET_SET_PROPERTY_FUNC(U, L, T *) \
protected:                                    \
	T *L{__VA_ARGS__}; \

#define WIDGET_SET_PROPERTY_CSTR(U, L, ...)      \
public:                                          \
	WIDGET_SET_PROPERTY_FUNC(U, L, const char *) \
protected:                                       \
	const char *L{__VA_ARGS__};                  \

#define PUSH_PADDING ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {padding.right, padding.bottom}); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {padding.right, padding.bottom});
#define POP_PADDING ImGui::PopStyleVar(2);

#define WIDGET_SET_PROPERTIES(W) \
    using WidgetType = W;        \
    WIDGET_SET_PROPERTY_FUNC(Width,        width,        float) \
    WIDGET_SET_PROPERTY_FUNC(Height,       height,       float) \
    WIDGET_SET_PROPERTY_FUNC(RenderWidth,  renderWidth,  float) \
    WIDGET_SET_PROPERTY_FUNC(RenderHeight, renderHeight, float) \
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
    const float &PaddingLeft() const                     \
    {                                                    \
        return padding.left;                             \
    }                                                    \
                                                         \
    const float &PaddingRight() const                    \
    {                                                    \
        return padding.right;                            \
    }                                                    \
                                                         \
    const float &PaddingTop() const                      \
    {                                                    \
        return padding.top;                              \
    }                                                    \
                                                         \
    const float &PaddingBottom() const                   \
    {                                                    \
        return padding.bottom;                           \
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

#define WIDGET_PROPERTY_TEXT                             \
public:                                                  \
    const String &Text() const                           \
    {                                                    \
        return text;                                    \
    }                                                    \
                                                         \
    WidgetType *Text(const String &_text)                \
    {                                                    \
        text = Translator::Translate(_text);             \
	    return this;                                     \
    }                                                    \
protected:                                               \
    String text;

#define WIDGET_SET_CSTR(U, L, ...)   \
	const String &U() const          \
	{                                \
		return L;                    \
	}                                \
                                     \
protected:                           \
	const String &L = {__VA_ARGS__};

#define WIDGET_SET_KCSTR(U, ...) WIDGET_SET_CSTR(U, k##U, __VA_ARGS__)

#define WIDGET_PROPERTY_VAR_COLOR(U, L, ...)                  \
    WIDGET_SET_PROPERTY(U, L, uint32_t, 0xff000000)

#define WIDGET_PROPERTY_COLOR \
    WIDGET_PROPERTY_VAR_COLOR(Color, color)

#define WIDGET_PROPERTY_BACKGROUND_COLOR \
    WIDGET_PROPERTY_VAR_COLOR(BackgroundColor, backgroundColor)

#define WIDGET_PROPERTY_ALIGN                     \
public:                                           \
	WIDGET_SET_PROPERTY_FUNC(Align, align, WAlignMode) \
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

    virtual bool Draw()
    {
		return false;
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
        float x = width;
        float y = height;
        if (width == 0)
        {
			x = ImGui::CalcItemWidth();
        }
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

        x = int(x);
		y = int(y);

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
        position += ImVec2{ padding.left, padding.top };
        auto relative = position;
        for (auto &child : children)
        {
            child->RelativeTo(relative);
            child->__PreCalculateSize();
            auto size = child->padding.left + child->padding.right + child->renderWidth;
            if ((relative.x - position.x + size) < renderWidth)
            {
                relative.x += size;
            }
            else
            {
                relative.y += child->padding.top + child->padding.bottom + child->renderHeight;
            }
        }

        PUSH_WINDOW_POS(position)
        for (auto &child : children)
        {
            window->DC.CursorPos = child->position;
            child->render();
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

class IMMORTAL_API WDockerSpace : public Widget
{
public:
	WDockerSpace();
};

class IMMORTAL_API WDemoWindow : public Widget
{
public:
    WDemoWindow(Widget *parent = nullptr) :
        Widget{ parent }
    {
		Connect([&] { ImGui::ShowDemoWindow(&isOpen); });
    }

    bool Toggle()
    {
        isOpen = !isOpen;
        return isOpen;
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
	WIDGET_SET_PROPERTY(Flags, flags, ImGuiWindowFlags, 0)

public:
	WFrame(Widget *parent = nullptr);

    bool IsFocused() const
    {
		return state.isFocused;
    }

    bool IsHovered() const
    {
		return state.isHovered;
    }

    void SetFocus()
    {
		ImGui::SetWindowFocus(Text().c_str());
    }

    void ResetState()
    {
		state = {};
    }

protected:
	WidgetState state;

    ImVec2 scroll;
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
			Draw();
            });
    }

    virtual bool Draw() override
    {
        WidgetLock lock{ this };

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
                (ImTextureID)(uint64_t)resource.image,
                {renderWidth, renderHeight},
                resource.uv._0,
                resource.uv._1
            );
        }
        ImGui::PopStyleVar();

        __RelativeTrampoline();

        return true;
    }

    void __PreClaculateImageSize()
    {
		size = { renderWidth, renderHeight };
		if (descriptor || !resource.image)
        {
            return;
        }

        auto scale = (float)resource.image->GetWidth() / (float)resource.image->GetHeight();
        auto rscale = renderWidth / renderHeight;

        float x = renderWidth;
        float y = renderHeight;
        if (scale > rscale)
        {
            y = int(x * ((float) resource.image->GetHeight() / (float) resource.image->GetWidth()));
        }
        else
        {
            x = int(y * scale);
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

    WIDGET_SET_PROPERTY_FUNC(Descriptor, descriptor, uint64_t)

    WidgetType *Source(const WImageResource &res)
    {
        resource = res;
        return this;
    }

    Vector2 MinBound() const
    {
        return bounds.min;
    }

    Vector2 GetSize() const
    {
		return size;
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

    Vector2 size;

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
				ImGui::OpenPopup(Text().c_str());
            }

            ImGui::SetNextWindowSize({ renderWidth, renderHeight });

            StyleColorStack<uint32_t> styleColor{
			    {ImGuiCol_PopupBg, Color() }
            };

            StyleVarStack<ImVec2> styleVar{
			    {ImGuiStyleVar_WindowPadding, ImVec2{ padding.right, padding.bottom}}
            };
			if (ImGui::BeginPopup(Text().c_str(), ImGuiWindowFlags_NoMove))
            {
                position = ImGui::GetItemRectMin();
                __RelativeTrampoline();
                ImGui::EndPopup();
            }

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
    WIDGET_PROPERTY_ALIGN

public:
    WHBox(Widget *parent = nullptr) :
        Widget{parent}
    {
        Connect([this] {
            auto pos = position + ImVec2{ padding.left, padding.top };
            auto relative = pos;
            for (auto &child : children)
            {
                child->RelativeTo(relative);
                child->__PreCalculateSize();
                relative.x += child->padding.left + child->renderWidth + child->padding.right;
            }

            size_t totalWidth = relative.x - position.x;
            if (align & WAlignMode::HCenter && totalWidth < renderWidth)
            {
                auto paddingLeft = (renderWidth - totalWidth) * 0.5;
                position.x += paddingLeft;

                for (auto &child : children)
                {
                    child->position.x += paddingLeft;
                }
            }
            else
            {
                position = pos;
            }

            PUSH_WINDOW_POS(position)
            for (auto &child : children)
            {
                window->DC.CursorPos = child->position;
                child->render();
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
            __RelativeTrampoline();
        });
    }
};

class IMMORTAL_API WBox : public Widget
{
public:
	WIDGET_SET_PROPERTIES(WBox)
	WIDGET_SET_PROPERTY(Visible, visible, bool)

public:
	WBox(Widget *parent = nullptr) :
	    Widget{ parent },
	    visible{ true }
	{
		Connect([this] {
            if (visible)
		    {
                for (auto &child : children)
                {
					child->RealRender();
                }
		    }
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
            if (align & WAlignMode::VCenter)
            {
                renderHeight += padding.top + padding.bottom;
                padding.top = padding.bottom = (renderHeight - fontSize) * 0.5f;
            }
            if (align & WAlignMode::HCenter)
            {
                renderWidth += padding.left + padding.right;
				auto [x, y] = ImGui::CalcTextSize(Text().c_str());
                padding.left = padding.right = (renderWidth - x) * 0.5f;
            }

            auto fontScale = fontSize / ImGui::GetFontSize();
            ImGui::SetWindowFontScale(fontScale);
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{padding.right, padding.bottom});
            ImGuiWindow *window = ImGui::GetCurrentWindow();
            window->DC.CursorPos = window->DC.CursorPos + ImVec2{padding.left, padding.top};
			ImGui::Text("%s", Text().c_str());
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            ImGui::SetWindowFontScale(1.0f);
        });
    }

    WIDGET_SET_PROPERTY_FUNC(FontSize, fontSize, float     )

protected:
    float fontSize = 16.0f;

    float spacing = 0;
};

template <class T>
class IMMORTAL_API WDragDropTarget : public Widget
{
public:
    WDragDropTarget(Widget *parent = nullptr) :
        Widget{ parent },
        type{}
    {
        width = 0;
        height = 0;
        Connect([&] {
			Draw();
            });
    }

    virtual bool Draw() override
    {
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type);
			if (payload)
			{
				auto data = *(const T **) payload->Data;
				if (callback)
				{
					callback(data);
				}
			}
			ImGui::EndDragDropTarget();

            return true;
		}

        return false;
    }

    WDragDropTarget *Type(const char *_type)
    {
        type = _type;
        return this;
    }

    template <class C>
    WDragDropTarget *Callback(C &&_callback)
    {
        callback = _callback;
        return this;
    }

protected:
    const char *type;

    std::function<void(const T *)> callback;
};

class WCollapsingHeader
{
public:
    using WidgetType = WCollapsingHeader;
	WIDGET_PROPERTY_TEXT
    WIDGET_SET_PROPERTY(Flags,               flags,               int,      0         )
	WIDGET_SET_PROPERTY(Expanded,            expanded,            bool,     true      )
	WIDGET_SET_PROPERTY(BodyHeigth,          bodyHeigth,          float,    0         )
	WIDGET_SET_PROPERTY(Margin,              margin,              float,    6.0f      )
	WIDGET_SET_PROPERTY(IndentSpacing,       indentSpacing,       float,    8.0f      )
	WIDGET_SET_PROPERTY(ExpandSpeed,         expandSpeed,         float,    3.0f      )
	WIDGET_SET_PROPERTY(ContentPaddingY,     contentPaddingY,     float,    8.0f      )
	WIDGET_SET_PROPERTY(HasBorder,           hasBorder,           bool,     true      )
	WIDGET_SET_PROPERTY(BodyBackgroundColor, bodyBackgroundColor, uint32_t, 0x861f1f1f)

public:
	WCollapsingHeader(bool defaultOpen = false);

	template <class T>
	void Draw(T &&callback)
	{
		EXPORT_WINDOW
        using namespace ImGui;
		//Dummy({margin, 0});
		//ImGui::SameLine(0, 0);

        StyleColorStack<uint32_t> styleColor{
		    {ImGuiCol_ChildBg, BodyBackgroundColor()}
		};
		StyleVarStack<ImVec2> styleVar{};
		StyleVarStack<float> styleVar2{
		    {ImGuiStyleVar_IndentSpacing, 0.0f}
        };

		window->WorkRect.Max.x -= margin;
		auto region = GetContentRegionAvail();

        auto name = Text().c_str();
		if ((expanded = TreeNodeEx(name, flags | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth, "%s%s", Icon::Arrows[expanded], name)))
		{
			factor = tween.step(ExpandSpeed() * Time::DeltaTime);
			DrawBorder(region);

			{
				region = GetContentRegionAvail();
				WidgetLock lock{this};
				ImVec2 size = {region.x, (BodyHeigth() + ContentPaddingY() * 2) * factor};
				if (BodyHeigth() != 0)
				{
					if (BeginChild(window->GetID(this), size, 0, ImGuiWindowFlags_NoDecoration))
					{
						Indent(IndentSpacing() + Margin());
						Dummy({0, ContentPaddingY()});
						callback();
						Dummy({0, ContentPaddingY()});
					}
					EndChild();
				}
			}
			TreePop();
		}
		else
		{
			tween.seek(0);
			DrawBorder(region);
		}
		window->WorkRect.Max.x += margin;
	}

	~WCollapsingHeader()
	{
	}

	void DrawBorder(const ImVec2 &region)
	{
        if (!HasBorder())
        {
			return;
        }

		EXPORT_WINDOW
		ImRect bb{ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
		bb.Max.x = bb.Min.x + region.x - margin;
		window->DrawList->Flags &= ~ImDrawListFlags_SignedDistanceShapes;
		window->DrawList->AddRect(bb.Min, bb.Max, 0xff121212, 0.0f, ImDrawFlags_None, 1.0f);
		window->DrawList->Flags |= ImDrawListFlags_SignedDistanceShapes;
	}

public:
	float factor = 1.0f;

	tweeny::tween<float> tween;
};

class WSelectable
{
public:
    using WidgetType = WSelectable;
    WIDGET_SET_PROPERTY(Opened,   opened,   bool,  false)
    WIDGET_SET_PROPERTY(PaddingY, paddingy, float, 4.0f )

public:
    WSelectable()
    {

    }
    
    template <class T>


    bool Draw(int popupId, bool opended, const ImRect &bb, const ImVec2 &pos, float width, float height, T &data, int &selected, uint32_t hoveredColor)
    {
        using namespace ImGui;

        ImGuiContext &g         = *GImGui;
        const ImGuiStyle &style = g.Style;

        auto textColor       = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]);
        auto backgroundColor = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_FrameBg]);
        auto activeColor     = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_HeaderActive]);

        EXPORT_WINDOW
        bool valueChanged = false;
        for (int i = 0; i < data.size(); i++)
        {
            //FontSizeStack font{ GuiLayer::NotoSans.Bold, 17.0f };
            int flags = 0;
            const char *itemText = data[i].c_str();
			const char *textEnd = itemText + data[i].size();
            if (itemText == nullptr)
            {
                itemText = "*Unknown item*";
            }

            const bool itemSelected = (i == selected);

            WidgetLock lock{ itemText };
            auto id = window->GetID(itemText);

            auto labelSize = ImGui::CalcTextSize(itemText, textEnd);
            ImVec2 size = { std::max(width, labelSize.x), height };
            ImRect bb = { window->DC.CursorPos, {} };
            bb.Max = bb.Min + size;
                
            ItemSize(size, 0);
            if (!ItemAdd(bb, id))
                continue;

            bool hovered;
            bool held;
            bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
            RenderFrame(bb.Min, bb.Max, hovered ? hoveredColor : (itemSelected ? activeColor : backgroundColor), false);

            if (size.y >= height)
            {
                ImRect textRect{ { bb.Min.x + 4.0f, bb.Min.y + PaddingY() }, { bb.Max.x, bb.Max.y - PaddingY() } };
                ImVec4 textClipRect{ textRect.Min.x, textRect.Min.y, textRect.Max.x, textRect.Max.y };
				RenderTextClipped(textRect.Min, textRect.Max, itemText, textEnd, nullptr, {}, &textRect);
            }

            if (hovered && labelSize.x > width)
			{
                if (BeginTooltip())
                {
					ImGui::TextEx(itemText, textEnd);
					EndTooltip();
                }
			}

            if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup))
            {
                CloseCurrentPopup();
            }

            if (pressed && selected != i)
            {
                valueChanged = true;
                selected = i;
            }

            if (itemSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        return valueChanged;
    }
};

enum WComboFlagBits
{
	WComboFlagBit_NoFixedWidth = 1 << 8,
};

class WCombo
{
public:
    using WidgetType = WCombo;
    WIDGET_SET_PROPERTY(TextStartOffset, textStartOffset, float,       4.0f      )
    WIDGET_SET_PROPERTY(Color,           color,           uint32_t,    0xff1f1f1f)
    WIDGET_SET_PROPERTY(BorderColor,     borderColor,     uint32_t,    0xff121212)
    WIDGET_SET_PROPERTY(BorderSize,      borderSize,      float,       1         )
    WIDGET_SET_PROPERTY(Rounding,        rounding,        float,       2         )
    WIDGET_SET_PROPERTY(Selected,        selected,        int,         0         )
    WIDGET_SET_PROPERTY(SeletableUI,     seletableUI,     WSelectable            )
    WIDGET_SET_PROPERTY(PaddingY,        paddingY,        float,       2.0f      )
	WIDGET_SET_PROPERTY(Opened,          opened,          bool,        false     )
	WIDGET_SET_PROPERTY(Tween,           tween,           tweeny::tween<float>, tweeny::from(0.0f).to(1.0f).during(10000).via(tweeny::easing::sinusoidalInOut)   );

public:
	WCombo(int selected = 0) :
        selected{ selected }
    {

    }

    template <class T>
    static const char *StringGetter(void *userData, int index)
    {
        auto &data = *(T *)userData;
        return data[index].c_str();
    }

    using PFN_Getter = const char *(*)(void *, int);

    template <class T>
	bool Draw(T &data, ImGuiComboFlags flags = ImGuiComboFlags_None)
    {
        using namespace ImGui;

        ImGuiContext &g         = *GImGui;
        const ImGuiStyle &style = g.Style;
		EXPORT_WINDOW

        if (data.empty())
        {
			return false;
        }

		bool selectedChange = false;
        auto &io = ImGui::GetIO();

		auto width = ImGui::CalcItemWidth();
        auto lineHeight = ImGui::GetTextLineHeight();
        auto height = lineHeight + PaddingY() * 2;

        auto text = data[selected];
        auto textStart = text.c_str();
        auto textEnd   = text.c_str() + text.size();

        auto id = ImGui::GetID(this);
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = { width, height };
        const ImRect bb(pos, pos + size);
        ItemSize(size, 0);
        if (!ItemAdd(bb, id))
            return false;

        bool hovered = false;
        bool held    = false;
        bool pressed = ButtonBehavior({ bb.Min, {bb.Max.x - height, bb.Max.y} }, id, &hovered, &held);

        auto textColor    = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]);
        auto hoveredColor = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_HeaderHovered]);
        window->DrawList->AddRectFilled(bb.Min, bb.Max, Color(), Rounding(), ImDrawFlags_None);
        window->DrawList->AddRect(bb.Min, bb.Max, BorderColor(), Rounding(), ImDrawFlags_None, BorderSize());

        ImRect textRect{ { bb.Min.x + TextStartOffset(), bb.Min.y + PaddingY() }, {bb.Max.x - height, bb.Max.y - PaddingY() }};
        ImVec4 textClipRect{ textRect.Min.x, textRect.Min.y, textRect.Max.x, textRect.Max.y };
        window->DrawList->AddText(
            GetFont(),
            GetFontSize(),
            textRect.Min,
            textColor,
            textStart,
            textEnd,
            0,
            &textClipRect
        );

        {
            ImGui::SameLine();
            auto iconId = ImHashStr("##Arrow", 0, id);
            ImRect bbIcon = { {bb.Max.x - height, bb.Min.y}, bb.Max };
            WindowCursorSwitcher s(bbIcon.Min);
            
            auto size = bbIcon.Max - bbIcon.Min;
            ItemSize(size, 0);
            if (!ItemAdd(bbIcon, iconId))
                return false;

            bool hovered;
            bool held;
            pressed |= ButtonBehavior(bbIcon, iconId, &hovered, &held);
			window->DrawList->AddText(bbIcon.Min, (hovered || Opened()) ? textColor : 0xffaaaaaa, kArrowDown);
        }

        if (hovered)
        {
            if (ImGui::BeginTooltip())
            {
                ImGui::Dummy({ 0, 2 });
                ImGui::Dummy({ 2, 0 });
                ImGui::SameLine();
                ImGui::Text(textStart, textEnd);
                ImGui::SameLine();
                ImGui::Dummy({ 0, 2 });
                ImGui::Dummy({ 0, 2 });
                EndTooltip();
            }

			if (io.MouseWheel > 0)
			{
				selected = std::max(0, selected - 1);
				selectedChange = true;
			}
			else if (io.MouseWheel < 0)
			{
				selected = std::min((int) data.size() - 1, selected + 1);
				selectedChange = true;
			}
        }

        auto popupId = ImHashStr("##ComboUI", 0, id);
        if (pressed)
		{
			OpenPopupEx(popupId, ImGuiPopupFlags_None);
		}

        if (!IsPopupOpen(popupId, ImGuiPopupFlags_None))
		{
			tween.seek(0);
			g.NextWindowData.ClearFlags();
			Opened(false);
			return selectedChange;
        }

        Opened(true);

        char name[16];
		ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginComboDepth);
		if (ImGuiWindow *popup_window = FindWindowByName(name))
		{
			if (popup_window->WasActive)
			{
				ImVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
				popup_window->AutoPosLastDirection = ImGuiDir_Left; //(flags & ImGuiComboFlags_PopupAlignLeft) ? ImGuiDir_Left : ImGuiDir_Down;;
				ImRect r_outer = GetPopupAllowedExtentRect(popup_window);
				ImVec2 pos = FindBestWindowPosForPopupEx(bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, bb, ImGuiPopupPositionPolicy_ComboBox);
				SetNextWindowPos(pos);
			}
		}
		g.BeginComboDepth++;

        if (!(flags & WComboFlagBit_NoFixedWidth))
		{
			height = ImGui::GetTextLineHeight();
			height += PaddingY() * 4;

			ImVec2 popupSize = {width, height * data.size()};
			popupSize.y *= tween.step(5.0f * Time::DeltaTime);
			SetNextWindowSize(popupSize);
			SetNextWindowPos({bb.Min.x, bb.Max.y});
        }

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
		if (Begin(name, nullptr, windowFlags))
        {
			StyleVarStack<ImVec2> styleVar = {
			    {ImGuiStyleVar_ItemSpacing, {0, 0}}};

			selectedChange |= seletableUI.Draw(popupId, Opened(), bb, {bb.Min.x, bb.Max.y}, width, height, data, selected, hoveredColor);
        }

        ImGui::End();
		g.BeginComboDepth--;

        return selectedChange;
    }
};

static inline bool IconButton(ImGuiID id, const char *text, const char *textEnd = nullptr, const ImVec2 &bbSize = {}, const char *hoveredIcon = nullptr)
{
	using namespace ImGui;
	ImGuiWindow *window = GetCurrentWindow();
	if (window->SkipItems)
	{
		return false;
	}

	ImGuiContext &g = *GImGui;
	const ImGuiStyle &style = g.Style;
    if (id == 0)
	{
		id = window->GetID(text, textEnd);
	}

	ImVec2 textSize = CalcTextSize(text, textEnd);
	textSize = textSize + style.ItemSpacing;
	ImVec2 size = CalcItemSize(bbSize, textSize.x, textSize.y);
	if (size.x == 0 && size.y == 0)
	{
		auto lineHeight = ImGui::GetTextLineHeight();
		size = {lineHeight, lineHeight};
	}

	ImRect bb = {window->DC.CursorPos, window->DC.CursorPos + size};
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
	{
		return false;
	}

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);
	if (hovered)
	{
		if (hoveredIcon)
		{
			text = hoveredIcon;
			textEnd = nullptr;
		}
	}
	uint32_t color = ColorConvertFloat4ToU32(hovered ? style.Colors[ImGuiCol_ButtonHovered] : style.Colors[ImGuiCol_Text]);
	if (textEnd == nullptr)
	{
		textEnd = text + strlen(text);
	}

    auto pos = GetCenterAlignPosition(size, textSize);
	StyleColorStack<uint32_t> styleColor = {{ImGuiCol_Text, color}};
	RenderTextClipped(bb.Min + pos, bb.Max, text, textEnd, &size);

	return pressed;
}

class WIconButton
{
public:
	using WidgetType = WIconButton;
	WIDGET_SET_PROPERTY_CSTR(Icon, icon, nullptr)
	WIDGET_SET_PROPERTY_CSTR(ActiveIcon, activeIcon, nullptr)
	WIDGET_SET_PROPERTY_CSTR(HoveredIcon, hoveredIcon, nullptr)
	WIDGET_SET_PROPERTY(Active, active, bool, false)

public:
	WIconButton(const char *icon = nullptr, const char *activeIcon = nullptr, const char *hoveredIcon = nullptr)
	{
		Icon(icon);
        if (!activeIcon)
        {
			activeIcon = icon;
        }
		ActiveIcon(activeIcon);
		HoveredIcon(hoveredIcon);
	}

	bool Draw(const ImVec2 &size = {})
	{
		ImGuiWindow *window = ImGui::GetCurrentWindow();
		auto visibleIcon = Active() && ActiveIcon() ? ActiveIcon() : Icon();
		bool pressed = IconButton(window->GetID(this), visibleIcon, nullptr, size, HoveredIcon() ? HoveredIcon() : visibleIcon);
		if (pressed)
		{
			Active(!Active());
		}

		return pressed;
	}
};

class IMMORTAL_API WRightClickPopup : public Widget
{
public:
	WIDGET_SET_PROPERTIES(WRightClickPopup)
	WIDGET_PROPERTY_TEXT
	WIDGET_PROPERTY_COLOR
	WIDGET_PROPERTY_BACKGROUND_COLOR
	WIDGET_SET_PROPERTY(HoveredColor,   hoveredColor,    uint32_t)
	WIDGET_SET_PROPERTY(IsMouseClicked, isMousedClicked, bool,    false)
	WIDGET_SET_PROPERTY(ActiveItemId,   activeItemId,    ImGuiID, 0    )
	WIDGET_SET_PROPERTY(ManualOpen,     manualOpen,      bool,    false)

public:
	WRightClickPopup(Widget *parent = nullptr);

    virtual bool Draw() override;

	WidgetType *Items(std::initializer_list<std::pair<const String &, std::function<void()>>> &&list);

	void Open();

	WidgetType *Callback(std::function<void()> value)
	{
		callback = value;
		return this;
	}

protected:
	std::vector<std::pair<const String &, std::function<void()>>> items;

	std::function<void()> callback;

	float factor = 0.0f;

	tweeny::tween<float> tween;
};

class WInputText : public Widget
{
public:
    using WidgetType = WInputText;
	WIDGET_SET_PROPERTY(OutlineColor,       outlineColor,       uint32_t, 0xff000000)
	WIDGET_SET_PROPERTY(ActiveOutlineColor, activeOutlineColor, uint32_t, 0xff000000)
	WIDGET_SET_PROPERTY(OutlineBorderSize, outlineBorderSize, float, 1.1f)
	WIDGET_SET_PROPERTY(Rounding,          rounding,          float, 0.0f)
    WIDGET_PROPERTY_TEXT
	WIDGET_SET_PROPERTY(Hint,              hint, String)

public:
	WInputText();

    virtual bool Draw(const ImVec2 &size = {-1, -1});
};

}
