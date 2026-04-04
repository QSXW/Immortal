#pragma once

#include <cstdint>
#include <vector>

#include "Immortal.h"

namespace Immortal
{

static inline void AddComponents(Scene *scene, Object &object)
{
    if (ImGui::MenuItem(Translator::Translate("Script Component").c_str()))
    {
        object.AddComponent<ScriptComponent>();
    }
    if (ImGui::MenuItem(Translator::Translate("Light Component").c_str()))
    {
        object.AddComponent<LightComponent>();
    }
    if (ImGui::MenuItem(Translator::Translate("Camera Component").c_str()))
    {
        object.AddComponent<CameraComponent>();
    }
}

class WHierarchyGraphics : public Widget
{
public:
    template <class T>
    WHierarchyGraphics(T callback, Widget *parent = nullptr) :
        Widget{ parent },
	    scene{},
	    selectedObject{},
	    callback{ callback }
    {

    }

    virtual bool Draw() override
    {
        if (!scene)
        {
			return false;
        }

		ImGui::PushFont(GuiLayer::NotoSans.Bold);
		ImGui::Begin(Translator::Translate("Project").c_str());

		static ImGuiTextFilter filter;
		ImGui::SetNextItemWidth(-FLT_MIN);
		filter.Draw(Translator::Translate("Filter (inc,-exc)").c_str());

		ImGui::Checkbox(Translator::Translate("Use Clipper").c_str(), &useClipper);

		Object pendingDestroy{};

		if (ImGui::BeginTable("##hierarchy", 1, ImGuiTableFlags_RowBg))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow |
			                               ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth |
			                               ImGuiTreeNodeFlags_NavLeftJumpsToParent | ImGuiTreeNodeFlags_DrawLinesToNodes;

			const bool rootOpen = ImGui::TreeNodeEx("##scene_root", rootFlags, "%s", Translator::Translate("Scene").c_str());
			if (rootOpen)
			{
				std::vector<entt::entity> entities;
				{
					auto view = scene->Registry().view<TagComponent>();
					for (auto e : view)
					{
						const auto &tag = view.get<TagComponent>(e).Tag;
						if (filter.PassFilter(tag.c_str()))
						{
							entities.push_back(e);
						}
					}
				}

				if (useClipper)
				{
					ImGuiListClipper clipper;
					clipper.Begin((int)entities.size());
					while (clipper.Step())
					{
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
						{
							Object object{ entities[(size_t)i], scene };
							DrawObjectRow(scene, object, pendingDestroy);
						}
					}
				}
				else
				{
					for (auto e : entities)
					{
						Object object{ e, scene };
						DrawObjectRow(scene, object, pendingDestroy);
					}
				}

				ImGui::TreePop();
			}

			ImGui::EndTable();
		}

		if (pendingDestroy)
		{
			scene->DestroyObject(pendingDestroy);
			if (selectedObject == pendingDestroy)
			{
				selectedObject = {};
			}
		}

		ImGui::End();
		ImGui::PopFont();

		callback(selectedObject);

	    scene->OnGuiRender();

        return false;
    }

    void OnUpdate(Scene *other)
    {
		scene = other;
    }

    void Select(Object object)
    {
        selectedObject = object;
    }

private:
	void DrawBoneMeshSelectables(Object sceneObject, MeshComponent &meshComp, Ref<Mesh> mesh, BoneNode *node, bool bulletEachRow = false)
	{
		auto &nodeList = mesh->NodeList();
		for (size_t i = 0; i < node->Meshes.Size(); i++)
		{
			const uint32_t mi = node->Meshes[i];
			if (mi >= nodeList.size())
			{
				continue;
			}
			ImGui::PushID((int)mi);
			const std::string &meshName = nodeList[mi].Name;
			const char *label = meshName.empty() ? "<mesh>" : meshName.c_str();
			const bool sel = (selectedObject == sceneObject && meshComp.SelectedDrawNodeIndex == mi);
			if (bulletEachRow)
			{
				ImGui::Bullet();
				ImGui::SameLine();
			}
			if (ImGui::Selectable(label, sel))
			{
				selectedObject = sceneObject;
				meshComp.SelectedDrawNodeIndex = mi;
			}
			ImGui::PopID();
		}
	}

