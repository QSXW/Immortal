/**
 * Copyright (C) 2021-2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include <string>
#include <functional>
#include <imgui.h>

#include "Core.h"
#include "Interface/IObject.h"

namespace Immortal
{

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

class Widget;
struct WidgetProperty
{
    std::string text = "Untitled";

    float width  = -1;
    float height = -1;
    ImVec4 color;
    ImVec4 backgroundColor;

    ImVec2 padding = { 0, 0 };
    uint64_t descriptor;

    struct {
        const Widget *left;
        const Widget *right;
        const Widget *top;
        const Widget *bottom;
        const Widget *fill = nullptr;
    } anchors;
};

struct WidgetState
{
    bool isHovered;
    bool isFocused;
};

class IMMORTAL_API Widget : public IObject
{
public:
    using Property = WidgetProperty;

public:
    Widget(Widget *parent = nullptr) :
        parent{}
    {
        AddParent(parent);
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

    virtual void SetProperty(const WidgetProperty &other)
    {
        props = other;
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

    std::string &Text()
    {
        return props.text;
    }

#define WIDGET_SET_PROPERTY(U, L, T) \
    Widget &U(T L) \
    { \
        props.L = L; \
        return *this; \
    }

    WIDGET_SET_PROPERTY(Text, text, const std::string &)
    WIDGET_SET_PROPERTY(Color, color, const ImVec4 &)
    WIDGET_SET_PROPERTY(Width, width, float)
    WIDGET_SET_PROPERTY(Height, height, float)
    WIDGET_SET_PROPERTY(Descriptor, descriptor, uint64_t)
    WIDGET_SET_PROPERTY(Padding, padding, const ImVec2 &)

    Widget &Anchors(const Widget *widget)
    {
        props.anchors.fill = widget;
        return *this;
    }

    Widget &Resize(const Vector2 &size)
    {
        props.width = size.x;
        props.height = size.y;
        return *this;
    }

    float Width() const
    {
        return props.width;
    }

    float Height() const
    {
        return props.height;
    }

    Vector2 Size() const
    {
        return Vector2{ props.width, props.height };
    }

    bool IsHovered() const
    {
        return state.isHovered;
    }

    bool IsFocused() const
    {
        return state.isFocused;
    }

    template <class F>
    void Connect(F f)
    {
        render = f;
    }

protected:
    Widget *parent;

    std::vector<Widget*> children;

    WidgetProperty props;

    WidgetState state;

    std::function<void()> render;
};

class IMMORTAL_API WFrame : public Widget
{
public:
    WFrame(Widget *parent = nullptr) :
        Widget{ parent }
    {
        Connect([&]() {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, props.color);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, props.padding);
            if (ImGui::Begin(props.text.c_str()))
            {
                auto [x, y] = ImGui::GetContentRegionAvail();
                Width(x);
                Height(y);

                ImGui::BeginChild("###");
                for (auto &child : children)
                {
                    child->RealRender();
                }
                ImGui::EndChild();
            }
            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            });
    }
};

class IMMORTAL_API WImage : public Widget
{
public:
    WImage(Widget *v = nullptr) :
        Widget{ v }
    {
        Connect([&]() {
            ImGui::PushID((intptr_t)this);

            float x = props.width;
            float y = props.height;

            if (props.anchors.fill == parent || x < 0 || y < 0)
            {
                x = parent->Width();
                y = parent->Height();
                props.width = x;
                props.height = y;
            }

            ImVec2 offset = ImGui::GetWindowPos();
            ImVec2 minRegion = ImGui::GetWindowContentRegionMin();
            ImVec2 maxRegion = ImGui::GetWindowContentRegionMax();

            bounds.min = { minRegion.x + offset.x, minRegion.y + offset.y };
            bounds.max = { maxRegion.x + offset.x, maxRegion.y + offset.y };

            state.isHovered = ImGui::IsWindowHovered();
            state.isFocused = ImGui::IsWindowFocused();

            ImGui::Image(
                (ImTextureID)props.descriptor,
                { x, y }
            );

            for (auto &child : children)
            {
                child->RealRender();
            }

            ImGui::PopID();
            });
    }

    Vector2 MinBound() const
    {
        return bounds.min;
    }

public:
    struct
    {
        Vector2 min;
        Vector2 max;
    } bounds;
};

}
