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

    static inline bool DrawFloat(const std::string &label, float *values, float speed = 0.001f, float min = -1.0f, float max = 1.0f)
    {
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
                            UI::DrawVec3Control(WordsMap::Get("Rotation"), rotation, 1.0f);
                            UI::DrawVec3Control(WordsMap::Get("Scale"), transform.Scale, 0.01f);

                            transform.Rotation = Vector::Radians(rotation);
                    });
                }
                
                if (o.Has<MaterialComponent>())
                {
                    DrawComponent(
                        WordsMap::Get("Material"),
                        [&]() -> void {
                            auto &material = o.Get<MaterialComponent>();

                            auto id = WordsMap::Get("Color").c_str();
                            ImGui::PushID(id);
                            ImGui::Columns(2);
                            ImGui::Text(id);
                            ImGui::SetColumnWidth(0, 64);
                            ImGui::NextColumn();
                            ImGui::PushItemWidth(-1);
                            ImGui::SetColorEditOptions(ImGuiColorEditFlags_DefaultOptions_);
                            ImGui::ColorEdit4("##C", (float *)&material.AlbedoColor);
                            ImGui::NextColumn();
                            ImGui::PopID();

                            float smoothness = 1.0f - material.Roughness;
                            DrawFloat(WordsMap::Get("Metallic"),   &material.Metallic, 0.001, 0, 1.0f);
                            DrawFloat(WordsMap::Get("Smoothness"), &smoothness,        0.001, 0, 1.0f);
                            material.Roughness = 1.0f - smoothness;

                            ImGui::Columns(1);
                            auto &textures = material.Textures;
                            ImVec2 size = { 64.0f, 64.0f };
                            if (ImGui::ImageButton((ImTextureID)(uint64_t)*textures.Albedo, size))
                            {
                                auto res = FileDialogs::OpenFile(FileFilter::Image);
                                if (res.has_value())
                                {
                                    textures.Albedo.reset(Render::Create<Texture>(
                                        res.value(),
                                        Texture::Description{
                                            Format::UNDEFINED,
                                            Wrap::Mirror,
                                            Filter::Linear
                                        }));
                                }
                            }
                        });
                }

                if (o.Has<ColorMixingComponent>())
                {
                    DrawComponent(
                        WordsMap::Get("ColorMixing"),
                        [&]() -> void {    
                            auto &c = o.Get<ColorMixingComponent>();
                            c.Modified = false;
                            
                            c.Modified |= DrawFloat(WordsMap::Get("Red").c_str(),   &c.RGBA.r);
                            c.Modified |= DrawFloat(WordsMap::Get("Green").c_str(), &c.RGBA.g);
                            c.Modified |= DrawFloat(WordsMap::Get("Blue").c_str(),  &c.RGBA.b);
                            c.Modified |= DrawFloat(WordsMap::Get("Alpha").c_str(), &c.RGBA.a);

                            auto channels = WordsMap::Get("Channels").c_str();
                            ImGui::PushID(channels);
                            ImGui::Columns(2);
                            ImGui::Text(channels);
                            ImGui::SetColumnWidth(0, 64);
                            ImGui::NextColumn();
                            ImGui::PushItemWidth(-1);
                            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_PickerHueBar);
                            c.Modified |= ImGui::ColorEdit4("##C", (float *)&c.RGBA);
                            ImGui::NextColumn();
                            ImGui::PopID();

                            c.Modified |= DrawFloat(WordsMap::Get("Hue").c_str(),        &c.HSL.x);
                            c.Modified |= DrawFloat(WordsMap::Get("Saturation").c_str(), &c.HSL.y);
                            c.Modified |= DrawFloat(WordsMap::Get("Luminance").c_str(),  &c.HSL.z);

                            auto hsl = WordsMap::Get("HSL").c_str();
                            ImGui::PushID(hsl);
                            ImGui::Columns(2);
                            ImGui::Text(hsl);
                            ImGui::SetColumnWidth(0, 64);
                            ImGui::NextColumn();
                            ImGui::PushItemWidth(-1);
                            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel);
                            c.Modified |= ImGui::ColorEdit4("##C", (float *)&c.HSL, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);

                            ImGui::NextColumn();
                            ImGui::Columns(1);
                            ImGui::PopID();

                            c.Modified |= DrawFloat(WordsMap::Get("White Gradation").c_str(), &c.Gradation.White);
                            c.Modified |= DrawFloat(WordsMap::Get("Black Gradation").c_str(), &c.Gradation.Black);
                            c.Modified |= DrawFloat(WordsMap::Get("Exposure"       ).c_str(), &c.Exposure       );
                            c.Modified |= DrawFloat(WordsMap::Get("Contrast"       ).c_str(), &c.Contrast       );
                            c.Modified |= DrawFloat(WordsMap::Get("Hightlights"    ).c_str(), &c.Hightlights    );
                            c.Modified |= DrawFloat(WordsMap::Get("Shadow"         ).c_str(), &c.Shadow         );
                            c.Modified |= DrawFloat(WordsMap::Get("Vividness"      ).c_str(), &c.Vividness      );
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
