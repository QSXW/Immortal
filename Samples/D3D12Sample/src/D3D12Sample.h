#pragma once

#include "Immortal.h"

using namespace Immortal;

class WVec3Control : public Widget
{
public:
    WVec3Control(Widget *parent, Ref<RenderTarget> renderTarget, TransformComponent &transformComponent) :
        Widget{ parent }
    {
		Connect([&] {
			ImGui::ColorEdit4("Clear Color", rcast<float *>(renderTarget->ClearColor()));
			Immortal::UI::DrawVec3Control("Position", transformComponent.Position);
			Immortal::UI::DrawVec3Control("Rotation", transformComponent.Rotation);
			Immortal::UI::DrawVec3Control("Scale", transformComponent.Scale);
		});
    }
};

class RenderLayer : public Layer
{
public:
    RenderLayer() :
        eventSink{ this },
	    window{new WWindow},
        viewportFrame{ new WFrame },
	    controlPanel{ new WFrame },
	    viewport{new WImage{viewportFrame}},
	    renderViewportFrame{ new WFrame },
	    renderViewport{new WImage{renderViewportFrame}}
    {
		window->Wrap({
            viewportFrame
		        ->Text("Render Target"),
		    renderViewportFrame,
		    controlPanel
                ->Text("Contorl Panel")
        });

        eventSink.Listen(&RenderLayer::OnKeyPressed, Event::Type::KeyPressed);
        texture = Render::Preset()->Textures.White;
        texture2 = Render::Preset()->Textures.White;
        pipeline = Render::Create<GraphicsPipeline>(Render::GetShader("Basic"));
        renderTarget = Render::Create<RenderTarget>(RenderTarget::Description{ Vector2{ 1920, 1080 }, { { Format::RGBA8 }, { Format::Depth32F } }});

        auto &triangle = DataSet::Classic::Triangle;
        pipeline->Set(Render::Create<Buffer>(triangle.Vertices(), Buffer::Type::Vertex) );
        pipeline->Set(Render::Create<Buffer>(triangle.Indices(), Buffer::Type::Index) );

        pipeline->Set(triangle.Description());
        pipeline->Reconstruct(renderTarget);

        uniformBuffer = Render::Create<Buffer>(sizeof(ubo), 0);
        pipeline->AllocateDescriptorSet(0);
        pipeline->Bind("ubo", uniformBuffer);
        pipeline->Bind(texture, 1);

        camera.SetPerspective(90.0f);
        transformComponent.Position.z = -0.5f;

        vec3Control = new WVec3Control{controlPanel, renderTarget, transformComponent};
    }

    ~RenderLayer()
    {

    }

    void OnUpdate()
    {
        Render::Begin(renderTarget, camera);
        {
            Render2D::BeginScene(camera);
            Render2D::DrawRect(transformComponent.Transform(), texture);
            auto newTransform = transformComponent;
            newTransform.Position -= 0.5f;
            Render2D::DrawRect(newTransform, texture2);
            Render2D::EndScene();
        }
        Render::End();

        Vector2 viewportSize = viewportFrame->Size();
		if ((viewportSize.x != renderTarget->Width() || viewportSize.y != renderTarget->Height()) &&
		    (viewportSize.x != 0 && viewportSize.y != 0))
		{
			renderTarget->Resize(viewportSize);
			camera.SetViewportSize(renderTarget->ViewportSize());
		}

		ubo.modeTransform = transformComponent;
		ubo.viewProjection = camera.ViewProjection();
		uniformBuffer->Update(sizeof(ubo), &ubo);

		renderViewport->Descriptor(*renderTarget);
		viewport->Descriptor(*texture);
  
        auto gui = Application::App()->GetGuiLayer();
		gui->BlockEvent(false);
    }
    
    bool LoadObject()
    {
        auto res = FileDialogs::OpenFile(FileFilter::Image);
        if (res.has_value())
        {
            texture = Render::Create<Texture>(res.value());
            return true;
        }

        return false;
    }

    bool OnKeyPressed(KeyPressedEvent &e)
    {
        if (e.RepeatCount() > 0)
        {
            return false;
        }

        bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
        bool shift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);
        switch (e.GetKeyCode())
        {
        case KeyCode::L:
            if (control || shift)
            {
                LoadObject();
            }
            break;

        case KeyCode::W:
            if (control || shift)
            {
                Application::App()->Close();
            }
            break;

        case KeyCode::N:
        default:
            return false;
        }
        return true;
    }

    virtual void OnEvent(Event &e) override
    {
        LOG::INFO("{}", e);
        eventSink.Dispatch(e);
    }

private:
    Ref<Texture> texture;
    Ref<Texture> texture2;

    URef<WWindow> window;
	URef<WFrame> viewportFrame;
    URef<WImage> viewport;

    URef<WFrame> renderViewportFrame;
	URef<WImage> renderViewport;

    URef<WFrame> controlPanel;
	URef<WVec3Control> vec3Control;

    EventSink<RenderLayer> eventSink;

    Ref<RenderTarget> renderTarget;
    Ref<GraphicsPipeline> pipeline;

    SceneCamera camera;
    TransformComponent transformComponent;

    struct
    {
        Matrix4 viewProjection;
        Matrix4 modeTransform;
    } ubo;

    Ref<Buffer> uniformBuffer;
};

class D3D12Sample : public Application
{
public:
    D3D12Sample() : Application({ "D3D12 Sample", 1920, 1080 })
    {
        PushLayer(new RenderLayer());
    }

    ~D3D12Sample()
    {

    }

private:

};
