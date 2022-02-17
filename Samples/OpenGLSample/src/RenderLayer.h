#pragma once

#include "Immortal.h"

using namespace Immortal;

class RenderLayer : public Layer
{
public:
	RenderLayer() :
		Layer{ "OpenGLSample2D" }
	{
		camera.SetViewportSize({ 1920.0f, 1080.0f });
	}

	~RenderLayer()
	{

	}

	void OnAttach()
	{
		renderTarget.reset(Render::Create<RenderTarget>(RenderTarget::Description{ { 1920, 1080 }, { {  Format::RGBA8, Wrap::Clamp, Filter::Bilinear }, { Format::Depth } } }));
		texture = Render::Preset()->Textures.White;

		renderTarget->Set(Colour{ 0.10980392f, 0.10980392f, 0.10980392f, 1 });
		Render2D::Setup(renderTarget);
	}

	void OnDetach()
	{

	}

	void OnUpdate()
	{
		camera.OnUpdate();
		Render2D::BeginScene(camera);

		static float rotation = 0.0f;
		rotation += Application::App()->DeltaTime() * 50.0f;

		Render2D::DrawQuad({ 0.0f, 0.0f }, { texture->Ratio() * 2.0, 2.0f }, texture, 1.0f, Vector::Color(1.f));
		Render2D::SetColor(color, luminance);
		Render2D::EndScene();

		Render::Begin(renderTarget, camera);
		Render2D::BeginScene(camera);
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
		static bool show_demo_window = false;

		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);
		}


		offline.OnUpdate(renderTarget, []{});
	}

	void OnEvent(Event &e)
	{
		LOG::INFO(e);
		// camera.OnMouseScrolled(e);
	}

private:
	std::shared_ptr<RenderTarget> renderTarget;

	std::shared_ptr<Shader> shader;

	std::shared_ptr<Pipeline> pipeline;

	std::shared_ptr<Texture> texture;
	
	OrthographicCamera camera;

	Widget::Viewport offline{ WordsMap::Get("offlineRender") };

	float luminance{ 0 };

	Vector4 color{ 0.0f, 0.0f, 0.0f, 0.0f };
};
