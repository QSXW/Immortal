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

    static inline int uuid = 0;

    static void BeginDraw()
    {
        uuid = 0;
    }

    template <class T>
    static inline bool DrawColumn(const std::string &label, T draw)
    {
        bool modified = false;

        ImGui::PushID(uuid++);
        ImGui::Columns(2);
        ImGui::Text("%s", label.c_str());
        ImGui::SetColumnWidth(0, 64);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        modified = draw();

        ImGui::Columns(1);
        ImGui::PopID();

        return modified;
    }

    static inline bool DrawFloat(const std::string &label, float *values, float speed = 0.001f, float min = -1.0f, float max = 1.0f)
    {
        return DrawColumn(label, [&]() -> bool {
            return ImGui::SliderFloat("##C", values, min, max);
            });
    };

public:
    void OnUpdate(Object &object)
    {
        BeginDraw();

        ImGui::PushFont(GuiLayer::NotoSans.Bold);
        ImGui::Begin(WordsMap::Get("Properties"));
        {
            if (object)
            {
                if (object.HasComponent<TransformComponent>())
                {
                    DrawComponent(
                        WordsMap::Get("Transform"),
                        [&]() -> void {
                            // ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing() - 8.0f);

                            auto &transform = object.GetComponent<TransformComponent>();
                            auto rotation = Vector::Degrees(transform.Rotation);

                            UI::DrawVec3Control(WordsMap::Get("Position"), transform.Position, 0.01f);
                            UI::DrawVec3Control(WordsMap::Get("Rotation"), rotation, 1.0f);
                            UI::DrawVec3Control(WordsMap::Get("Scale"), transform.Scale, 0.01f);

                            transform.Rotation = Vector::Radians(rotation);
                    });
                }

                if (object.HasComponent<LightComponent>())
                {
                    DrawComponent(
                        WordsMap::Get("Light"),
                        [&]() -> void {
                            auto &light = object.GetComponent<LightComponent>();
                            DrawColumn(WordsMap::Get("Enabled"), [&]() -> bool { return ImGui::Checkbox("##C", &light.Enabled); });
                            DrawColumn(WordsMap::Get("Color"), [&]() -> bool { return ImGui::ColorEdit4("##C", (float *)&light.Radiance); });
                        });
                }

                if (object.HasComponent<MaterialComponent>() && object.HasComponent<MeshComponent>())
                {
                    DrawComponent(
                        WordsMap::Get("Material"),
                        [&]() -> void {
                            auto &material = object.GetComponent<MaterialComponent>();
                            auto &mesh     = object.GetComponent<MeshComponent>();
                            auto &nodeList = mesh.Mesh->NodeList();
                            
                            for (auto &node : nodeList)
                            {
                                auto &ref = material.Ref[node.MaterialIndex];
                                ImGui::Text("%s", node.Name.c_str());

                                DrawColumn(WordsMap::Get("Color"), [&]() -> bool { return ImGui::ColorEdit4("###", (float *)&ref.AlbedoColor); });

                                float smoothness = 1.0f - ref.Roughness;
                                DrawFloat(WordsMap::Get("Metallic"), &ref.Metallic, 0.001, 0, 1.0f);
                                DrawFloat(WordsMap::Get("Smoothness"), &smoothness, 0.001, 0, 1.0f);
                                ref.Roughness = 1.0f - smoothness;

                                ImGui::Columns(1);
                                auto &textures = ref.Textures;
                                ImVec2 size = { 64.0f, 64.0f };

                                auto drawTexture = [&](std::shared_ptr<Texture> &texture, const std::string &label) -> void {
                                    ImGui::PushID((void*)(node.MaterialIndex + (uint64_t)label.c_str()));
                                    ImGui::Columns(2);
                                    ImGui::Text("%s", label.c_str());
                                    ImGui::SetColumnWidth(0, 64);
                                    ImGui::NextColumn();
                                    ImGui::PushItemWidth(-1);
                                    if (ImGui::ImageButton((ImTextureID)(uint64_t)*texture, size))
                                    {
                                        auto res = FileDialogs::OpenFile(FileFilter::Image);
                                        if (res.has_value())
                                        {
                                            texture.reset(Render::Create<Texture>(
                                                res.value(),
                                                Texture::Description{
                                                    Format::None,
                                                    Wrap::Mirror,
                                                    Filter::Linear
                                                }));
                                        }
                                    }
                                    ImGui::Columns(1);
                                    ImGui::PopID();
                                };
                                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{ 1.0f, 1.0f, 1.0f, 0.0f });
                                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{ 1.0f, 1.0f, 1.0f, 0.2f });
                                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4{ 1.0f, 1.0f, 1.0f, 0.2f });

                                drawTexture(textures.Albedo,    WordsMap::Get("Albedo"));
                                drawTexture(textures.Normal,    WordsMap::Get("Normal"));
                                drawTexture(textures.Metallic,  WordsMap::Get("Metallic"));
                                drawTexture(textures.Roughness, WordsMap::Get("Roughness"));

                                ImGui::PopStyleColor(3);
                            }
                        });
                }

                if (object.HasComponent<ColorMixingComponent>())
                {
                    DrawComponent(
                        WordsMap::Get("ColorMixing"),
                        [&]() -> void {
                            auto &c = object.GetComponent<ColorMixingComponent>();
                            c.Modified = false;

                            c.Modified |= DrawFloat(WordsMap::Get("Red").c_str(),   &c.RGBA.r);
                            c.Modified |= DrawFloat(WordsMap::Get("Green").c_str(), &c.RGBA.g);
                            c.Modified |= DrawFloat(WordsMap::Get("Blue").c_str(),  &c.RGBA.b);
                            c.Modified |= DrawFloat(WordsMap::Get("Alpha").c_str(), &c.RGBA.a);

                            auto channels = WordsMap::Get("Channels").c_str();
                            ImGui::PushID(channels);
                            ImGui::Columns(2);
                            ImGui::Text("%s", channels);
                            ImGui::SetColumnWidth(0, 64);
                            ImGui::NextColumn();
                            ImGui::PushItemWidth(-1);
                            c.Modified |= ImGui::ColorEdit4("##C", (float *)&c.RGBA);
                            ImGui::NextColumn();
                            ImGui::PopID();

                            c.Modified |= DrawFloat(WordsMap::Get("Hue").c_str(),        &c.HSL.x);
                            c.Modified |= DrawFloat(WordsMap::Get("Saturation").c_str(), &c.HSL.y);
                            c.Modified |= DrawFloat(WordsMap::Get("Luminance").c_str(),  &c.HSL.z);

                            auto hsl = WordsMap::Get("HSL").c_str();
                            ImGui::PushID(hsl);
                            ImGui::Columns(2);
                            ImGui::Text("%s", hsl);
                            ImGui::SetColumnWidth(0, 64);
                            ImGui::NextColumn();
                            ImGui::PushItemWidth(-1);
                            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel);
                            c.Modified |= ImGui::ColorEdit4("##C", (float *)&c.HSL, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);
                            ImGui::SetColorEditOptions(ImGuiColorEditFlags_DefaultOptions_);

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
                if (object.HasComponent<VideoPlayerComponent>())
                {
                    DrawComponent(
                        WordsMap::Get("VideoPlayer"),
                        [&]() -> void {
                            auto &v = object.GetComponent<VideoPlayerComponent>();
                            auto animator = v.GetAnimator();

                            float progress = (float)animator->timestamps.current / animator->frameNumber;
                            DrawColumn(WordsMap::Get("Progress"), [&]() { ImGui::ProgressBar(progress, ImVec2{ 0, 0 }); return false; });
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
        auto [x, y] = ImGui::GetContentRegionAvail();

        // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

        bool isOpen = ImGui::TreeNodeEx("##Undefined", TreeNodeFlags, "%s", name.c_str());
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
