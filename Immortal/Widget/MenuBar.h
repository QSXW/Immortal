#pragma once

#include "Widget.h"

#include "ImGui/GuiLayer.h"

namespace Immortal
{

class MenuBar : public Widget
{
public:
    MenuBar()
    {
    
    }

    MenuBar(const std::string &name)
    {

    }

    template<class T>
    void OnUpdate(T func)
    {
        ImGui::PushStyleColor(ImGuiCol_Text,    ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        ImGui::BeginMenuBar();
        func();
        ImGui::EndMenuBar();
        ImGui::PopStyleColor(2);
    }
};

}
