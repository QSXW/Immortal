#pragma once

#include "Widget.h"

#include "ImGui/GuiLayer.h"
#include "Render/Texture.h"

namespace Immortal
{

class Viewport : public Widget
{
public:
    Viewport(const std::string &name)
    {

    }

    void Set(const Vector2 &value)
    {
        size = value;
    }

    template<class T, class P>
    void OnUpdate(T *target, P process = []() -> void{ })
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin(props.text.c_str(), NULL, ImGuiWindowFlags_NoTitleBar);
        ImGui::BeginChild("###");

        auto [x, y] = ImGui::GetContentRegionAvail();

        Set({ x, y });

        ImVec2 offset    = ImGui::GetWindowPos();
        ImVec2 minRegion = ImGui::GetWindowContentRegionMin();
        ImVec2 maxRegion = ImGui::GetWindowContentRegionMax();

        bounds.min = { minRegion.x + offset.x, minRegion.y + offset.y };
        bounds.max = { maxRegion.x + offset.x, maxRegion.y + offset.y };

        isHovered = ImGui::IsWindowHovered();
        isFocused = ImGui::IsWindowFocused();

        ImGui::Image(
            (ImTextureID)(uint64_t)(*target),
            { x, y }
            );

        process();

        ImGui::EndChild();
        ImGui::End();
        ImGui::PopStyleVar();
    }

    Vector2 Size() const
    {
        return size;
    }

    bool IsHovered()
    {
        return isHovered;
    }

    bool IsFocused()
    {
        return isFocused;
    }

    Vector2 MinBound()
    {
        return Vector2{ bounds.min.x, bounds.min.y };
    }

private:
    Vector2 size{ 0, 0 };

    struct
    {
        ImVec2 min;
        ImVec2 max;
    } bounds;

    bool isHovered = false;
    bool isFocused = false;
};

}
