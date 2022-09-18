#include "Widget.h"
#include "ImGui/GuiLayer.h"

namespace Immortal
{

std::unordered_map<std::string, Widget *> Widget::WidgetTracker;

WWindow::WWindow()
{
    GuiLayer::Inject2Dockspace(this);
}

}
