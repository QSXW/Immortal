#pragma once

#include "Immortal.h"

using namespace Immortal;

class RenderLayer : public Layer
{
public:
    RenderLayer() :
        eventSink{ this }
    {
        eventSink.Listen(&RenderLayer::OnKeyPressed, Event::Type::KeyPressed);
        texture = Render::Preset()->WhiteTexture;
    }

    ~RenderLayer()
    {

    }

    void OnGuiRender()
    {
        viewport.OnUpdate(texture);
        auto gui = Application::App()->GetGuiLayer();
        gui->BlockEvent(false);
    }
    
    bool LoadObject()
    {
        auto res = FileDialogs::OpenFile(FileDialogs::ImageFilter);
        if (res.has_value())
        {
            auto image = Render::Create<Texture>(res.value());
            texture = image;
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
        LOG::INFO(e);
        eventSink.Dispatch(e);
    }

private:
    std::shared_ptr<Texture> texture;

    Widget::Viewport viewport{ "Texture Preview " };

    EventSink<RenderLayer> eventSink;
};

class D3D12Sample : public Application
{
public:
    D3D12Sample() : Application({ U8("D3D12 Sample (Graphics API: DirectX 12)"), 1920, 1080 })
    {
        PushLayer(new RenderLayer());
    }

    ~D3D12Sample()
    {

    }

private:

};
