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
        
        ImGui::PushFont(GuiLayer::NotoSans.Bold);
        ImGui::Begin(WordsMap::Get("navigator"));
        
        {
            auto &[x, y] = ImGui::GetContentRegionAvail();

            ImVec2 size{};
            size.x = x - 8;
            size.y = size.x * sprite.Texture->Height() / sprite.Texture->Width();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 1.0f,    1.0f,    1.0f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.4509f, 0.7882f, 0.8980f, 1.0f });
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 20.0f, 20.0f });
            if (ImGui::ImageButton((ImTextureID)(uint64_t)*sprite.Texture, size))
            {
                callback();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);

            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{ 1.0f, 1.0f, 1.0f, 0.0f });
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{ 1.0f, 1.0f, 1.0f, 0.2f });
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4{ 1.0f, 1.0f, 1.0f, 0.2f });
            if (ImGui::CollapsingHeader(WordsMap::Get("meta")))
            {

            }
            if (ImGui::CollapsingHeader(WordsMap::Get("preset")))
            {

            }
            ImGui::Separator();
            if (ImGui::CollapsingHeader(WordsMap::Get("snapshot")))
            {

            }
            ImGui::Separator();
            if (ImGui::CollapsingHeader(WordsMap::Get("history")))
            {

            }
            ImGui::Separator();
            if (ImGui::CollapsingHeader(WordsMap::Get("favorite")))
            {

            }
            ImGui::Separator();

            ImGui::PopStyleColor(3);
        }

        ImGui::End();
        ImGui::PopFont();
    }
};

}
