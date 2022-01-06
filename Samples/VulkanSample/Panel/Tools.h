#pragma once

#include "Immortal.h"

namespace Immortal
{

class Tools : public Layer
{
public:
    void OnUpdate(Object &o)
    {
        SpriteRendererComponent sprite;
        if (o && o.Has<SpriteRendererComponent>())
        {
            sprite = o.Get<SpriteRendererComponent>();
        }


        ImGui::Begin(WordsMap::Get("Tools Bar"));

        {
            ImVec2 size = { 32.0f, 18.0f };
            int framePadding = 0;
            ImVec2 uv0 = { 0.0f, 0.0f };
            ImVec2 uv1 = { 0.0f, 0.0f };
            ImVec4 backgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };
            ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

            for (int i = 0; i < 8; i++)
            {
                ImGui::PushID(i);
                if (ImGui::ImageButton((ImTextureID)(uint64_t)*Render::Preset()->WhiteTexture, size, uv0, uv1, framePadding, backgroundColor, tintColor))
                {

                }
                ImGui::PopID();
                ImGui::SameLine(0.0f, 1.0f);
            }

            ImGui::SameLine(0.0f, 24.0f);
            size.x *= 2.4f;
            ImGui::PushFont(GuiLayer::NotoSans.Bold);
            if (ImGui::Button(WordsMap::Get("Local").c_str(), size))
            {

            }
            ImGui::SameLine(0.0f, 1.0);
            if (ImGui::Button(WordsMap::Get("World").c_str(), size))
            {

            }
            ImGui::PopFont();
        }

        ImGui::End();
    }
};

}
