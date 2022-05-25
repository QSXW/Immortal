#pragma once

#include "Immortal.h"

using namespace Immortal;

class RenderLayer : public Layer
{
public:
	RenderLayer() :
		Layer{ "OpenGLSample2D" },
		frame{ new WFrame },
		image{ new WImage{ frame } }
	{
		camera.SetViewportSize({ 1920.0f, 1080.0f });
		frame->Text("Text Frame").Color(ImVec4{ 1.0f, 0.5f, 0.5f, 1.0f });
	}

	~RenderLayer()
	{

	}

	void OnAttach()
	{
		renderTarget = Render::Create<RenderTarget>(RenderTarget::Description{ { 1920, 1080 }, { {  Format::RGBA8, Wrap::Clamp, Filter::Bilinear }, { Format::Depth } } });
		texture = Render::Preset()->Textures.White;

		image->Width(1920).Height(1080).Descriptor(*texture);
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

		Render2D::DrawRect({ 0.0f, 0.0f }, { texture->Ratio() * 2.0, 2.0f }, texture, 1.0f, Vector::Color(1.f));
		Render2D::SetColor(color, luminance);
		Render2D::EndScene();

		Render::Begin(renderTarget, camera);
		Render2D::BeginScene(camera);
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };// { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
				Render2D::DrawRect({ x, y }, { 0.45f, 0.45f }, color);
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

		frame->Render();
		LOG::DEBUG("Width:{}, Height:{}", frame->Width(), frame->Height());
		offline.OnUpdate(renderTarget.Get(), [] {});
	}

	void OnEvent(Event &e)
	{
		if (e.GetType() == MouseScrolledEvent::GetStaticType())
		{
			camera.OnMouseScrolled(dynamic_cast<MouseScrolledEvent &>(e));
		}
	}

private:
	Ref<RenderTarget> renderTarget;

	Ref<Shader> shader;

	Ref<Pipeline> pipeline;

	Ref<Texture> texture;
	
	OrthographicCamera camera;

	Viewport offline{ WordsMap::Get("Offline Render") };

	float luminance{ 0 };

	Vector4 color{ 0.0f, 0.0f, 0.0f, 0.0f };

	Ref<WFrame> frame;

	Ref<WImage> image;
};
