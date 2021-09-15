#pragma once

#include <imgui.h>

#include "Framework/Layer.h"

#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

namespace Immortal
{
class IMMORTAL_API GuiLayer : public Layer
{
public:
    template <class... A>
    static GuiLayer *Create(A &&... args);

public:
    GuiLayer();
    ~GuiLayer();

    void OnUpdate() { };
    virtual void OnAttach() override;
    virtual void OnEvent(Event &e) override;
    virtual void OnGuiRender() override;

    inline virtual void GuiLayer::OnDetach() override
    {
        ImGui::DestroyContext();
    }

    inline void GuiLayer::Begin()
    {
        ImGui::NewFrame();
    }

    inline void GuiLayer::End()
    {
        ImGui::Render();
    }

    void BlockEvent(bool block) { blockEvents = block;  }
    void SetTheme();

private:
    bool blockEvents = true;
    float time = 0.0f;
};

using SuperGuiLayer = GuiLayer;
}
