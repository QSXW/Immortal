#include "SceneHierarchyPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Immortal/ImGui/ui.h"

#include "Immortal/Scene/Scene.h"
#include "Immortal/Scene/Entity.h"

#include "Immortal/Utils/PlatformUtils.h"

namespace Immortal
{
	SceneHierarchyPanel::SceneHierarchyPanel()
	{
		Mesh::LoadPrimitives();
		SetContext(nullptr);
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		Mesh::LoadPrimitives();
		SetContext(scene);
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{

	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		mContext = scene;
		mSelectedEntity = {};
	}

	void SceneHierarchyPanel::OnGuiRender()
	{
		ImGui::Begin(UNICODE8("世界树"));
		{
			mContext->Registry().each([&](auto id)
			{
				Entity e{ id, mContext.get() };
				if (e.HasComponent<TagComponent>())
				{
					DrawEntityNode(e);
				}
			});
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			mSelectedEntity = { };
		}

		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::BeginMenu(UNICODE8("新建")))
			{
				if (ImGui::MenuItem(UNICODE8("空物体")))
				{
					mSelectedEntity = mContext->CreateEntity(UNICODE8("空物体"));
				}
				if (ImGui::BeginMenu(UNICODE8("网格")))
				{
					if (ImGui::MenuItem(UNICODE8("无网格原型")))
					{
						auto &e = mContext->CreateEntity(UNICODE8("空物体"));
						auto &c = e.AddComponent<MeshComponent>();
						mSelectedEntity = e;
					}
					if (ImGui::MenuItem(UNICODE8("立方体")))
					{
						auto &e = mContext->CreateEntity(UNICODE8("立方体"));
						auto &c = e.AddComponent<MeshComponent>();
						c.Mesh = Mesh::Get<Mesh::Primitive::Cube>();
						e.AddComponent<MaterialComponent>();
						mSelectedEntity = e;
					}
					if (ImGui::MenuItem(UNICODE8("球体")))
					{
						auto &e = mContext->CreateEntity(UNICODE8("球体"));
						auto &c = e.AddComponent<MeshComponent>();
						c.Mesh = Mesh::Get<Mesh::Primitive::Sphere>();
						e.AddComponent<MaterialComponent>();
						mSelectedEntity = e;
					}
					if (ImGui::MenuItem(UNICODE8("胶囊")))
					{
						auto &e = mContext->CreateEntity(UNICODE8("胶囊"));
						auto &c = e.AddComponent<MeshComponent>();
						c.Mesh = Mesh::Get<Mesh::Primitive::Capsule>();
						e.AddComponent<MaterialComponent>();
						mSelectedEntity = e;
					}
					if (ImGui::MenuItem(UNICODE8("圆锥体")))
					{
						auto &e = mContext->CreateEntity(UNICODE8("圆锥体"));
						auto &c = e.AddComponent<MeshComponent>();
						c.Mesh = Mesh::Get<Mesh::Primitive::Cone>();
						e.AddComponent<MaterialComponent>();
						mSelectedEntity = e;
					}
					if (ImGui::MenuItem(UNICODE8("圆柱体")))
					{
						auto &e = mContext->CreateEntity(UNICODE8("圆柱体"));
						auto &c = e.AddComponent<MeshComponent>();
						c.Mesh = Mesh::Get<Mesh::Primitive::Cylinder>();
						e.AddComponent<MaterialComponent>();
						mSelectedEntity = e;
					}
					if (ImGui::MenuItem(UNICODE8("平面")))
					{
						auto &e = mContext->CreateEntity(UNICODE8("平面"));
						auto &c = e.AddComponent<MeshComponent>();
						c.Mesh = Mesh::Get<Mesh::Primitive::Plane>();
						e.AddComponent<MaterialComponent>();
						mSelectedEntity = e;
					}

					ImGui::EndMenu();
				}

				if (ImGui::MenuItem(UNICODE8("相机")))
				{
					auto &e = mContext->CreateEntity(UNICODE8("相机"));
					e.AddComponent<CameraComponent>();
					mSelectedEntity = e;
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin(UNICODE8("属性"));
		if (mSelectedEntity)
		{
			DrawComponents(mSelectedEntity);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity & e)
	{
		auto &tag = e.GetComponent<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ((mSelectedEntity == e) ? ImGuiTreeNodeFlags_Selected : 0 )| ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool isExpanded = ImGui::TreeNodeEx((void *)(uint64_t)e, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{
			mSelectedEntity = e;
		}

		bool isDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem(UNICODE8("删除")))
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
			mContext->DestroyEntity(e);
			if (mSelectedEntity == e)
			{
				mSelectedEntity = { };
			}
		}
	}

	static bool DrawVec3Control(const std::string& label, Vector::Vector3& values, float speed = 0.01f, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		bool modified = false;

		const ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
			modified = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##X", &values.x, speed, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
			modified = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##Y", &values.y, speed, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
			modified = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##Z", &values.z, speed, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return modified;
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool canBeRemoved = true)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			//	This fixes an issue where the first "+" button would display the "Remove" buttons for ALL components on an Entity.
			//	This is due to ImGui::TreeNodeEx only pushing the id for it's children if it's actually open
			ImGui::PushID((void*)typeid(T).hash_code());
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, name.c_str());
			bool right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
			ImGui::PopStyleVar();

			bool resetValues = false;
			bool removeComponent = false;

			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }) || right_clicked)
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem(UNICODE8("重置")))
					resetValues = true;

				if (canBeRemoved)
				{
					if (ImGui::MenuItem(UNICODE8("移除组件")))
						removeComponent = true;
				}

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent || resetValues)
				entity.RemoveComponent<T>();

			if (resetValues)
				entity.AddComponent<T>();

			ImGui::PopID();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity &e)
	{
		ImGui::AlignTextToFramePadding();
		if (e.HasComponent<TagComponent>())
		{
			auto &tag = e.GetComponent<TagComponent>().Tag;
			static char buffer[256] = { 0 };
			memcpy(buffer, tag.c_str(), tag.size() + 1);
			if (ImGui::InputText(UNICODE8("标签"), buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		auto id = e.GetComponent<IDComponent>().uid;
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		// ID
		ImGui::SameLine();
		ImGui::TextDisabled("%llx", id);
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 textSize = ImGui::CalcTextSize(UNICODE8("添加组件"));
		ImGui::SameLine(contentRegionAvailable.x - (textSize.x + GImGui->Style.FramePadding.y));
		if (ImGui::Button(UNICODE8("添加组件")))
		{
			ImGui::OpenPopup("AddComponentPanel");
		}

		if (ImGui::BeginPopup("AddComponentPanel"))
		{
			if (!mSelectedEntity.HasComponent<CameraComponent>())
			{
				if (ImGui::Button(UNICODE8("相机")))
				{
					mSelectedEntity.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!mSelectedEntity.HasComponent<MeshComponent>())
			{
				if (ImGui::Button(UNICODE8("网格")))
				{
					MeshComponent& component = mSelectedEntity.AddComponent<MeshComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!mSelectedEntity.HasComponent<MaterialComponent>())
			{
				if (ImGui::Button(UNICODE8("材质")))
				{
					auto &c = mSelectedEntity.AddComponent<MaterialComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!mSelectedEntity.HasComponent<NativeScriptComponent>())
			{
				if (ImGui::Button(UNICODE8("脚本")))
				{
					mSelectedEntity.AddComponent<NativeScriptComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!mSelectedEntity.HasComponent<SpriteRendererComponent>())
			{
				if (ImGui::Button(UNICODE8("精灵渲染器")))
				{
					mSelectedEntity.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndPopup();
		}

		DrawComponent<TransformComponent>(UNICODE8("变换"), e, [](TransformComponent& t)
		{
			DrawVec3Control(UNICODE8("位置"), t.Position, 0.005f);
			Vector::Vector3 rotation = Vector::Degrees(t.Rotation);
			DrawVec3Control(UNICODE8("旋转"), rotation, 1.0f);
			t.Rotation = Vector::Radians(rotation);
			DrawVec3Control(UNICODE8("缩放"), t.Scale, 0.02f);
		}, false);

		DrawComponent<CameraComponent>(UNICODE8("相机"), e, [](CameraComponent& c)
			{
				auto &camera = c.Camera;

				ImGui::Checkbox(UNICODE8("主摄像机"), &c.Primary);

				static constexpr char *projectionTypeStrings[] = {
					UNICODE8("透视投影"),
					UNICODE8("正交投影")
				};

				bool isChanged = false;
				char *current = projectionTypeStrings[static_cast<int>(camera.Type())];
				if (ImGui::BeginCombo(UNICODE8("投影"), current))
				{
					for (int i = 0; i < sizeof(projectionTypeStrings) / sizeof(projectionTypeStrings[0]); i++)
					{
						bool isSelected = (current == projectionTypeStrings[i]);
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							IM_CORE_WARN("{0}", i);
							current = projectionTypeStrings[i];
							camera.SetProjectionType(static_cast<Camera::ProjectionType>(i));
							isChanged = true;
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}

				if (camera.Type() == Camera::ProjectionType::Perspective)
				{
					auto degrees = camera.PerspectiveVerticalFOV();
					if (ImGui::DragFloat(UNICODE8("视野角度"), &degrees, 0.02f, 0.0f))
					{
						camera.SetPerspectiveVerticalFOV(degrees);
					}
					ImGui::DragFloat(UNICODE8("近端平面"), &camera.PerspectiveNearClip(), 0.02f);
					ImGui::DragFloat(UNICODE8("远端平面"), &camera.PerspectiveFarClip(), 0.02f);
				}
				if (camera.Type() == Camera::ProjectionType::Orthographic)
				{
					ImGui::DragFloat(UNICODE8("尺寸"), &camera.OrthographicSize(), 0.02f);
					ImGui::DragFloat(UNICODE8("近端平面"), &camera.OrthographicNearClip(), 0.02f);
					ImGui::DragFloat(UNICODE8("远端平面"), &camera.OrthographicFarClip(), 0.02f);
				}

			}, true);

		DrawComponent<SpriteRendererComponent>(UNICODE8("贴图"), e, [&](SpriteRendererComponent& s)
			{
				ImGui::ColorEdit4(UNICODE8("颜色"), &s.Color.r);
				float aspect = (float)s.Texture->Width() / (float)s.Texture->Height();
				if (ImGui::ImageButton((void *)(uint64_t)(uint32_t)s.Texture->RendererID(), { aspect * 128.0f, 128.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
				{
					std::optional<std::string> filename = FileDialogs::OpenFile(FileDialogs::ImageFilter);
					if (filename.has_value())
					{
						s.Texture = Texture2D::Create(filename.value());
						auto &transform = e.Transform();
						if (s.Texture->Width() < s.Texture->Height())
						{
							transform.Scale = { s.Texture->Ratio() * 1.5f, 1.5f, 1.0f };
						}
						else
						{
							transform.Scale = { s.Texture->Ratio() * 1.0f, 1.0f, 1.0f };
						}
					}
				}
			}, true);

		DrawComponent<MeshComponent>(UNICODE8("网格"), e, [&](MeshComponent& c)
			{
				UI::BeginPropertyGrid();
				ImGui::SetColumnWidth(0, 100.0f);
				ImGui::Text(UNICODE8("网格"));
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::InputText("##assetRef", (char*)c.Mesh->Path(), 256, ImGuiInputTextFlags_ReadOnly);

				ImGui::PopItemWidth();
				ImGui::NextColumn();
				UI::EndPropertyGrid();
			});

		DrawComponent<NativeScriptComponent>(UNICODE8("脚本"), e, [&](NativeScriptComponent& c)
			{
				UI::BeginPropertyGrid();
				ImGui::SetColumnWidth(0, 100.0f);
				ImGui::Text(UNICODE8("脚本"));
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::InputText("##assetRef", (char*)c.Module.c_str(), c.Module.size(), ImGuiInputTextFlags_ReadOnly);

				ImGui::PopItemWidth();
				ImGui::NextColumn();
				if (ImGui::Button(UNICODE8("添加脚本")))
				{
					auto &res = FileDialogs::OpenFile("CPP Scripts(*.cpp)\0 * .cpp\0");
					if (res.has_value())
					{
						c.Module = res.value();
					}
				}
				
				UI::EndPropertyGrid();
			});

		DrawComponent<MaterialComponent>(UNICODE8("材质"), e, [&](MaterialComponent &c)
			{
				Texture::Description spec = { Texture::Format::RGB8, Texture::Wrap::Clamp, Texture::Filter::Linear };
				if (mSelectedEntity.HasComponent<MeshComponent>())
				{
					Ref<Mesh> mesh = mSelectedEntity.GetComponent<MeshComponent>().Mesh;

						// ImGui::Text("Shader: %s", "Physically-Based Renderering");
						// Textures ------------------------------------------------------------------------------
						{
							// Albedo
							if (ImGui::CollapsingHeader(UNICODE8("反照率(Albedo)"), nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

								auto& albedoColor = c.AlbedoColor;
								bool useAlbedoMap = true;
								ImGui::Image((void*)(UINT64)c.AlbedoMap->RendererID(), ImVec2(64, 64));

								ImGui::PopStyleVar();
								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
									ImGui::TextUnformatted(c.AlbedoMap->Path());
									ImGui::PopTextWrapPos();
									ImGui::Image((void*)(UINT64)c.AlbedoMap->RendererID(), ImVec2(384, 384));
									ImGui::EndTooltip();

									if (ImGui::IsItemClicked())
									{
										auto &filename = FileDialogs::OpenFile("");
										if (filename.has_value())
										{
											c.AlbedoMap = Texture2D::Create(filename.value(), Texture::Wrap::Repeat, Texture::Filter::Linear);
										}
									}
								}
								ImGui::SameLine();
								ImGui::BeginGroup();
								ImGui::Checkbox("##AlbedoMap", &useAlbedoMap);
								ImGui::EndGroup();
								ImGui::SameLine();
								ImGui::ColorEdit3("Color##Albedo", glm::value_ptr(albedoColor), ImGuiColorEditFlags_NoInputs);
							}
						}
						{
							// Normals
							if (ImGui::CollapsingHeader(UNICODE8("法线(Normal)"), nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								bool useNormalMap = true;
								ImGui::Image((void*)(UINT64)c.NormalMap->RendererID(), ImVec2(64, 64));

								ImGui::PopStyleVar();
								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
									ImGui::TextUnformatted(c.NormalMap->Path());
									ImGui::PopTextWrapPos();
									ImGui::Image((void*)(UINT64)c.NormalMap->RendererID(), ImVec2(384, 384));
									ImGui::EndTooltip();

									if (ImGui::IsItemClicked())
									{
										auto &filename = FileDialogs::OpenFile("");
										if (filename.has_value())
										{
											c.NormalMap = Texture2D::Create(filename.value(), Texture::Wrap::Repeat, Texture::Filter::Linear);
										}
									}
								}
								ImGui::SameLine();
								ImGui::BeginGroup();
								ImGui::Checkbox("##AlbedoMap", &useNormalMap);
								ImGui::EndGroup();
							}
						}
						{
							// Metalness
							if (ImGui::CollapsingHeader(UNICODE8("金属性(Metalness)"), nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								bool useMetalnessMap = true;
								ImGui::Image((void*)(UINT64)c.MetalnessMap->RendererID(), ImVec2(64, 64));

								ImGui::PopStyleVar();
								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
									ImGui::TextUnformatted(c.MetalnessMap->Path());
									ImGui::PopTextWrapPos();
									ImGui::Image((void*)(UINT64)c.MetalnessMap->RendererID(), ImVec2(384, 384));
									ImGui::EndTooltip();

									if (ImGui::IsItemClicked())
									{
										auto &filename = FileDialogs::OpenFile("");
										if (filename.has_value())
										{
											c.MetalnessMap = Texture2D::Create(filename.value(), Texture::Wrap::Repeat, Texture::Filter::Linear);
										}
									}
								}
								ImGui::SameLine();
								ImGui::Checkbox("##MetalnessMap", &useMetalnessMap);
								ImGui::SameLine();
								ImGui::SliderFloat("Value##MetalnessInput", &c.Metalness, 0.0f, 1.0f);
							}
						}
						{
							// Roughness
							if (ImGui::CollapsingHeader(UNICODE8("粗糙度(Roughness))"), nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								bool useRoughnessMap = true;
								ImGui::Image((void*)(UINT64)c.RoughnessMap->RendererID(), ImVec2(64, 64));

								ImGui::PopStyleVar();
								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
									ImGui::TextUnformatted(c.RoughnessMap->Path());
									ImGui::PopTextWrapPos();
									ImGui::Image((void*)(UINT64)c.RoughnessMap->RendererID(), ImVec2(384, 384));
									ImGui::EndTooltip();

									if (ImGui::IsItemClicked())
									{
										auto &filename = FileDialogs::OpenFile("");
										if (filename.has_value())
										{
											c.RoughnessMap = Texture2D::Create(filename.value(), Texture::Wrap::Repeat, Texture::Filter::Linear);
										}
									}
								}
								ImGui::SameLine();
								ImGui::Checkbox("##RoughnessMap", &useRoughnessMap);
								ImGui::SameLine();
								ImGui::SliderFloat("Value##RoughnessInput", &c.Roughness, 0.0f, 1.0f);
							}
						}
					}
			});
	}
}