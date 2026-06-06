#include "Widget.h"
#include "ImGui/GuiLayer.h"

namespace Immortal
{

std::unordered_map<std::string, Widget *> Widget::Identify2WidgetTracker;
std::unordered_map<Widget *, std::string> Widget::Widget2IdentifyTracker;

WWindow::WWindow()
{
    GuiLayer::Inject2Dockspace(this);
}

}
