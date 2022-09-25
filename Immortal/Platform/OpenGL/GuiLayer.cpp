#include "GuiLayer.h"

#include "Framework/Application.h"
#include "Common.h"

#define IMGUI_IMPL_OPENGL_LOADER_GL3W
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

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

void GuiLayer::Begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    Super::__Begin();
}

void GuiLayer::End()
{
    Super::__End();

    ImGuiIO &io = ImGui::GetIO();
    Application *app = Application::App();
    io.DisplaySize = ImVec2((float)app->Width(), (float)app->Height());

    Submit([] {
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	});

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
		Submit([] {
			GLFWwindow *backCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backCurrentContext);
		});
    }
}
}
}
