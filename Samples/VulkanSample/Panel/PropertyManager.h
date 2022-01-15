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
    void OnUpdate(Object &o)
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
                            // ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing() - 8.0f);

                            auto &transform = o.Get<TransformComponent>();
                            auto rotation = Vector::Degrees(transform.Rotation);

                            UI::DrawVec3Control(WordsMap::Get("Position"), transform.Position, 0.01f);
                            UI::DrawVec3Control(WordsMap::Get("Rotation"), rotation, 10.0f);
                            UI::DrawVec3Control(WordsMap::Get("Scale"), transform.Scale, 0.01f);

                            transform.Rotation = Vector::Radians(rotation);
                    });
                }
                
                if (o.Has<ColorMixingComponent>())
                {
                    DrawComponent(
                        WordsMap::Get("ColorMixing"),
                        [&]() -> void {    
                            bool modified = false;

                            auto &c = o.Get<ColorMixingComponent>();

                            auto drawFloat = [&](const std::string &label, float *values, float speed = 0.001f, float min = -1.0f, float max = 1.0f) {
                                bool modified = false;

                                ImGui::PushID(label.c_str());

                                ImGui::Columns(2);
                                ImGui::Text(label.c_str());
                                ImGui::SetColumnWidth(0, 64);
                                ImGui::NextColumn();
                                ImGui::PushItemWidth(-1);
                                modified |= ImGui::SliderFloat("##C", values, min, max);

                                ImGui::Columns(1);
                                ImGui::PopID();

                                return modified;
                            };
                            
                            drawFloat(WordsMap::Get("Red").c_str(),   &c.RGBA.r);
                            drawFloat(WordsMap::Get("Green").c_str(), &c.RGBA.g);
                            drawFloat(WordsMap::Get("Blue").c_str(),  &c.RGBA.b);
                            drawFloat(WordsMap::Get("Alpha").c_str(), &c.RGBA.a);

                            auto channels = WordsMap::Get("Channels").c_str();
                            ImGui::PushID(channels);
                            ImGui::Columns(2);
                            ImGui::Text(channels);
                            ImGui::SetColumnWidth(0, 64);
                            ImGui::NextColumn();
                            ImGui::PushItemWidth(-1);
                            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_PickerHueBar);
                            modified |= ImGui::ColorEdit4("##C", (float *)&c.RGBA);
                            ImGui::NextColumn();
                            ImGui::PopID();

                            drawFloat(WordsMap::Get("Hue").c_str(),        &c.HSL.x);
                            drawFloat(WordsMap::Get("Saturation").c_str(), &c.HSL.y);
                            drawFloat(WordsMap::Get("Luminance").c_str(),  &c.HSL.z);

                            auto hsl = WordsMap::Get("HSL").c_str();
                            ImGui::PushID(hsl);
                            ImGui::Columns(2);
                            ImGui::Text(hsl);
                            ImGui::SetColumnWidth(0, 64);
                            ImGui::NextColumn();
                            ImGui::PushItemWidth(-1);
                            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel);
                            modified |= ImGui::ColorEdit4("##C", (float *)&c.HSL, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);

                            ImGui::NextColumn();
                            ImGui::Columns(1);
                            ImGui::PopID();

                            drawFloat(WordsMap::Get("White Gradation").c_str(), &c.Gradation.White);
                            drawFloat(WordsMap::Get("Black Gradation").c_str(), &c.Gradation.Black);
                            drawFloat(WordsMap::Get("Exposure"       ).c_str(), &c.Exposure       );
                            drawFloat(WordsMap::Get("Contrast"       ).c_str(), &c.Contrast       );
                            drawFloat(WordsMap::Get("Hightlights"    ).c_str(), &c.Hightlights    );
                            drawFloat(WordsMap::Get("Shadow"         ).c_str(), &c.Shadow         );
                            drawFloat(WordsMap::Get("Vividness"      ).c_str(), &c.Vividness      );
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
        ImGui::PushID(name.c_str());
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
        ImGui::PopID();
    }
};

}
