/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Widget.h"

namespace Immortal
{

static inline ImVec2 CalculateFixedImageSize(float width, float height, float imageWidth, float imageHeight, float &xhalf, float &yhalf)
{
	float x = width;
	float y = height;

	xhalf = 0;
	yhalf = 0;
	if (imageWidth > imageHeight)
	{
		float scale = imageHeight / imageWidth;
		y = x * scale;
		yhalf = (height - y) * 0.5f;
	}
	else
	{
		float scale = imageWidth / imageHeight;
		x = y * scale;
		xhalf = (width - x) * 0.5f;
	}

    return { x, y };
}

class IMMORTAL_API WImageButton : public Widget
{
public:
    enum Status
    {
        Disabled,
        Active,
    };

public:
   WIDGET_SET_PROPERTIES(WImageButton)
   WIDGET_PROPERTY_COLOR
   WIDGET_PROPERTY_VAR_COLOR(HoveredColor, hoveredColor)
   WIDGET_PROPERTY_VAR_COLOR(ActiveColor, activeColor)
   WIDGET_SET_PROPERTY_FUNC(Callback, callback, const std::function<void(WImageButton::Status status)> &)

public:
    WImageButton(Widget* v = nullptr) :
        Widget{ v }
    {
        Connect([&]() {
            WidgetLock lock{ this };

            ImGuiWindow* window = ImGui::GetCurrentWindow();
            window->DC.CursorPos = window->DC.CursorPos + ImVec2{ padding.left, padding.top };
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ padding.right, padding.bottom });
            ImGui::PushStyleColor(ColorStyle::Button,        color);
            ImGui::PushStyleColor(ColorStyle::ButtonHovered, hoveredColor);
            ImGui::PushStyleColor(ColorStyle::ButtonActive,  activeColor);

            auto resource = imageResources[status];
            if (ImGui::ImageButton(
                WIMAGE(resource.image),
                { renderWidth, renderHeight },
                resource.uv._0,
                resource.uv._1,
                0
            ))
            {
                if (callback)
                {
                    callback((Status)status);
                }
            }

            if (ImGui::IsItemClicked())
            {
                Toggle();
            }
            
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();
            
            __RelativeTrampoline();

            });
    }

    void Toggle()
    {
        status = status ? 0 : 1;
    }

    template <Status status>
    WImageButton *Resource(const WImageResource &resource)
    {
        imageResources[status] = resource;
        return this;
    }
    
    Status GetStatus() const
    {
        return (Status)status;
    }

protected:
    WImageResource imageResources[2];

    std::function<void(WImageButton::Status status)> callback;

    int status = 0;
};

}
