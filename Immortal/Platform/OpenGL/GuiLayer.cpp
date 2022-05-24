#include "GuiLayer.h"

#include "Framework/Application.h"
#include "Common.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Immortal
{
namespace OpenGL
{

GuiLayer::GuiLayer(RenderContext::Super *context)
{
    (void)context;
}

GuiLayer::~GuiLayer()
{

}

void GuiLayer::OnAttach()
{
    Super::OnAttach();
    Application *app = Application::App();

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)app->GetWindow().GetNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void GuiLayer::OnDetach()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    Super::OnDetach();
}

void GuiLayer::OnEvent(Event &e)
{

}

void GuiLayer::OnGuiRender()
{
    Super::OnGuiRender();
}

void GuiLayer::Begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    Super::Begin();
}

void GuiLayer::End()
{
    Super::End();

    ImGuiIO &io = ImGui::GetIO();
    Application *app = Application::App();
    io.DisplaySize = ImVec2((float)app->Width(), (float)app->Height());

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backCurrentContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backCurrentContext);
    }
}
}
}
