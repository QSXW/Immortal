#pragma once

#include "Immortal.h"

namespace Immortal
{

class WNavigator : public Widget
{
public:
    template <class Callback>
    WNavigator(Callback callback = [] {}, Widget *parent = nullptr) :
	    Widget{parent},
	    object{}
    {
		Connect([=, this] {
			UI::BeginDraw();
			Ref<Texture> sprite = Graphics::Preset()->Textures.White;
			if (object && object.HasComponent<SpriteRendererComponent>())
			{
				auto &spriteRendererComponent = object.GetComponent<SpriteRendererComponent>();
				if (spriteRendererComponent.Sprite)
				{
					sprite = spriteRendererComponent.Sprite;
				}
			}

			ImGui::PushFont(GuiLayer::NotoSans.Bold);
			ImGui::Begin(WordsMap::Get("Navigator"));

			{
				if (object)
				{
					std::string &tag = object.GetComponent<TagComponent>().Tag;
					char buf[4096] = {0};
					strcat(buf, tag.c_str());
					if (ImGui::InputText(WordsMap::Get("Object Name").c_str(), buf, SL_ARRAY_LENGTH(buf)))
					{
						tag = std::string{buf};
					}
				}
				ImGui::NewLine();

				auto [x, y] = ImGui::GetContentRegionAvail();

				ImVec2 size{};
				size.x = x - 8;
				size.y = size.x * sprite->GetHeight() / sprite->GetWidth();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.4509f, 0.7882f, 0.8980f, 1.0f});
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{20.0f, 20.0f});
				if (ImGui::ImageButton(WIMAGE(sprite), size))
				{
					if (object)
					{
						callback();
					}
				}
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(2);
				ImGui::NewLine();

				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{1.0f, 1.0f, 1.0f, 0.0f});
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{1.0f, 1.0f, 1.0f, 0.2f});
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4{1.0f, 1.0f, 1.0f, 0.2f});

				if (ImGui::CollapsingHeader(WordsMap::Get("Meta")))
				{
					const auto width = sprite->GetWidth();
					const auto height = sprite->GetHeight();
					UI::DrawColumn(WordsMap::Get("Width"), [&]() -> bool { ImGui::Text("%d", width);  return false ; });
					UI::DrawColumn(WordsMap::Get("Height"), [&]() -> bool { ImGui::Text("%d", height); return false; });
				}
				ImGui::Separator();

				if (ImGui::CollapsingHeader(WordsMap::Get("Preset")))
				{
				}
				ImGui::Separator();

				if (ImGui::CollapsingHeader(WordsMap::Get("Snapshot")))
				{
				}
				ImGui::Separator();

				if (ImGui::CollapsingHeader(WordsMap::Get("History")))
				{
				}
				ImGui::Separator();

				if (ImGui::CollapsingHeader(WordsMap::Get("Favorite")))
				{
				}
				ImGui::Separator();

				ImGui::PopStyleColor(3);
			}

			ImGui::End();
			ImGui::PopFont();
		});
    }

	void OnUpdate(Object other)
	{
		Select(other);
	}

	void Select(Object other)
	{
		object = other;
	}

private:
	Object object;
};

}
