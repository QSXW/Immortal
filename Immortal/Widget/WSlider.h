/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Widget.h"

namespace Immortal
{

class IMMORTAL_API WSlider : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WSlider)
    WIDGET_PROPERTY_COLOR
    WIDGET_PROPERTY_BACKGROUND_COLOR
    WIDGET_PROPERTY_VAR_COLOR(GrabColor, grabColor)
    WIDGET_PROPERTY_VAR_COLOR(GrabHoveredColor, grabHoveredColor)
    WIDGET_SET_PROPERTY(Rounding, rounding, float)
    WIDGET_SET_PROPERTY(Progress, progress, float)
    WIDGET_SET_PROPERTY(Radius,   radius,   float)
    WIDGET_SET_PROPERTY(Min,      min,      float)
    WIDGET_SET_PROPERTY(Max,      max,      float)
    WIDGET_SET_PROPERTY(Callback, callback, std::function<void(float progress)>)

public:
    WSlider(Widget *v = nullptr) :
        Widget{ v },
        radius{ 10 }
    {
        this->BackgroundColor(0xffffffdd)
            ->Color(0x007bffff)
            ->GrabColor(0xffffffdd)
            ->Min(0.0f)
            ->Max(1.0f);

        bbGrab = ImRect({ radius, radius }, { radius, radius });
        Connect([=, this]() -> void {
            WidgetLock lock(this);
            EXPORT_WINDOW
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ padding.right, padding.bottom });
            MOVEPOS(padding.left, padding.right);
            Draw({ renderWidth, renderHeight });

            ImGui::PopStyleVar();
        });
    }

    void Draw(const ImVec2 &size)
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            return;
        }

        ImGuiContext &g = *GImGui;
        const ImGuiStyle &style = g.Style;
        const ImGuiID frameId = window->GetID(this);

        const ImRect bbFrame(window->DC.CursorPos, window->DC.CursorPos + ImVec2(renderWidth, renderHeight));
        const ImRect bbTotal(bbFrame.Min, bbFrame.Max);

        const bool temp_input_allowed = (0 & ImGuiSliderFlags_NoInput) == 0;
        ImGui::ItemSize(bbTotal, style.FramePadding.y);
        if (!ImGui::ItemAdd(bbTotal, frameId, &bbFrame, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
        {
            return;
        }

        auto grabId = window->GetID(&bbGrab);
        ImGui::ItemSize(bbGrab);
        if (!ImGui::ItemAdd(bbGrab, grabId))
        {

        }

        bool hovered = false;
        if (ImGui::ItemHoverable(bbGrab, grabId))
        {
            hovered = true;
            id = grabId;
        }
        if (!hovered && ImGui::ItemHoverable(bbFrame, frameId))
        {
            hovered = true;
            id = frameId;
        }

        const bool clicked = hovered && ImGui::IsMouseClicked(0, id);
        const bool isActive = clicked || g.NavActivateId == id || g.NavActivateInputId == id;
        if (isActive && clicked)
        {
            ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        }

        if (isActive)
        {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }

        const ImU32 frameColor = ImGui::GetColorU32(g.ActiveId == frameId ? grabHoveredColor : hovered ? grabHoveredColor : backgroundColor);
        ImGui::RenderNavHighlight(bbFrame, frameId);
        ImGui::RenderFrame(bbFrame.Min, bbFrame.Max, frameColor, true, g.Style.FrameRounding);

        ImRect outGrab;
        if (ImGui::SliderBehavior(bbFrame, id, ImGuiDataType_Float, &progress, &min, &max, "", ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_NoInput, &outGrab))
        {
            if (std::abs(outGrab.GetCenter().x - bbGrab.GetCenter().x) > 1.0f)
            {
                callback(progress);
            }
            ImGui::MarkItemEdited(id);
        }

        if (outGrab.Max.x > outGrab.Min.x)
        {
            auto center = outGrab.GetCenter();
            center.x = IM_ROUND(center.x); 
            center.y = IM_ROUND(center.y);
            window->DrawList->AddCircleFilled(center, radius, ImGui::GetColorU32(g.ActiveId == grabId ? grabHoveredColor : grabColor), 16);
            bbGrab = ImRect({ center.x - radius, center.y - radius }, { center.x + radius, center.y + radius });
        }

        ImRect hightlightRect(bbFrame.Min, { outGrab.Min.x, bbFrame.Max.y });
        DrawRect(window, hightlightRect, color);
    }

    void DrawRect(ImGuiWindow *window, const ImRect &bb, const ImVec4 &color)
    {
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, 0))
        {
            return;
        }

        window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(color), rounding);
    }

protected:
    ImRect bbGrab;
    ImGuiID id;
};

}
