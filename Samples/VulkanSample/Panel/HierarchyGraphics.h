#pragma once

#include "Immortal.h"

namespace Immortal
{

static inline void AddComponents(Scene *scene, Object &object)
{
    if (ImGui::MenuItem(WordsMap::Get("Script Component")))
    {
        object.AddComponent<ScriptComponent>();
    }
    if (ImGui::MenuItem(WordsMap::Get("Light Component")))
    {
        object.AddComponent<LightComponent>();
    }
    if (ImGui::MenuItem(WordsMap::Get("Camera Component")))
    {
        object.AddComponent<CameraComponent>();
    }
}

class HierarchyGraphics : public Layer
{
public:
    void DrawObjectNode(Scene *scene, Object &object)
    {
        auto &tag = object.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = (selectedObject == object ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool isExpanded = ImGui::TreeNodeEx((void *)(uint64_t)object, flags, "%s", tag.c_str());
        ImGui::SameLine(64.0f);
        ImGui::NewLine();

        if (ImGui::IsItemClicked())
        {
            selectedObject = object;
        }

        bool isDeleted = false;
        if (Input::IsKeyPressed(KeyCode::Delete) && selectedObject == object)
        {
            isDeleted = true;
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::BeginMenu(WordsMap::Get("Add Component")))
            {
                AddComponents(scene, object);
                ImGui::EndMenu();
            }
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
            scene->DestroyObject(object);
            if (selectedObject == object)
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

        scene->Registry().each([&](auto id)
        {
            Object object{ id, scene };
            if (object.HasComponent<TagComponent>())
            {
                DrawObjectNode(scene, object);
            }
        });      

        ImGui::End();
        ImGui::PopFont();

        callback(selectedObject);
    }

    void Select(Object object)
    {
        selectedObject = object;
    }

private:
    Object selectedObject;
};

}
