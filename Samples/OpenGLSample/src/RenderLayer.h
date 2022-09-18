#pragma once

#include "Immortal.h"

using namespace Immortal;

class RenderLayer : public Layer
{
public:
	RenderLayer() :
		Layer{ "OpenGLSample2D" },
	    window{new WWindow },
		frame{ new WFrame },
		image{ new WImage{ frame } },
	    renderViewportFrame{new WFrame},
	    renderViewport{new WImage{ }}
	{
		window->Wrap({
			frame
				->Text("Text Frame")
				->Color({ 1.0f, 0.5f, 0.5f, 1.0f })
				->Wrap({
				image
					->Width(1920)
					->Height(1080)
				}),
			renderViewportFrame
				->Text("Offline Render")
				->Wrap({
				renderViewport,
				}),
		});

		camera.SetViewportSize({ 1920.0f, 1080.0f });
	}

	~RenderLayer()
	{

	}

	void OnAttach()
	{
		renderTarget = Render::Create<RenderTarget>(RenderTarget::Description{ { 1920, 1080 }, { {  Format::RGBA8, Wrap::Clamp, Filter::Bilinear }, { Format::Depth32F } } });
		texture = Render::Preset()->Textures.White;

		renderTarget->Set(Colour{ 0.10980392f, 0.10980392f, 0.10980392f, 1 });
		Render2D::Setup(renderTarget);

		image->Descriptor(*texture);
	}

	void OnUpdate()
	{
		camera.OnUpdate();

		Render::Begin(renderTarget, camera);

		Render2D::BeginScene(camera);

		static float rotation = 0.0f;
		rotation += Time::DeltaTime * 50.0f;

		Render2D::DrawRect({ 0.0f, 0.0f }, { texture->Ratio() * 2.0, 2.0f }, texture, 1.0f, Vector::Color(1.f));
		Render2D::EndScene();

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

		renderViewport->Descriptor(*renderTarget);
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

	Ref<Pipeline> pipeline;

	Ref<Texture> texture;

	URef<WWindow> window;

	Ref<WFrame> frame;

	Ref<WImage> image;

	URef<WFrame> renderViewportFrame;

	URef<WImage> renderViewport;
	
	OrthographicCamera camera;
};
