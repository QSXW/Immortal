#pragma once

#include "Immortal.h"

namespace Immortal
{

class HierarchyGraphics : public Layer
{
public:
    void DrawObjectNode(Scene *scene, Object &o)
    {
        auto &tag = o.Get<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = (selectedObject == o ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool isExpanded = ImGui::TreeNodeEx((void *)(uint64_t)o, flags, tag.c_str());
        ImGui::SameLine(64.0f);
        ImGui::NewLine();

        if (ImGui::IsItemClicked())
        {
            selectedObject = o;
        }

        bool isDeleted = false;
        if (Input::IsKeyPressed(KeyCode::Delete) && selectedObject == o)
        {
            isDeleted = true;
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem(WordsMap::Get("Delete")))
            {
                isDeleted = true;
            }

            ImGui::EndPopup();
        }

        if (isExpanded)
        {
            ImGui::TreePop();
        }

        if (isDeleted)
        {
            scene->DestroyObject(o);
            if (selectedObject == o)
            {
                selectedObject = {};
            }
        }
    }

    template <class Callback>
    void OnUpdate(Scene *scene, Callback callback)
    {
        ImGui::PushFont(GuiLayer::NotoSans.Bold);
        ImGui::Begin(WordsMap::Get("Project"));

        {
            scene->Registry().each([&](auto id)
            {
                Object o{ id, scene };
                if (o.Has<TagComponent>())
                {
                    DrawObjectNode(scene, o);
                }
            });
        }

        ImGui::End();
        ImGui::PopFont();

        callback(selectedObject);
    }

    void Select(Object o)
    {
        selectedObject = o;
    }

private:
    Object selectedObject;
};

}
