#pragma once

#include "Widget.h"

#include "ImGui/GuiLayer.h"
#include "Render/Texture.h"

namespace Widget
{

using namespace Immortal;

class Viewport : public Super
{
public:
    Viewport(const std::string &name) :
        Super(name)
    {
    
    }

    template<class T>
    void OnUpdate(std::shared_ptr<T> &target)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin(Super::text.c_str(), NULL, ImGuiWindowFlags_NoTitleBar);

        auto &[x, y] = ImGui::GetContentRegionAvail();

        ImGui::Image(
            (ImTextureID)(target->Handle()),
            { x, y }
            );

        ImGui::End();
        ImGui::PopStyleVar();
    }
};

}
