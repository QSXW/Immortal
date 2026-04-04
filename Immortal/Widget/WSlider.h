/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Widget.h"

namespace Immortal
{

enum class WSliderType
{
    RangeEditor,
};

class IMMORTAL_API WSlider : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WSlider)
    WIDGET_PROPERTY_COLOR
	WIDGET_SET_PROPERTY(Type,             type,             WSliderType)
	WIDGET_SET_PROPERTY(BackgroundColor,  backgroundColor,  uint32_t, 0xffffffcc)
	WIDGET_SET_PROPERTY(GrabColor,        grabColor,        uint32_t, 0xffffffcc)
	WIDGET_SET_PROPERTY(GrabHoveredColor, grabHoveredColor, uint32_t, 0xffffffcc)
    WIDGET_SET_PROPERTY(Rounding,         rounding,         float               )
    WIDGET_SET_PROPERTY(Progress,         progress,         float               )
    WIDGET_SET_PROPERTY(Radius,           radius,           float,    10.0f     )
    WIDGET_SET_PROPERTY(Min,              min,              float,    0.0f      )
    WIDGET_SET_PROPERTY(Max,              max,              float,    1.0f      )
    WIDGET_SET_PROPERTY(Callback,         callback,         std::function<void(float progress)>)
	WIDGET_SET_PROPERTY(HoveredCallback,  hoveredCallback,  std::function<void(float progress, std::string &tooltip)>)

public:
    WSlider(Widget *v = nullptr) :
        Widget{ v }
    {
        bbGrab = ImRect({ radius, radius }, { radius, radius });
    }

    virtual bool Draw() override
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            return false;
        }

        WidgetLock lock(this);
        __PreCalculateSize();

        StyleVarStack<float> styleVar{
			{ ImGuiStyleVar_GrabMinSize, 1.0f },
        };

        StyleVarStack<ImVec2> paddingVar{
		    {ImGuiStyleVar_ItemSpacing, {padding.right, padding.bottom}}
        };

		MOVEPOS(padding.left, padding.top);

        ImGuiContext &g = *GImGui;
        const ImGuiStyle &style = g.Style;
        const ImGuiID frameId = window->GetID(this);

        const ImRect bbFrame(window->DC.CursorPos, window->DC.CursorPos + ImVec2(RenderWidth(), radius * 2));
        const ImRect bbTotal(bbFrame.Min, bbFrame.Max);

        const bool temp_input_allowed = (0 & ImGuiSliderFlags_NoInput) == 0;
        ImGui::ItemSize(bbTotal, 0);
        if (!ImGui::ItemAdd(bbTotal, frameId, &bbFrame, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
        {
            return false;
        }

        auto grabId = window->GetID(&bbGrab);
        //ImGui::ItemSize(bbGrab);
        if (!ImGui::ItemAdd(bbGrab, grabId))
        {

        }

        bool hovered = false;
		if (ImGui::ItemHoverable(bbGrab, grabId, ImGuiItemFlags_None))
        {
            hovered = true;
            id = grabId;
        }
		if (!hovered && ImGui::ItemHoverable(bbFrame, frameId, ImGuiItemFlags_None))
        {
            hovered = true;
            id = frameId;
        }

        const bool clicked = hovered && ImGui::IsMouseClicked(0, 0, id);
        const bool isActive = clicked || g.NavActivateId == id;
        if (isActive && clicked)
        {
            ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        }

        if (isActive && id != 0)
        {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }

        const ImU32 frameColor = ImGui::GetColorU32(g.ActiveId == frameId ? grabHoveredColor : hovered ? grabHoveredColor : backgroundColor);

        ImVec2 frameCenter = bbFrame.GetCenter();
		float rectHeight = radius / 4.0f;

        ImRect bbRect = {{ bbFrame.Min.x,  frameCenter.y - rectHeight}, { bbFrame.Max.x, frameCenter.y + rectHeight }};
		//ImGui::RenderNavHighlight(bbRect, frameId);
		//ImGui::RenderFrame(bbRect.Min, bbRect.Max, frameColor, true, Rounding());
        //if (g.ActiveId == frameId)
        {
			window->DrawList->AddRectFilled(bbRect.Min, bbRect.Max, frameColor, Rounding());
        }

        ImRect outGrab;
		if (ImGui::SliderBehavior(bbFrame, id, ImGuiDataType_Float, &progress, &min, &max, "", ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_NoInput, &outGrab))
        {
            if (std::abs(outGrab.GetCenter().x - bbGrab.GetCenter().x) > 1.0f)
            {
				if (callback)
				{
					callback(progress);
				}
            }
            ImGui::MarkItemEdited(id);
        }

		auto center = outGrab.GetCenter();
		center.x = IM_ROUND(center.x);
		center.y = IM_ROUND(center.y);
        if (outGrab.Max.x > outGrab.Min.x)
        {
            if (type == WSliderType::RangeEditor)
            {
				ImVec2 bbMin = {outGrab.Min.x - 2.0f, bbFrame.Min.y};
				ImVec2 bbMax = {outGrab.Min.x + 2.0f, bbFrame.Max.y};
				window->DrawList->AddRectFilled(bbMin, bbMax, grabColor, 0.0f);
            }
            else
			{
				outGrab.Min.x -= 2;
				outGrab.Max.x -= 2;

				window->DrawList->AddCircleFilled(center, radius, ImGui::GetColorU32(g.ActiveId == grabId ? grabHoveredColor : grabColor), 16);
				bbGrab = ImRect({center.x - radius, center.y - radius}, {center.x + radius, center.y + radius});
            }
        }

        if (type != WSliderType::RangeEditor)
		{
			ImRect hightlightRect(bbRect.Min, {center.x, bbRect.Max.y});
			DrawRect(window, hightlightRect, Color());
		}

        return true;
    }

    void DrawRect(ImGuiWindow *window, const ImRect &bb, const uint32_t &color)
    {
        window->DrawList->AddRectFilled(bb.Min, bb.Max, color, Rounding());
    }

protected:
    ImRect bbGrab;
    ImGuiID id;
};

}
