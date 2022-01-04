#pragma once

#include "Immortal.h"

namespace Immortal
{

class PropertyManager : public Layer
{
public:
	static constexpr ImGuiTreeNodeFlags TreeNodeFlags =
			ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_Framed |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_AllowItemOverlap |
			ImGuiTreeNodeFlags_FramePadding;

public:
    template <class Callback>
    void OnUpdate(Object &o, Callback callback)
    {
		ImGui::PushFont(GuiLayer::NotoSans.Bold);
		ImGui::Begin(WordsMap::Get("Properties"));
        {
            if (o)
            {
                if (o.Has<TransformComponent>())
                {
                    DrawComponent(
						WordsMap::Get("Transform"),
						[&]() -> void {
							ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing() - 8.0f);

							auto &transform = o.Get<TransformComponent>();
							auto rotation = Vector::Degrees(transform.Rotation);

							UI::DrawVec3Control(WordsMap::Get("Position"), transform.Position);
							UI::DrawVec3Control(WordsMap::Get("Rotation"), rotation);
							UI::DrawVec3Control(WordsMap::Get("Scale"), transform.Scale);

							transform.Rotation = Vector::Radians(rotation);
					});
                }
            }
        }
        ImGui::End();
		ImGui::PopFont();
    }

    template <class F>
    static void DrawComponent(const std::string &name, F process)
    {
		ImGui::AlignTextToFramePadding();
		auto &[x, y] = ImGui::GetContentRegionAvail();

		// ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		bool isOpen = ImGui::TreeNodeEx("##Undefined", TreeNodeFlags, name.c_str());
		bool isRightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

		// ImGui::PopStyleVar();
		ImGui::SameLine(x - lineHeight * 0.5f);

		if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }) || isRightClicked)
		{
			ImGui::OpenPopup("Conf");
		}

		if (ImGui::BeginPopup("Conf"))
		{
			if (ImGui::MenuItem(WordsMap::Get("Remove")))
			{

			}
			ImGui::EndPopup();
		}

		if (isOpen)
		{
			process();
			ImGui::TreePop();
		}
    }
};

}
