/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Widget.h"

namespace Immortal
{

class IMMORTAL_API WImageButton : public Widget
{
public:
   WIDGET_SET_PROPERTIES(WImageButton)
   WIDGET_PROPERTY_COLOR
   WIDGET_PROPERTY_VAR_COLOR(HoveredColor, hoveredColor, ImVec4)
   WIDGET_PROPERTY_VAR_COLOR(ActiveColor, activeColor, ImVec4)

public:
    enum Status
    {
        Disabled,
        Active,
    };

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
            ImGui::ImageButton(
                (ImTextureID)(uint64_t)*resource.image,
                { renderWidth, renderHeight },
                resource.uv._0,
                resource.uv._1,
                0
            );

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

    int status = 0;
};

}
