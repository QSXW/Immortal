#pragma once

#include "Immortal.h"

namespace Immortal
{

class Tools : public Layer
{
public:
    enum
    {
        None      = 0,
        Activated = 1,
        Hovered   = 2
    };

    static constexpr float xOffset = 0.14285714f;
    static constexpr float yOffset = 0.33333333f;

public:
    Tools() :
        texture{ Render::Create<Texture>("Assets/Icon/Tools2100x200.png") }
    {
        
    }

    void OnUpdate(Object &o)
    {
        SpriteRendererComponent sprite;
        if (o && o.Has<SpriteRendererComponent>())
        {
            sprite = o.Get<SpriteRendererComponent>();
        }

        ImGui::BeginMenuBar();
        ImGui::Begin(WordsMap::Get("Tools Bar"));
        
        auto &style = ImGui::GetStyle();

        {
            ImGui::SameLine(0.0f, 12.0f);
            ImVec2 size = { 30.0f, 20.0f };
            int framePadding = 0;
            
            ImVec2 uv1 = { 1.0f, 1.0f };
            ImVec4 backgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };
            ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

            for (int i = 0; i < SL_ARRAY_LENGTH(vOffset); i++)
            {
                ImVec2 uv0 = { xOffset * i, yOffset * vOffset[i]};
                ImVec2 uv1 = { uv0.x + xOffset, uv0.y + yOffset };

                ImGui::PushID(i);
                if (ImGui::ImageButton((ImTextureID)(uint64_t)*texture, size, uv0, uv1, framePadding, backgroundColor, tintColor))
                {
                    memset(vOffset, None, sizeof(vOffset));
                    vOffset[i] = Activated;
                }
                if (ImGui::IsItemHovered() && vOffset[i] != Activated)
                {
                    vOffset[i] = Hovered;
                }
                else
                {
                    if (vOffset[i] == Hovered)
                    {
                        vOffset[i] = None;
                    }
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
        ImGui::EndMenuBar();
    }

private:
    std::shared_ptr<Texture> texture;

    float vOffset[7] = { None };
};

}
