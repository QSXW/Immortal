#pragma once

#include "Immortal.h"

namespace Immortal
{

class Navigator : public Layer
{
public:
    template <class Callback>
    void OnUpdate(Object &o, Callback callback)
    {
        auto &sprite = o.Get<SpriteRendererComponent>();
        
        ImGui::Begin(WordsMap::Get("navigator"));
        
        auto &[x, y] = ImGui::GetContentRegionAvail();
        
        ImVec2 size{};
        size.x = x - 8;
        size.y = size.x * sprite.Texture->Height() / sprite.Texture->Width();

        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4{    1.0f,    1.0f,    1.0f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.4509f, 0.7882f, 0.8980f, 1.0f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 20.0f, 20.0f });
        if (ImGui::ImageButton((ImTextureID)(uint64_t)*sprite.Texture, size))
        {
            callback();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        if (ImGui::CollapsingHeader(WordsMap::Get("preset")))
        {

        }

        ImGui::End();
    }
};

}