	void DrawFlatMeshSubnodes(Object sceneObject, MeshComponent &meshComp, Ref<Mesh> mesh)
	{
		auto &nodeList = mesh->NodeList();
		for (size_t i = 0; i < nodeList.size(); i++)
		{
			ImGui::PushID((int)i);
			const std::string &meshName = nodeList[i].Name;
			const char *label = meshName.empty() ? "<mesh>" : meshName.c_str();
			const bool sel = (selectedObject == sceneObject && meshComp.SelectedDrawNodeIndex == (uint32_t)i);
			ImGui::Bullet();
			ImGui::SameLine();
			if (ImGui::Selectable(label, sel))
			{
				selectedObject = sceneObject;
				meshComp.SelectedDrawNodeIndex = (uint32_t)i;
			}
			ImGui::PopID();
		}
	}

	void DrawMeshBoneSubtree(Object sceneObject, MeshComponent &meshComp, BoneNode *node, Ref<Mesh> mesh)
	{
		if (!node)
		{
			return;
		}

		ImGui::PushID((void *)(uintptr_t)node);

		const bool hasChildren = node->Children.Size() > 0;
		const bool hasMeshes = node->Meshes.Size() > 0;
		const char *displayName = node->Name.empty() ? "<node>" : node->Name.c_str();

		if (!hasChildren && !hasMeshes)
		{
			ImGui::BulletText("%s", displayName);
			ImGui::PopID();
			return;
		}

		/* No child bones: leaf — use bullet (circle), not an expandable tree row. */
		if (!hasChildren && hasMeshes)
		{
			if (!node->Name.empty())
			{
				ImGui::Bullet();
				ImGui::SameLine();
				ImGui::TextUnformatted(displayName);
				ImGui::Indent();
				DrawBoneMeshSelectables(sceneObject, meshComp, mesh, node, false);
				ImGui::Unindent();
			}
			else
			{
				DrawBoneMeshSelectables(sceneObject, meshComp, mesh, node, true);
			}
			ImGui::PopID();
			return;
		}

		/* Has child bones: expandable folder; meshes list under this node when open. */
		ImGuiTreeNodeFlags nflags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth |
		                            ImGuiTreeNodeFlags_NavLeftJumpsToParent | ImGuiTreeNodeFlags_DrawLinesToNodes;
		const bool open = ImGui::TreeNodeEx("##bone", nflags, "%s", displayName);
		if (ImGui::IsItemClicked())
		{
			selectedObject = sceneObject;
		}

		if (open)
		{
			DrawBoneMeshSelectables(sceneObject, meshComp, mesh, node);
			for (size_t c = 0; c < node->Children.Size(); c++)
			{
				DrawMeshBoneSubtree(sceneObject, meshComp, &node->Children[c], mesh);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	void DrawObjectRow(Scene *scene, Object &object, Object &pendingDestroy)
	{
		auto &tag = object.GetComponent<TagComponent>().Tag;

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::PushID((void *)(uintptr_t)(uint64_t)object);

		MeshComponent *meshComp = object.HasComponent<MeshComponent>() ? &object.GetComponent<MeshComponent>() : nullptr;
		Ref<Mesh> mesh = (meshComp && meshComp->Mesh) ? meshComp->Mesh : Ref<Mesh>{};

		const bool showMeshSubtree = mesh && (mesh->Size() > 1 || mesh->GetRootNode() != nullptr);

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
		                           ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_NavLeftJumpsToParent |
		                           ImGuiTreeNodeFlags_DrawLinesToNodes;

		if (!showMeshSubtree)
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		if (selectedObject == object)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		ImGui::SetNextItemStorageID((ImGuiID)(uint32_t)(uint64_t)object);

		const bool entityOpen = ImGui::TreeNodeEx("##node", flags, "%s", tag.c_str());

		if (ImGui::IsItemClicked())
		{
			selectedObject = object;
			if (meshComp)
			{
				meshComp->SelectedDrawNodeIndex = UINT32_MAX;
			}
		}

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::BeginMenu(Translator::Translate("Add Component").c_str()))
			{
				AddComponents(scene, object);
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem(Translator::Translate("Delete").c_str()))
			{
				pendingDestroy = object;
			}

			ImGui::EndPopup();
		}

		if (Input::IsKeyPressed(KeyCode::Delete) && selectedObject == object && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
		{
			pendingDestroy = object;
		}

		if (showMeshSubtree && entityOpen && meshComp && mesh)
		{
			if (BoneNode *root = mesh->GetRootNode())
			{
				DrawMeshBoneSubtree(object, *meshComp, root, mesh);
			}
			else
			{
				DrawFlatMeshSubnodes(object, *meshComp, mesh);
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	Scene *scene;

    Object selectedObject;

	bool useClipper = false;

    std::function<void(Object)> callback;
};

}
