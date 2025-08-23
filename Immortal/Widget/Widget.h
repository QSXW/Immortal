/**
 * Copyright (C) 2021-2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include <string>
#include <functional>
#include <unordered_map>

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
        kKeyboardArrowRight,
        kKeyboardArrowDown,
    };
};

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
    }

    virtual bool Draw()
    {
		bool ret = false;
		for (auto &child : children)
		{
			ret |= child->Draw();
		}

        return ret;
    }

    virtual bool OnEvent(Event &event)
    {
		bool ret = false;
		for (auto &child : children)
		{
			ret |= child->OnEvent(event);
            if (ret)
            {
				break;
            }
		}

        return ret;
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

    void Render()
    {
        SLASSERT(!parent && "Widget::Render{ Only the root node could emit the render functions }");
		Draw();
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
            child->Draw();
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
            child->Draw();
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

    virtual bool Draw() override;
};

class IMMORTAL_API WDemoWindow : public Widget
{
public:
    WDemoWindow(Widget *parent = nullptr) :
        Widget{ parent }
    {

    }

    virtual bool Draw() override
    {
		ImGui::ShowDemoWindow(&isOpen);
		return isOpen;
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
	WIDGET_SET_PROPERTY(Flags,   flags,   ImGuiWindowFlags, 0   )
	WIDGET_SET_PROPERTY(Visible, visible, bool,             true)

public:
	WFrame(Widget *parent = nullptr);

    virtual bool Draw() override;

    bool IsFocused() const
    {
		return state.isFocused;
    }

    bool IsHovered() const
    {
		return state.isHovered;
    }

    void SetHovered(bool enabled)
    {
		state.isHovered = enabled;
    }

    void AddHovered(bool enabled)
    {
		state.isHovered |= enabled;
    }

    void SetFocus()
    {
		ImGui::SetWindowFocus(Text().c_str());
    }

    void ResetState()
    {
		state = {};
    }

    virtual bool OnEvent(Event &event)
	{
        if (!state.isFocused && !state.isHovered)
        {
			return false;
        }
		return Widget::OnEvent(event);
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

    }

    virtual bool Draw() override
    {
		WidgetLock lock{this};

		ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{padding.right, padding.bottom});
		ImGuiWindow *window = ImGui::GetCurrentWindow();
		window->DC.CursorPos = window->DC.CursorPos + ImVec2{padding.left, padding.top};
		Draw({renderWidth, renderHeight});

		__RelativeTrampoline();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

        return false;
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
	WIDGET_SET_PROPERTY(ImageWidth,  imageWidth,  float,   0.0f)
	WIDGET_SET_PROPERTY(ImageHeight, imageHeight, float,   0.0f)
	WIDGET_SET_PROPERTY(UV0,         uv0,         ImVec2,  0.0f, 0.0f)
	WIDGET_SET_PROPERTY(UV1,         uv1,         ImVec2,  1.0f, 1.0f)
	WIDGET_SET_PROPERTY(Image,       image,       Ref<Texture>)
	WIDGET_SET_PROPERTY(Rotation,    rotation,    float,   0.0f)

public:
    WImage(Widget *v = nullptr) :
        Widget{ v }
    {

    }

    ~WImage()
    {
		Graphics::ReleaseResource(image);
    }

	static inline ImVec2 ImRotate(const ImVec2 &v, float cos_a, float sin_a)
	{
		return ImVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a);
	}

	void ImageRotated(ImTextureID textureId, ImVec2 center, ImVec2 size, float angle)
	{
		ImDrawList *draw_list = ImGui::GetWindowDrawList();

		float cos_a = cosf(angle);
		float sin_a = sinf(angle);
		ImVec2 pos[4] = {
		    center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
		    center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
		    center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
		    center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
        };
		ImVec2 uvs[4] =
		{
		        ImVec2(0.0f, 0.0f),
		        ImVec2(1.0f, 0.0f),
		        ImVec2(1.0f, 1.0f),
		        ImVec2(0.0f, 1.0f)
        };

		draw_list->AddImageQuad(textureId, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], IM_COL32_WHITE);
	}

    virtual bool Draw() override
    {
        using namespace ImGui;
        WidgetLock lock{ this };

		size = ImGui::GetContentRegionAvail();
		width  = size.x;
		height = size.y;

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
    
        ImTextureID id = (ImTextureID)(uint64_t)image.Get();
		ImVec2 size = { renderWidth, renderHeight };
		ImVec2 cursor = GetCursorScreenPos();

        if (rotation != 0)
        {
			if (rotation != -180)
			{
				std::swap(size.x, size.y);
			}
            ImageRotated(
			    id,
			    {cursor.x + size.y * 0.5f, cursor.y + size.x * 0.5f},
			    size,
                Vector::Radians(rotation)
            );
        }
        else
        {
            ImGui::Image(
			    id,
                size,
                uv0,
                uv1
            );
        }

        ImGui::PopStyleVar();

        __RelativeTrampoline();

        return true;
    }

    void __PreClaculateImageSize()
    {
		size = { renderWidth, renderHeight };
		if (!image)
        {
            return;
        }

        auto w = ImageWidth();
		auto h = ImageHeight();

        if (w == 0)
        {
			w = (float)image->GetWidth();
			h = (float)image->GetHeight();
            if (rotation == -90 || rotation == -270)
            {
				std::swap(w, h);
            }
        }

        auto scale = w / h;
        auto rscale = renderWidth / renderHeight;

        float x = renderWidth;
        float y = renderHeight;
        if (scale > rscale)
        {
            y = int(x * (h/ w));
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

    WidgetType *Source(const Ref<Texture> &_image)
    {
		Graphics::ReleaseResource(image);
		image = _image;
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

    }

    virtual bool Draw() override 
    {
		if (isOpen)
		{
			ImGui::OpenPopup(Text().c_str());
		}

		ImGui::SetNextWindowSize({renderWidth, renderHeight});

		StyleColorStack<uint32_t> styleColor{
		    {ImGuiCol_PopupBg, Color()}};

		StyleVarStack<ImVec2> styleVar{
		    {ImGuiStyleVar_WindowPadding, ImVec2{padding.right, padding.bottom}}};
		bool opened = ImGui::BeginPopup(Text().c_str(), ImGuiWindowFlags_NoMove);
        if (opened)
		{
			position = ImGui::GetItemRectMin();
			__RelativeTrampoline();
			ImGui::EndPopup();
		}

        return opened;
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

    }

    virtual bool Draw() override
    {
		ImGui::PushStyleColor(ImGuiCol_Separator, color);
		ImGui::Separator();
		ImGui::PopStyleColor();

        return false;
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

    }

    virtual bool Draw() override
    {
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

		return false;
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

    }

    virtual bool Draw() override
    {
		__RelativeTrampoline();
		return false;
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

	}

    virtual bool Draw() override
    {
		bool ret = false;
		if (visible)
		{
			for (auto &child : children)
			{
				ret |= child->Draw();
			}
		}

        return ret;
    }
};

template <class T>
class WDragDropSource : public Widget
{
public:
    using WidgetType = WDragDropSource;

public:
	WDragDropSource(Widget *parent = nullptr) :
	    Widget{parent}
	{
		width = 0;
		height = 0;
	}

    virtual bool Draw() override
	{
        using namespace ImGui;

        if (BeginDragDropSource())
        {
			EndDragDropSource();
			return true;
        }

		return false;
	}
};

template <class T>
class IMMORTAL_API WDragDropTarget : public Widget
{
public:
    using WidgetType = WDragDropTarget;
	WIDGET_SET_PROPERTY(Flags, flags, ImGuiDragDropFlags, 0)

public:
    WDragDropTarget(Widget *parent = nullptr) :
        Widget{ parent },
        type{}
    {
        width = 0;
        height = 0;
    }

    virtual bool Draw() override
    {
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type, Flags());
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

constexpr float kAlignPaddingY = 2.0f;
constexpr float CalculateCircleCheckboxWidth(float height)
{
    height -= kAlignPaddingY * 2;
    return height * 1.5 + 1;
}

static inline bool CircleCheckbox(const char *label, bool *v, uint32_t backgroundColor, uint32_t enabledColor = 0xCCdd8844, uint32_t disabledColor = 0xAAeeeeee)
{
    //ImGui::Checkbox(label, v);

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImGuiID id = window->GetID(label);

    float height = ImGui::GetFrameHeightWithSpacing();
    const Vector2 pos = window->DC.CursorPos;

    const float width = CalculateCircleCheckboxWidth(height);
	height -= kAlignPaddingY * 2;
    ImVec2 bbMin = {pos.x, pos.y + kAlignPaddingY};
	ImVec2 bbMax = {pos.x + width, bbMin.y + height};

    ImRect bb{bbMin, bbMax};
    window->DrawList->AddRectFilled(bb.Min, bb.Max, backgroundColor, width);

    bool value = *v;
    bool hovered = false;
    bool held    = false;

    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }
    if (ImGui::ButtonBehavior(bb, id, &hovered, &held))
    {
        value = !value;
    }

    float radius = height * 0.5f;
	ImVec2 center = { bbMin.x + radius,bbMin.y + radius };
    if (value)
    {
        center.x += radius + 1;
    }
    *v = value;

    uint32_t color = !value ? disabledColor : enabledColor;
    window->DrawList->AddCircleFilled(center, radius - 2.0f, color);

    return true;
}

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
	WIDGET_SET_PROPERTY(HasBorder,           hasBorder,           bool,     false     )
	WIDGET_SET_PROPERTY(BodyBackgroundColor, bodyBackgroundColor, uint32_t, 0x861f1f1f)
	WIDGET_SET_PROPERTY(CurrentSizeY,        currentSizeY,        float,    0         )
	WIDGET_SET_PROPERTY(TargetSizeY,         targetSizeY,         float,    0         )
    WIDGET_SET_PROPERTY(SizeChanged,         sizeChanged,         bool,     true      )
	WIDGET_SET_PROPERTY(HasTriggerButton,    hasTriggerButton,    bool,     false     )
	WIDGET_SET_PROPERTY(Enabled,             enabled,             bool,     false     )

public:
	WCollapsingHeader(bool defaultOpen = false);

	template <class T>
	void Draw(T &&callback)
	{
        using namespace ImGui;
        
        StyleColorStack<uint32_t> styleColor{
		    {ImGuiCol_ChildBg, BodyBackgroundColor()}
		};

        ImGuiContext &g = *GImGui;
		float indentSpacing = g.Style.IndentSpacing;

		StyleVarStack<ImVec2> styleVar{};
		StyleVarStack<float> styleVar2{
		    {ImGuiStyleVar_IndentSpacing, 0.0f}
        };

		auto region = GetContentRegionAvail();
		
        ImGuiWindow *window = GetCurrentWindow();

		auto name = Text().c_str();
        ImGuiID id = window->GetID(name);

        ImGuiTreeNodeFlags flags = Flags() | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (!hasTriggerButton)
        {
			flags |= ImGuiTreeNodeFlags_SpanFullWidth;
        }
		bool newState = TreeNodeEx(name, flags, "%s%s", Icon::Arrows[expanded], name);

        if (hasTriggerButton)
        {
			auto width = ImGui::GetWindowWidth();
			float height = ImGui::GetFrameHeightWithSpacing();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - height);
			width = width - CalculateCircleCheckboxWidth(height) - 10;
			ImGui::SameLine(width);
			CircleCheckbox("UICheckBox", &enabled, 0xff111111);
        }

		float renderSize = BodyHeigth() + ContentPaddingY() * 2;
        sizeChanged |= newState != expanded;
		expanded = newState;

		if (sizeChanged)
		{
			targetSizeY = expanded ? renderSize : 0.0f;
			if (currentSizeY == 0 && targetSizeY == 0)
			{
				currentSizeY = expanded ? 0.0f : renderSize;
			}

		    tween = tweeny::from(currentSizeY).to(targetSizeY).during(1000).via(tweeny::easing::quadraticOut);
			sizeChanged = false;
		}

		if (expanded || currentSizeY > 0)
		{
			float t = 5 * Time::DeltaTime;
			//currentSizeY = Math::HermiteLerp(currentSizeY, targetSizeY, t);

   //         float diff = std::abs(targetSizeY - currentSizeY);
   //         if (diff < 0.001f)
   //         {
			//	currentSizeY = targetSizeY;
   //         }

            currentSizeY = tween.step(t);
			//currentSizeY = std::clamp(currentSizeY, 0.0f, renderSize);

			//factor = tween.step(ExpandSpeed() * Time::DeltaTime);
			DrawBorder(region);
            
			{
				region = GetContentRegionAvail();
				WidgetLock lock{this};
				ImVec2 size = {region.x, currentSizeY};
				if (size.y > 0 || bodyHeigth == 0)
				{
					if (BeginChild(window->GetID(this), size, bodyHeigth == 0 ? ImGuiChildFlags_AutoResizeY : 0, ImGuiWindowFlags_NoDecoration))
				    {
					    EXPORT_WINDOW
					    Indent(IndentSpacing() + Margin());
					    StyleVarStack<float> styleVar2{
					        {ImGuiStyleVar_IndentSpacing, indentSpacing }};

						float padding = bodyHeigth > 0 ? ContentPaddingY() : 0;
					    Dummy({ 0, padding });
					    auto startY = window->DC.CursorPos.y;
					    callback();

                        float newBodyHeight = window->DC.CursorPos.y - startY;
						if (newBodyHeight != bodyHeigth)
                        {
							sizeChanged = true;
                        }
						BodyHeigth(newBodyHeight);
					    Dummy({ 0, padding });
						Unindent();
				    }
				    EndChild();
                }
			}
            
   //         if (expanded)
			//{
			//	TreePop();
			//}
		}
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
            int flags = 0;
            const char *itemText = data[i].c_str();
			const char *textEnd = itemText + data[i].size();
            if (itemText == nullptr)
            {
                itemText = "*Unknown item*";
            }

            float linePadding = (height - GetTextLineHeight()) * 0.5f;

            WidgetLock lock{itemText};
            const bool itemSelected = (i == selected);
			bool pressed = Selectable("###", itemSelected, 0, ImVec2(width, height));
			if (IsItemVisible())
			{
				ImRect bb{GetItemRectMin(), GetItemRectMax()};
				const char *s = itemText;
				const char *e = textEnd;
				auto labelSize = CalcTextSize(s, e);

				ImVec2 primaryLabelPos{bb.Min.x + 8, bb.Min.y + linePadding};
				RenderTextClipped(primaryLabelPos, bb.Max, s, e, &labelSize);
			}

   //         auto id = window->GetID(itemText);

   //         auto labelSize = ImGui::CalcTextSize(itemText, textEnd);
   //         ImVec2 size = { std::max(width, labelSize.x), height };
   //         ImRect bb = { window->DC.CursorPos, {} };
   //         bb.Max = bb.Min + size;
   //             
   //         ItemSize(size, 0);
   //         if (!ItemAdd(bb, id))
   //             continue;

   //         bool hovered;
   //         bool held;
   //         bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
   //         //RenderFrame(bb.Min, bb.Max, hovered ? hoveredColor : (itemSelected ? activeColor : backgroundColor), false);

   //         if (size.y >= height)
   //         {
   //             ImRect textRect{ { bb.Min.x + 8.0f, bb.Min.y + PaddingY() }, { bb.Max.x, bb.Max.y - PaddingY() } };
   //             ImVec4 textClipRect{ textRect.Min.x, textRect.Min.y, textRect.Max.x, textRect.Max.y };
			//	RenderTextClipped(textRect.Min, textRect.Max, itemText, textEnd, nullptr, {}, &textRect);
   //         }

   //         if (hovered && labelSize.x > width)
			//{
   //             if (BeginTooltip())
   //             {
			//		ImGui::TextEx(itemText, textEnd);
			//		EndTooltip();
   //             }
			//}

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
    WIDGET_SET_PROPERTY(TextStartOffset, textStartOffset, float,       8.0f      )
    WIDGET_SET_PROPERTY(Color,           color,           uint32_t,    0xff1f1f1f)
    WIDGET_SET_PROPERTY(BorderColor,     borderColor,     uint32_t,    0xff121212)
    WIDGET_SET_PROPERTY(BorderSize,      borderSize,      float,       1         )
    WIDGET_SET_PROPERTY(Rounding,        rounding,        float,       2         )
	WIDGET_SET_PROPERTY(MaxVisibleItem,  maxVisibleItem,  uint32_t,    0xffffffff)
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
			window->DrawList->AddText(bbIcon.Min, (hovered || Opened()) ? textColor : 0xffaaaaaa, kKeyboardArrowDown);
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

			if (io.MouseWheel > 1.0f)
			{
				selected = std::max(0, selected - 1);
				selectedChange = true;
			}
			else if (io.MouseWheel < -1.0f)
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
			height = ImGui::GetFrameHeight();
			//height += PaddingY() * 4;
            
			ImVec2 popupSize = {width, height * std::min(maxVisibleItem, uint32_t(data.size()))};
            if (popupSize.y != tween.peek(1.0f))
            {
				tween = tweeny::from(0.0f).to(popupSize.y).during(500).via(tweeny::easing::quadraticOut);
            }

			popupSize.y = tween.step(5.0f * Time::DeltaTime);
			SetNextWindowSize(popupSize);
			SetNextWindowPos({bb.Min.x, bb.Max.y});
        }

        {
			StyleVarStack<float> styleVar{
                { ImGuiStyleVar_WindowRounding,  4.0f },
			    { ImGuiStyleVar_PopupRounding,   4.0f },
			    { ImGuiStyleVar_PopupBorderSize, 0.0f },
			    { ImGuiStyleVar_FrameRounding,   4.0f }
			};

            bool hasScrollBar = maxVisibleItem < data.size();
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
            if (!hasScrollBar)
            {
				windowFlags |= ImGuiWindowFlags_NoScrollbar;
            }
            
			if (Begin(name, nullptr, windowFlags))
			{
				Indent(4.0f);
				StyleVarStack<ImVec2> styleVar = {
				    {ImGuiStyleVar_ItemSpacing, {0, 0}},
                };
				StyleColorStack<uint32_t> styleColor{
				    {ImGuiCol_ScrollbarBg, 0x0},
				    {ImGuiCol_ScrollbarGrab, 0x88444444},
				    {ImGuiCol_ScrollbarGrabHovered, 0xdd444444},
				    {ImGuiCol_ScrollbarGrabActive, 0xdd444444},
				    {ImGuiCol_HeaderHovered, 0x33ff8844}};

                float itemWidth = width - 8.0f;
                if (hasScrollBar)
				{
					ImGuiStyle &style = GetStyle();
					itemWidth -= style.ScrollbarSize;
                }
				selectedChange |= seletableUI.Draw(popupId, Opened(), bb, {bb.Min.x, bb.Max.y}, itemWidth, height, data, selected, hoveredColor);
				Unindent();
			}

			ImGui::End();
		}
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

class WTextCheckBox
{
public:
	using WidgetType = WTextCheckBox;
	WIDGET_SET_PROPERTY(Active,                active,                bool,     false     )
	WIDGET_SET_PROPERTY(BackgroundColor,       backgroundColor,       uint32_t, 0x860d0d0d)
	WIDGET_SET_PROPERTY(ActiveBackgroundColor, activeBackgroundColor, uint32_t, 0xff1f1f1f)

public:
	virtual bool Draw(const String &text, const ImVec2 &sizeArgs, WAlignMode alignMode = WAlignMode::HCenter, float indent = 0.0f)
	{
		using namespace ImGui;
		ImGuiContext &g = *GImGui;
		const ImGuiStyle &style = g.Style;
		ImGuiWindow *window = GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		auto [s, e] = text.GetTuple();
		ImVec2 textSize = CalcTextSize(s, e);
		ImVec2 size = textSize + style.ItemSpacing;
		size = CalcItemSize(sizeArgs, size.x, size.y);

		auto id = window->GetID(this);
		ImRect bb = GetBoundingBox(window->DC.CursorPos, size);

		ItemSize(size, 0);
		if (!ItemAdd(bb, id))
		{
			return false;
		}

		bool hovered;
		bool held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, 0);
		if (pressed)
		{
			Active(!Active());
		}

		RenderFrame(bb.Min, bb.Max, GetColorU32((hovered && !Active()) ? ImGuiCol_HeaderHovered : (!Active() ? ImGuiCol_Header : ImGuiCol_HeaderActive)), false, style.FrameRounding);

        ImVec2 textPos = {};
        if (alignMode == WAlignMode::HCenter)
		{
			textPos = GetCenterAlignPosition(size, textSize);
        }
        else if (alignMode == WAlignMode::Right)
        {
			textPos.x = GetRightAlignPosition(size.x, textSize.x);
			textPos.y = GetCenterAlignPosition(size.y, textSize.y);
        }
        else
        {
			textPos.x = 4.0f;
			textPos.y = GetCenterAlignPosition(size.y, textSize.y);
        }

        textPos.x += indent;
		textPos += bb.Min;
		RenderText(textPos, s, e);

		return pressed;
	}
};

class WCircleButton
{
public:
	using WidgetType = WCircleButton;
	WIDGET_PROPERTY_TEXT
	WIDGET_SET_PROPERTY(Active, active, bool, false)

public:
	bool Draw(const ImVec2 &sizeArgs)
	{
		using namespace ImGui;
		ImGuiContext &g = *GImGui;
		const ImGuiStyle &style = g.Style;
		ImGuiWindow *window = GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		auto [s, e] = text.GetTuple();
		ImVec2 textSize = CalcTextSize(s, e);
		ImVec2 size = textSize + style.ItemSpacing;
		size = CalcItemSize(sizeArgs, size.x, size.y);

		auto id = window->GetID(this);
		ImRect bb = GetBoundingBox(window->DC.CursorPos, size);

		ItemSize(size, 0);
		if (!ItemAdd(bb, id))
		{
			return false;
		}

		bool hovered;
		bool held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		float radius = size.y * 0.5f;
		auto center = bb.GetCenter();
		window->DrawList->AddCircle(center, radius, 0xffffffff, 100, 1.2f);

		if (hovered)
		{
			ImVec2 mouse_pos = ImGui::GetMousePos();
			float distance = std::sqrt(std::pow(mouse_pos.x - center.x, 2) + std::pow(mouse_pos.y - center.y, 2));
			hovered = distance <= radius;
			pressed = hovered && pressed;
		}

		int textColor = hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Text;
		if (pressed)
		{
			textColor = ImGuiCol_Text;
			Active(!Active());
		}

		ImVec2 textPos = GetCenterAlignPosition(size, textSize);
		window->DrawList->AddText(bb.Min + textPos, GetColorU32(textColor), s, e);

		return pressed;
	}
};

class WTextRectangle
{
public:
	bool Draw(const String &text, const ImVec2 &sizeArgs, uint32_t backgroundColor, float rounding = 0.0f, WAlignMode alignMode = WAlignMode::Left, float padding = 4.0f)
	{
		using namespace ImGui;
		ImGuiContext &g = *GImGui;
		const ImGuiStyle &style = g.Style;
		ImGuiWindow *window = GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		auto [s, e] = text.GetTuple();
		ImVec2 textSize = CalcTextSize(s, e);
		ImVec2 size = textSize + style.ItemSpacing;
		size = CalcItemSize(sizeArgs, size.x, size.y);

		auto id = window->GetID(this);
		ImRect bb = GetBoundingBox(window->DC.CursorPos, size);

		ItemSize(size, 0);
		if (!ItemAdd(bb, id))
		{
			return false;
		}

		RenderFrame(bb.Min, bb.Max, backgroundColor, false, rounding);

		ImVec2 pos = bb.Min;
		if (alignMode & WAlignMode::Right)
		{
			pos += GetRightAlignPosition(size, textSize);
			pos.x -= padding;
		}
		else if (alignMode & WAlignMode::HCenter)
		{
			pos += GetCenterAlignPosition(size, textSize);
		}
		else
		{
			pos.y += GetCenterAlignPosition(size.y, textSize.y);
			pos.x += padding;
		}

		RenderText(pos, s, e);

        return true;
	}
};

class WRoundedButton
{
public:
	using WidgetType = WRoundedButton;
	WIDGET_PROPERTY_TEXT
	WIDGET_SET_PROPERTY(Active, active, bool, false)

public:
	bool Draw(const ImVec2 &sizeArgs, float rounding, uint32_t backgroundColor, uint32_t activeBackgroundColor, float borderSize = 0.0f, uint32_t borderColor = 0x0)
	{
		using namespace ImGui;
		ImGuiContext &g = *GImGui;
		const ImGuiStyle &style = g.Style;
		ImGuiWindow *window = GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		auto [s, e] = text.GetTuple();
		ImVec2 textSize = CalcTextSize(s, e);
		ImVec2 size = textSize + style.ItemSpacing;
		size = CalcItemSize(sizeArgs, size.x, size.y);

		auto id = window->GetID(this);
		ImRect bb = GetBoundingBox(window->DC.CursorPos, size);

		ItemSize(size, 0);
		if (!ItemAdd(bb, id))
		{
			return false;
		}

		bool hovered;
		bool held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);
		if (pressed)
		{
			Active(!Active());
		}

		window->DrawList->AddRectFilled(bb.Min, bb.Max, hovered ? activeBackgroundColor : backgroundColor, rounding, 0, g.Style.FrameShadowSize, GetColorU32(ImGuiCol_FrameShadowStart), GetColorU32(ImGuiCol_FrameShadowEnd));
		if (borderSize > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, borderColor, rounding, 0, borderSize);
		}

		ImVec2 textPos = GetCenterAlignPosition(size, textSize);
		//window->DrawList->AddText(bb.Min + textPos, GetColorU32(ImGuiCol_Text), s, e);
		RenderTextClipped(bb.Min, bb.Max, s, e, &textSize, {0.5, 0.5}, &bb);

		return pressed;
	}
};

class WRoundedImageButton
{
public:
	using WidgetType = WRoundedImageButton;
	WIDGET_PROPERTY_TEXT
	WIDGET_SET_PROPERTY(Active, active, bool, false)

public:
	bool Draw(Texture *image, const ImVec2 &sizeArgs, float rounding, uint32_t backgroundColor, uint32_t activeBackgroundColor, float borderSize = 0.0f, uint32_t borderColor = 0x0, const ImVec2 &uv0 = {0, 0}, const ImVec2 &uv1 = {1, 1})
	{
		using namespace ImGui;
		ImGuiContext &g = *GImGui;
		const ImGuiStyle &style = g.Style;
		ImGuiWindow *window = GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		auto [s, e] = text.GetTuple();
		ImVec2 textSize = CalcTextSize(s, e);
		ImVec2 size = textSize + style.ItemSpacing;
		size = CalcItemSize(sizeArgs, size.x, size.y);

		auto id = window->GetID(this);
		ImRect bb = GetBoundingBox(window->DC.CursorPos, size);

		ItemSize(size, 0);
		if (!ItemAdd(bb, id))
		{
			return false;
		}

		bool hovered;
		bool held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);
		if (pressed)
		{
			Active(!Active());
		}

		// window->DrawList->AddRectFilled(bb.Min, bb.Max, hovered ? activeBackgroundColor : backgroundColor, rounding, 0, g.Style.FrameShadowSize, GetColorU32(ImGuiCol_FrameShadowStart), GetColorU32(ImGuiCol_FrameShadowEnd));
		window->DrawList->AddImageRounded(WIMAGE(image), bb.Min, bb.Max, uv0, uv1, hovered ? activeBackgroundColor : 0xffffffff, rounding);
		if (borderSize > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, borderColor, rounding, 0, borderSize);
		}

		ImVec2 textPos = GetCenterAlignPosition(size, textSize);

        RenderTextClipped(bb.Min, bb.Max, s, e, &textSize, {0.5, 0.5}, &bb);

		return pressed;
	}
};

class WTextClipped : public Widget
{
public:
	using WidgetType = WTextClipped;

public:
	bool Draw(const String &text, const ImVec2 &sizeArgs, WAlignMode alignMode = WAlignMode::HCenter, float indent = 0.0f)
	{
		using namespace ImGui;
		ImGuiContext &g = *GImGui;
		const ImGuiStyle &style = g.Style;
		ImGuiWindow *window = GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		auto [s, e] = text.GetTuple();
		ImVec2 textSize = CalcTextSize(s, e);
		ImVec2 size = textSize + style.ItemSpacing;
		size = CalcItemSize(sizeArgs, size.x, size.y);

		auto id = window->GetID(this);
		ImRect bb = GetBoundingBox(window->DC.CursorPos, size);

		ItemSize(size, 0);
		if (!ItemAdd(bb, id))
		{
			return false;
		}

		ImVec2 textPos{};
		if (alignMode == WAlignMode::HCenter)
		{
			textPos = GetCenterAlignPosition(size, textSize);
		}
		else if (alignMode == WAlignMode::Right)
		{
			textPos.x = GetRightAlignPosition(size.x, textSize.x);
		}
		textPos.y = GetCenterAlignPosition(size.y, textSize.y);

		textPos.x += indent;
		RenderTextClipped(bb.Min + textPos, bb.Max, s, e, nullptr);

		return true;
	}
};

}
