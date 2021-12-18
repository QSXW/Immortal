#pragma once

#include "Immortal.h"

using namespace Immortal;

class RenderLayer : public Layer
{
public:
	RenderLayer() :
		Layer{ "OpenGLSample2D" },
		cameraController{ (float)Application::App()->Width(), (float)Application::App()->Height(), true }
	{

	}

	~RenderLayer()
	{

	}

	void OnAttach()
	{
		renderTarget = Render::Create<RenderTarget>(RenderTarget::Description{ { 1920, 1080 }, { {  Format::RGBA8, Texture::Wrap::Clamp, Texture::Filter::Bilinear }, { Format::Depth } } });
		texture = Render::Preset()->WhiteTexture;

		renderTarget->Set(Color{ 0.10980392f, 0.10980392f, 0.10980392f, 1 });
		Render2D::Setup(renderTarget);
	}

	void OnDetach()
	{

	}

	void OnUpdate()
	{
		cameraController.OnUpdate(Application::App()->DeltaTime());

		Render2D::BeginScene(cameraController.Camera());

		static float rotation = 0.0f;
		rotation += Application::App()->DeltaTime() * 50.0f;

		Render2D::DrawQuad({ 0.0f, 0.0f }, { texture->Ratio() * 2.0, 2.0f }, texture, 1.0f, Vector::Color(1.f));
		Render2D::SetColor(color, luminance);
		Render2D::EndScene();

		Render::Begin(renderTarget, cameraController.Camera());
		Render2D::BeginScene(cameraController.Camera());
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				Vector4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
				Render2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
			}
		}
		Render2D::EndScene();
		Render::End();
	}

	void OnGuiRender()
	{
		static bool my_tool_active = true;
		static bool show_demo_window = false;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_FILE))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O"))
				{
					std::optional<std::string> filename = FileDialogs::OpenFile(FileDialogs::ImageFilter);
					if (filename.has_value())
					{
						texture = Render::Create<Texture>(filename.value());
						luminance = 0.0f;
						color = Vector::Vector4(0.0f);
						cameraController.SetZoomLevel(1.12f);
						cameraController.Resize((float)Application::App()->Width(), (float)Application::App()->Height());
						cameraController.Reset();
					}
				}
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
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

		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);
		}
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin(IMMORTAL_CONSTANT_STRING_CONSOLE);
			ImGui::Checkbox(IMMORTAL_CONSTANT_STRING_DEMO_CONSOLE, &show_demo_window);
			ImGui::Columns(2);
			ImGui::Text(U8("红色"));
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("#########", &color.r, -1, 1);
			ImGui::PopItemWidth();
			ImGui::NextColumn();
			ImGui::SliderFloat(U8("绿色"), &color.g, -1, 1);
			ImGui::SliderFloat(U8("蓝色"), &color.b, -1, 1);
			ImGui::SliderFloat(U8("透明度"), &color.a, -1, 1);
			ImGui::SliderFloat(U8("亮度"), &luminance, -1, 1);
			ImGui::Text(IMMORTAL_CONSTANT_STRING_RENDER_RATE, 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text(U8("图形设备：%s"), Render::GraphicsRenderer());

			ImGui::End();

			offline.OnUpdate(renderTarget);
		}
	}

	void OnEvent(Event &e)
	{
		LOG::INFO(e);
		cameraController.OnEvent(e);
	}

private:
	std::shared_ptr<RenderTarget> renderTarget;

	std::shared_ptr<Shader> shader;

	std::shared_ptr<Pipeline> pipeline;

	std::shared_ptr<Texture> texture;

	OrthographicCameraController cameraController;
	
	Widget::Viewport offline{ WordsMap::Get("offlineRender") };

	float luminance{ 0 };

	Vector4 color{ 0.0f, 0.0f, 0.0f, 0.0f };
};
