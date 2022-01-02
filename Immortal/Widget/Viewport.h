#pragma once

#include "Widget.h"

#include "ImGui/GuiLayer.h"
#include "Render/Texture.h"

namespace Immortal
{

namespace Widget
{

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

    template<class T, class P>
    void OnUpdate(std::shared_ptr<T> &target, P process = []() -> void{ })
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin(Super::Text.c_str(), NULL, ImGuiWindowFlags_NoTitleBar);

        auto &[x, y] = ImGui::GetContentRegionAvail();

        Set({ x, y });

        ImGui::Image(
            (ImTextureID)(uint64_t)(*target),
            { x, y }
            );

        process();

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
}
