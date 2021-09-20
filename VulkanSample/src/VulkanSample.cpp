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

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &Settings.showDemoWindow);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &Settings.showAnotherWindow);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&Settings.clearColor); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    if (Settings.showAnotherWindow)
    {
        ImGui::Begin("Another Window", &Settings.showAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            Settings.showAnotherWindow = false;
        ImGui::End();
    }
}

void VulkanLayer::OnUpdate()
{
    
}
