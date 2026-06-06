#pragma once

#include "Immortal.h"

namespace Immortal
{

class WTools : public Widget
{
public:
    static constexpr float None      = 0.;
    static constexpr float Activated = 1.;
    static constexpr float Hovered   = 2.;

    enum
    {
        Hand,
        Move,
        Rotate,
        Scale,
        Crop,
        Transform,
        Edit,
        Start = 0,
        Pause = 1
    };

    static inline float xOffset = 0.0;
    static inline float yOffset = 0.0f;

public:
	WTools() :
        texture{ Graphics::CreateTexture("Assets/Icon/Tools300x60.png") }
    {
        xOffset = 30.0f / texture->GetWidth();
        yOffset = 20.0f / texture->GetHeight();

        Connect([&] {
			ImGui::Begin(WordsMap::Get("Tools Bar"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
			ImGuiStyle *style = &ImGui::GetStyle();

			{
				ImGui::SameLine(0.0f, 12.0f);
				ImVec2 size = {30.0f, 20.0f};
				int framePadding = 0;

				ImVec4 backgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
				ImVec4 tintColor = {1.0f, 1.0f, 1.0f, 1.0f};

				for (int i = 0; i < SL_ARRAY_LENGTH(vOffset.tool); i++)
				{
					ImVec2 uv0 = {xOffset * i, yOffset * vOffset.tool[i]};
					ImVec2 uv1 = {uv0.x + xOffset, uv0.y + yOffset};

					ImGui::PushID(i);
					if (ImGui::ImageButton(WIMAGE(texture), size, uv0, uv1, framePadding, backgroundColor, tintColor))
					{
						Activate(i);
					}
					if (ImGui::IsItemHovered() && vOffset.tool[i] != Activated)
					{
						vOffset.tool[i] = Hovered;
					}
					else
					{
						if (vOffset.tool[i] == Hovered)
						{
							vOffset.tool[i] = None;
						}
					}

					ImGui::PopID();
					ImGui::SameLine(0.0f, 1.0f);
				}

				ImGui::PushFont(GuiLayer::NotoSans.Bold);
				{
					ImGui::SameLine(0.0f, 24.0f);
					if (ImGui::Button(WordsMap::Get("###").c_str(), ImVec2{size.x * 2.4f, size.y}))
					{
					}
					ImGui::SameLine(0.0f, 1.0);
					if (ImGui::Button(WordsMap::Get("###").c_str(), ImVec2{size.x * 2.4f, size.y}))
					{
					}
				}
				ImGui::PopFont();

				auto [x, y] = ImGui::GetContentRegionAvail();
				ImGui::SameLine(0.1f, x / 2 - size.x * 3 / 2);
				for (int i = 0; i < SL_ARRAY_LENGTH(vOffset.control); i++)
				{
					ImVec2 uv0 = {xOffset * (i + SL_ARRAY_LENGTH(vOffset.tool)), yOffset * vOffset.control[i]};
					ImVec2 uv1 = {uv0.x + xOffset, uv0.y + yOffset};

					ImGui::PushID(i + SL_ARRAY_LENGTH(vOffset.tool));
					if (ImGui::ImageButton(WIMAGE(texture), size, uv0, uv1, framePadding, backgroundColor, tintColor))
					{
						vOffset.control[i] = vOffset.control[i] == Activated ? None : Activated;
					}
					if (ImGui::IsItemHovered() && vOffset.control[i] != Activated)
					{
						vOffset.control[i] = Hovered;
					}
					else
					{
						if (vOffset.control[i] == Hovered)
						{
							vOffset.control[i] = None;
						}
					}

					ImGui::SameLine(0, 1.0f);
					ImGui::PopID();
				}
			}

			ImGui::End();
            });
    }

    void OnUpdate(Object &other)
    {
		object = other;
    }

    bool IsToolActive(size_t index)
    {
        return vOffset.tool[index] == Activated;
    }

    bool IsControlActive(size_t index)
    {
        return vOffset.control[index] == Activated;
    }
    
    void Activate(size_t index)
    {
        memset(vOffset.tool, None, sizeof(vOffset.tool));
        vOffset.tool[index] = Activated;
    }

private:
    Ref<Texture> texture;

	Object object;

    struct
    {
        float tool[7]    = { None };
        float control[3] = { None };
    } vOffset;
};

}
