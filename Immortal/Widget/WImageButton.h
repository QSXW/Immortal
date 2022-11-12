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

protected:
    WImageResource imageResources[2];

    int status = 0;
};

}
