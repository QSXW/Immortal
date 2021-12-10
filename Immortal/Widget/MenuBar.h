#pragma once

#include "Widget.h"

#include "ImGui/GuiLayer.h"

namespace Widget
{

using namespace Immortal;

class MenuBar : public Super
{
public:
    MenuBar()
    {
    
    }

    MenuBar(const std::string &name) :
        Super(name)
    {

    }

    template<class T>
    void OnUpdate(T func)
    {
        ImGui::BeginMenuBar();
        func();
        ImGui::EndMenuBar();
    }
};

}
