#include "impch.h"
#include "EditorLayer.h"

#include "Script/ScriptDriver.h"

#define DEBUG_IMAGE_PATH(x) std::string("D:/Digital Media/Assests/jpeg/"#x)

namespace Immortal {

	std::string EditorLayer::sWorkspace = "D:/Code/C++/Scripts";

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), mCameraController{ (float)Application::Width(), (float)Application::Height(), true }
	{
		
	}

	void EditorLayer::OnAttach()
	{
		//mTexture = Texture2D::Create(DEBUG_IMAGE_PATH(test17.jpg));

		Framebuffer::Specification spec;

		spec.Attachments = {
			{ Texture::Format::RGBA32F },
			{ Texture::Format::RedInterger },
			{ Texture::Format::Depth }
		};

		spec.Width = 1920;
		spec.Height = 1080;
		mFramebuffer = Framebuffer::Create(spec);

		mActiveScene = CreateRef<Scene>("Active Scene");
		mEditorCamera = EditorCamera(Vector::PerspectiveFOV(Vector::Radians(45.0f), mViewportSize.x, mViewportSize.y, 0.1f, 1000.0f));

		mPlayButtonTexture = Texture2D::Create("assets/Icon/Play.png");
		mPauseButtonTexture = Texture2D::Create("assets/Icon/Pause.png");
		mStopButtonTexture = Texture2D::Create("assets/Icon/Stop.png");

		mSceneHierarchyPanel.SetContext(mActiveScene);
	}

	void EditorLayer::OnDetach()
	{
		
	}

	void EditorLayer::OnUpdate()
	{
		if (Framebuffer::Specification spec = mFramebuffer->GetSpecification();
			mViewportSize.x != spec.Width || mViewportSize.y != spec.Height)
		{
			mFramebuffer->Resize(static_cast<uint32_t>(mViewportSize.x), static_cast<uint32_t>(mViewportSize.y));
			mActiveScene->SetViewportSize(mViewportSize);
			mEditorCamera.SetViewportSize(mViewportSize);
		}

		/* Only Update when the viewport was focused. */
		
		{
			RenderCommand::SetClearColor({ 0.18F, 0.18f, 0.18f, 1.0 });
			RenderCommand::Clear();
			// Renderer2D::SetColor(mRGBA, mBrightness);

			mFramebuffer->ClearAttachment(1, -1);
			switch (mState)
			{
				case SceneState::Edit:
				{
					if (mViewportFocused)
					{
						mEditorCamera.OnUpdate();
					}
					mActiveScene->OnRenderEditor(mEditorCamera);

					mFramebuffer->Map();
					RenderCommand::SetClearColor({ 0.18F, 0.18f, 0.18f, 1.0 });
					RenderCommand::Clear();
					Renderer::Shader<ShaderName::Tonemap>()->Map();
					glBindTextureUnit(0, mActiveScene->mFramebuffer->ColorAttachmentRendererID());
					RenderCommand::DrawIndexed(mActiveScene->mToneMap);
					Renderer::Shader<ShaderName::Tonemap>()->UnMap();
					mFramebuffer->UnMap();
					break;
				}
				case SceneState::Play:
				{
					mActiveScene->OnRenderRuntime();
					mFramebuffer->Map();
					RenderCommand::SetClearColor({ 0.18F, 0.18f, 0.18f, 1.0 });
					RenderCommand::Clear();
					Renderer::Shader<ShaderName::Tonemap>()->Map();
					glBindTextureUnit(0, mActiveScene->mFramebuffer->ColorAttachmentRendererID());
					mActiveScene->mToneMap->Map();
					RenderCommand::DrawIndexed(mActiveScene->mToneMap);
					Renderer::Shader<ShaderName::Tonemap>()->UnMap();
					mFramebuffer->UnMap();
					break;
				}
				case SceneState::Pause:
				{
					break;
				}
			}
		}
	}

	void EditorLayer::OnGuiRender()
	{
		/*
		ImGui::Begin("Test");
		{
			ImGui::Image((void *)(size_t)mActiveScene->mFramebuffer->ColorAttachmentRendererID(), ImVec2{ 1280, 720 }, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });
		}
		ImGui::End();
		*/
		static bool my_tool_active = true;
		static bool show_demo_window = false;
		static bool p_open = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		}

		ImGui::Begin("DockSpace Demo", &p_open, window_flags);

		if (!opt_padding)
		{
			ImGui::PopStyleVar();
		}
		if (opt_fullscreen)
		{
			ImGui::PopStyleVar(2);
		}

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle &style = ImGui::GetStyle();

		float minWindowSize = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		style.WindowMinSize.x = minWindowSize;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_FILE))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O"))
				{
					LoadObject(mActiveScene);
				}

				if (ImGui::MenuItem(U8("新建场景")))
				{
					NewScene();
				}

				if (ImGui::MenuItem(U8("保存场景")))
				{
					SaveSceneAs();
				}

				if (ImGui::MenuItem(U8("加载场景")))
				{
					LoadScene();
				}

				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }

				if (ImGui::MenuItem(U8("保存为..."), "Ctrl+Alt+S")) { /* Do stuff */ }

				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_EXIT, "Ctrl+W")) { Application::App()->Close(); }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_EDIT))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_VIEW))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_GRAPHICS))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_HELP))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin(IMMORTAL_CONSTANT_STRING_CONSOLE);
		{
			if (show_demo_window)
			{
				ImGui::ShowDemoWindow(&show_demo_window);
			}
			static int counter = 0;
			ImGui::Checkbox(IMMORTAL_CONSTANT_STRING_DEMO_CONSOLE, &show_demo_window);
			ImGui::Text(IMMORTAL_CONSTANT_STRING_RENDER_RATE, 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text(U8("图形设备：%s"), Application::App()->GetWindow().Context().GraphicsRenderer());
			ImGui::Text(U8("驱动版本：%s"), Application::App()->GetWindow().Context().DriverVersion());
			ImGui::Text(U8("图形数量：%d"), Renderer2D::Stats().QuadCount);
		}
		ImGui::End();

		ImGui::Begin(U8("调色板"));
		{
			ImGui::NextColumn();
			ImGui::Columns(2);
			ImGui::Text(U8("  红色"));
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("###RED", &mRGBA.r, -1, 1);
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Columns(2);
			ImGui::Text(U8("  绿色"));
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("###GREEN", &mRGBA.g, -1, 1);
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Columns(2);
			ImGui::Text(U8("  蓝色"));
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("###BLUE", &mRGBA.b, -1, 1);
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Columns(2);
			ImGui::Text(U8("  透明度"));
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("###ALPHA", &mRGBA.a, -1, 1);
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Columns(2);
			ImGui::Text(U8("  亮度"));
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("###BRIGHTNESS", &mBrightness, -1, 1);
			ImGui::PopItemWidth();
			ImGui::NextColumn();
			ImGui::Columns(2);

			ImGui::Text(U8("  色相"));
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("###HUE", &mHue, 0.0f, 6.0f);
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Text(U8("  饱和度"));
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("###SATURATION", &mSaturation, -1.0f, 1.0f);
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Text(U8("  明度"));
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("###LUMINANCE", &mLuminance, -1.0f, 1.0f);
			ImGui::PopItemWidth();
			ImGui::NextColumn();

		}
		ImGui::End();

		mSceneHierarchyPanel.OnGuiRender();
		
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoTitleBar);

			mViewportFocused = ImGui::IsWindowFocused();
			mViewportHovered = ImGui::IsWindowHovered();
			Application::App()->GetGuiLayer()->BlockEvent(!mViewportFocused && !mViewportHovered);

			auto [x, y] = ImGui::GetContentRegionAvail();
			mViewportSize = { x, y };

			uint32_t textureID = (mFramebuffer->ColorAttachmentRendererID());
			ImGui::Image((void *)(size_t)textureID, ImVec2{ x, y }, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });
			
			auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
			auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			auto viewportOffset = ImGui::GetWindowPos();
			mViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
			mViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

			/*_WARN("Min Bounds = {0}, {1}", mViewportBounds[0].x, mViewportBounds[0].y);
			_WARN("Max Bounds = {0}, {1}", mViewportBounds[1].x, mViewportBounds[1].y);*/

			/*---- Gizmos ----------------------------------------------------------------*/
			
			auto &e = mSceneHierarchyPanel.SelectedEntity();
			if (e && mGuizmoType != ImGuizmo::OPERATION::INVALID)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
				
				Vector::Matrix4 cameraProjectionMatrix;
				Vector::Matrix4 cameraViewMatrix;
				switch (mState)
				{
					case SceneState::Edit:
					{
						cameraProjectionMatrix = mEditorCamera.Projection();
						cameraViewMatrix = mEditorCamera.View();
						break;
					}
					case SceneState::Play:
					{
						auto cameraEntity = mActiveScene->PrimaryCameraEntity();
						if (cameraEntity)
						{
							auto &camera = cameraEntity.GetComponent<CameraComponent>().Camera;
							cameraProjectionMatrix = camera.Projection();
							cameraViewMatrix = Vector::Inverse(cameraEntity.Transform());
						}
						break;
					}
					default:
						break;
				}
				
				auto &t = e.Transform();
				Vector::mat4 transform = t.Transform();

				bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
				float snapValues[][3] = {
					{ 0.5f, 0.5f, 0.5f },
					{ 45.0f, 45.0f, 45.0f },
					{ 0.5f, 0.5f, 0.5f }
				};

				ImGuizmo::Manipulate(&cameraViewMatrix[0].x,
					&cameraProjectionMatrix[0].x,
					static_cast<ImGuizmo::OPERATION>(mGuizmoType),
					ImGuizmo::LOCAL,
					&transform[0].x,
					nullptr,
					snap ? snapValues[mGuizmoType] : nullptr);

				if (ImGuizmo::IsUsing())
				{
					Vector::Vector3 rotation;
					Vector::DecomposeTransform(transform, t.Position, rotation, t.Scale);

					/* Perfect! No Gimbal lock no everything will work as expected here. */
					Vector::Vector3 deltaRotation = rotation - t.Rotation;
					t.Rotation += deltaRotation;
				}
			}

			/*---- Gizmos ----------------------------------------------------------------*/
			ImGui::End(); /* Viewport */
			ImGui::PopStyleVar();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.305f, 0.31f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.1505f, 0.151f, 0.5f));
		ImGui::Begin("##tool_bar", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			float size = ImGui::GetWindowHeight() - 4.0F;
			ImGui::SameLine((ImGui::GetWindowContentRegionMax().x / 2.0f) - (1.5f * (ImGui::GetFontSize() + ImGui::GetStyle().ItemSpacing.x)) - (size / 2.0f));
			Ref<Texture2D> buttonTex = mState == SceneState::Play ? mStopButtonTexture : mPlayButtonTexture;
			if (ImGui::ImageButton((void*)(UINT32)buttonTex->RendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
			{
				if (mState == SceneState::Edit)
				{
					OnPlay();
				}
				else
				{
					OnStop();
				}
			}

			ImGui::SameLine();

			if (ImGui::ImageButton((void*)(UINT32)mPauseButtonTexture->RendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
			{
				if (mState == SceneState::Play)
				{
					//OnScenePause();
					mState = SceneState::Pause;
				}
				else if (mState == SceneState::Pause)
				{
					//OnSceneResume();
					mState = SceneState::Play;
				}
			}
		}
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
		ImGui::End();

		{
			static AppLog log;
			static bool logOpen;
			log.Clear();
			log.AddLog(Compiler::Log.c_str());
			log.Draw(U8("调试日志"), &logOpen);
		}

		ImGui::End(); /* Dock place */
	}

	void EditorLayer::OnEvent(Event & e)
	{
		if (mState == SceneState::Edit)
		{
			if (mViewportHovered)
			{
				mEditorCamera.OnEvent(e);
			}
		}
		else if (mState == SceneState::Play)
		{
			// Update the event of Scene when play
		}
		
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(IM_BIND_EVENT_FUNC(EditorLayer::OnResize));
		dispatcher.Dispatch<KeyPressedEvent>(IM_BIND_EVENT_FUNC(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(IM_BIND_EVENT_FUNC(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnResize(WindowResizeEvent &e)
	{
		mFramebuffer->Resize(static_cast<uint32_t>(mViewportSize.x), static_cast<uint32_t>(mViewportSize.y));
		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
	{
		if (mViewportHovered && e.GetMouseButton() == MouseCode::Left && !ImGuizmo::IsOver())
		{
			auto [x, y] = ImGui::GetMousePos();
			x -= mViewportBounds[0].x;
			y -= mViewportBounds[0].y;

			// flip the y axis to match the texture coordinate
			y = mViewportSize.y - y;

			int mouseX = (int)x;
			int mouseY = (int)y;

			// int pixel = (int)mFramebuffer->ReadPixel(0, mouseX, mouseY, mFramebuffer->GetSpecification().Attachments.Attachments[0].Format);
			int pixel = (int)mFramebuffer->ReadPixel(1, mouseX, mouseY, mFramebuffer->GetSpecification().Attachments.Attachments[1].Format);

			if (pixel > 0 && pixel < 100)
			{
				mSceneHierarchyPanel.SelectedEntity() = { pixel, mActiveScene.get() };
			}
			else
			{
				mSceneHierarchyPanel.SelectedEntity() = {};
			}

			// _WARN("pixel = {0}, {1}, {2}, {3}", ((uint8_t *)&pixel)[0], ((uint8_t *)&pixel)[1], ((uint8_t *)&pixel)[2], ((uint8_t *)&pixel)[3]);
			LOG::WARN("pixel = {0}", pixel);

			return false;
		}
		return true;
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent &e)
	{
		if (e.RepeatCount() > 0)
		{
			return false;
		}
		
		// @Gizmo Shortcut Key
		if (GImGui->ActiveId == 0 && mViewportHovered && mState == SceneState::Edit)
		{
			switch (e.GetKeyCode())
			{
				case static_cast<int>(KeyCode::Q) :
				{
					mGuizmoType = ImGuizmo::OPERATION::INVALID;
					break;
				}
				case static_cast<int>(KeyCode::W) :
				{
					mGuizmoType = ImGuizmo::OPERATION::TRANSLATE;
					break;
				}
				case static_cast<int>(KeyCode::E) :
				{
					mGuizmoType = ImGuizmo::OPERATION::ROTATE;
					break;
				}
				case static_cast<int>(KeyCode::R) :
				{
					mGuizmoType = ImGuizmo::OPERATION::SCALE;
					break;
				}
				case static_cast<int>(KeyCode::F) :
				{
					auto &e = mSceneHierarchyPanel.SelectedEntity();
					if (e)
					{
						mEditorCamera.Focus(e.Transform().Position);
					}
					break;
				}
			}
		}

		// @Shortcut key
		bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
		bool shift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);
		switch (e.GetKeyCode())
		{
			case (int)KeyCode::S:
			{
				if (control && shift)
				{
					SaveSceneAs();
				}
				break;
			}
			case (int)KeyCode::N:
			{
				if (control && shift)
				{
					NewScene();
				}
				break;
			}
			case (int)KeyCode::O:
			{
				if (control && shift)
				{
					LoadScene();
				}
				break;
			}
		default:
			return false;
		}
		return true;
	}

	bool EditorLayer::LoadObject(const Ref<Scene>& scene)
	{
		auto res = FileDialogs::OpenFile(FileDialogs::ImageFilter);
		if (res.has_value())
		{
			const char *path = res.value().c_str();
			const UINT32 *extension = reinterpret_cast<const UINT32 *>(path + res.value().size() - 4);
			
			if (*extension & static_cast<UINT32>(FileDialogs::Type::FBX))
			{
				auto &e = mActiveScene->CreateEntity(U8("模型"));
				auto &c = e.AddComponent<MeshComponent>();
				c.Mesh = CreateRef<Mesh>(res.value());
				e.AddComponent<MaterialComponent>();
				mSceneHierarchyPanel.SelectedEntity() = e;
			}
			else
			{
				mTexture = Texture2D::Create(res.value());
				auto &object = mActiveScene->CreateEntity("Image" + std::to_string(mImageLayer.size()));
				object.AddComponent<SpriteRendererComponent>(mTexture);
				mImageLayer.push_back(object);
				auto &transform = object.Transform();
				transform.Scale = { mTexture->Ratio() * 1.5f, 1.5f, 1.0f };
				mLuminance = 0.0f;
				mRGBA = Vector::Vector4(0.0f);
			}
	
			return true;
		}

		return false;
	}

	void EditorLayer::OnPlay()
	{
		std::vector<std::string> scripts;
		mActiveScene->Registry().view<NativeScriptComponent>().each([&](auto e, auto &c)
			{
				if (!c.Module.empty())
				{
					scripts.emplace_back(c.Module);
				}
			});

		auto &scene = mActiveScene;
		NativeScriptDriver::On(sWorkspace, scripts,
			[&scene](std::vector<ScriptableObject *> objects)
			{
				auto view = scene->Registry().view<NativeScriptComponent>();
				size_t i = 0;
				for (auto e : view)
				{
					auto &scriptComponent = view.get<NativeScriptComponent>(e);
					if (i < objects.size())
					{
						scriptComponent.Status = NativeScriptComponent::Status::Ready;
						// Not necessary to release memory here. The previous would 
						// be removed automatically with the DLL
						scriptComponent.Map({ e, scene.get() }, objects[i++]);
					}
					else
					{
						scriptComponent.Status = NativeScriptComponent::Status::NotLoaded;
					}
				}
			},
			[&scene]()
			{
				auto view = scene->Registry().view<NativeScriptComponent>();
				for (auto e : view)
				{
					view.get<NativeScriptComponent>(e).Status = NativeScriptComponent::Status::NotLoaded;
				}
			}
		);
		mState = SceneState::Play;
	}

	void EditorLayer::OnStop()
	{
		mState = SceneState::Edit;
	}

	bool EditorLayer::NewScene()
	{
		mActiveScene = CreateRef<Scene>("Untitled");
		mActiveScene->SetViewportSize(mViewportSize);
		mSceneHierarchyPanel.SetContext(mActiveScene);
		return true;
	}


	bool EditorLayer::LoadScene()
	{
		auto res = FileDialogs::OpenFile("Immortal Scene (*.immortal)\0*.immortal\0");
		if (res.has_value())
		{
			mActiveScene = CreateRef<Scene>(res.value());
			mActiveScene->SetViewportSize(mViewportSize);
			mSceneHierarchyPanel.SetContext(mActiveScene);

			SceneSerializer serializer(mActiveScene);
			return serializer.Deserialize(res.value());
		}

		return false;
	}

	bool EditorLayer::SaveSceneAs()
	{
		SceneSerializer serializer(mActiveScene);

		auto res = FileDialogs::SaveFile("Immortal Scene (*.immortal)\0*.immortal\0");
		
		if (res.has_value())
		{
			serializer.Serialize(res.value());;
		}

		return false;
	}

}