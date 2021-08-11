#include "impch.h"
#include "OpenGLSample2D.h"

#define DEBUG_IMAGE_PATH(x) std::string("C:/SDK/Assets/jpeg/"#x)

OpenGLSample2D::OpenGLSample2D()
	: Layer("OpenGLSample2D"), mCameraController{ (float)Immortal::Application::App()->GetWindow().Width(), (float)Immortal::Application::App()->GetWindow().Height(), true }
{
	Immortal::Renderer::Init();
}

void OpenGLSample2D::OnAttach()
{
	mTexture = Immortal::Texture2D::Create(DEBUG_IMAGE_PATH(wallhaven-x8j1l3.png));
}

void OpenGLSample2D::OnDetach()
{

}

void OpenGLSample2D::OnUpdate()
{
	mCameraController.OnUpdate(Immortal::Application::App()->DeltaTime());

	Immortal::RenderCommand::SetClearColor({ 0.18F, 0.18f, 0.18f, 1.0 });
	Immortal::RenderCommand::Clear();

	Immortal::Renderer2D::BeginScene(mCameraController.Camera());

	static float rotation = 0.0f;
	rotation += Immortal::Application::App()->DeltaTime() * 50.0f;

	Immortal::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { mTexture->Ratio() * 2.0, 2.0f }, mTexture, 1.0f, Immortal::Vector::Color(1.f));
	Immortal::Renderer2D::SetColor(mSquareColor, mLuminance);
	Immortal::Renderer2D::EndScene();

	Immortal::Renderer2D::BeginScene(mCameraController.Camera());
	for (float y = -5.0f; y < 5.0f; y += 0.5f)
	{
		for (float x = -5.0f; x < 5.0f; x += 0.5f)
		{
			glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
			Immortal::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
		}
	}
	Immortal::Renderer2D::EndScene();
}

void OpenGLSample2D::OnGuiRender()
{
	static bool my_tool_active = true;
	static bool show_demo_window = false;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_FILE))
		{
			if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O"))
			{
				std::optional<std::string> filename = Immortal::FileDialogs::OpenFile(Immortal::FileDialogs::ImageFilter);
				if (filename.has_value())
				{
					mTexture = Immortal::Texture2D::Create(filename.value());
					mLuminance = 0.0f;
					mSquareColor = Immortal::Vector::Vector4(0.0f);
					mCameraController.SetZoomLevel(1.12f);
					mCameraController.Resize((float)Immortal::Application::App()->GetWindow().Width(), (float)Immortal::Application::App()->GetWindow().Height());
					mCameraController.Reset();
				}
			}
			if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
			if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_EXIT, "Ctrl+W")) { Immortal::Application::App()->Close(); }
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
		//ImGui::ColorEdit4(UNICODE8("选择颜色"), Vector::value_ptr(mSquareColor) + 0);

		ImGui::Columns(2);
		ImGui::Text(UNICODE8("红色"));
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);
		ImGui::SliderFloat("#########", &mSquareColor.r, -1, 1);
		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::SliderFloat(UNICODE8("绿色"), &mSquareColor.g, -1, 1);
		ImGui::SliderFloat(UNICODE8("蓝色"), &mSquareColor.b, -1, 1);
		ImGui::SliderFloat(UNICODE8("透明度"), &mSquareColor.a, -1, 1);
		ImGui::SliderFloat(UNICODE8("亮度"), &mLuminance, -1, 1);
		ImGui::Text(IMMORTAL_CONSTANT_STRING_RENDER_RATE, 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text(UNICODE8("图形设备：%s"), Immortal::Application::App()->GetWindow().Context().GraphicsRenderer());
		ImGui::Text(UNICODE8("驱动版本：%s"), Immortal::Application::App()->GetWindow().Context().DriverVersion());

		ImGui::End();
	}
}

void OpenGLSample2D::OnEvent(Immortal::Event & e)
{
	mCameraController.OnEvent(e);
}
