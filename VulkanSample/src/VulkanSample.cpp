#include "VulkanSample.h"
#include "Platform/Vulkan/Common.h"

VulkanLayer::VulkanLayer()
{

}

void VulkanLayer::OnAttach()
{
    
}

void VulkanLayer::OnDetach()
{

}

void VulkanLayer::OnGuiRender()
{
    if (Settings.showDemoWindow)
            ImGui::ShowDemoWindow(&Settings.showDemoWindow);

    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Debug Tools");

        ImGui::Text("This is some useful text.");
        ImGui::Checkbox("Demo Window", &Settings.showDemoWindow);
        ImGui::Checkbox("Another Window", &Settings.showAnotherWindow);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*)&Settings.clearColor); 

        if (ImGui::Button("Button"))
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        static char buf[255] = { 0 };
        sprintf(buf, "Vulkan Sample (Graphics API: Vulkan) %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        std::string &title = std::string(buf);
        Application::SetTitle(title);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    if (Settings.showAnotherWindow)
    {
        ImGui::Begin("Another Window", &Settings.showAnotherWindow);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            Settings.showAnotherWindow = false;
        ImGui::End();
    }
}

void VulkanLayer::OnUpdate()
{
    
}
