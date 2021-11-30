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

    void Set(const Vector2 &value)
    {
        size = value;
    }

    template<class T>
    void OnUpdate(std::shared_ptr<T> &target)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin(Super::text.c_str(), NULL, ImGuiWindowFlags_NoTitleBar);

        auto &[x, y] = ImGui::GetContentRegionAvail();

        Set({ x, y });

        ImGui::Image(
            (ImTextureID)(target->Descriptor()),
            { x, y }
            );

        ImGui::End();
        ImGui::PopStyleVar();
    }

    Vector2 Size() const
    {
        return size;
    }

private:
    Vector2 size{ 0, 0 };
};

}
