#include "VulkanSample.h"
#include "Platform/Vulkan/Common.h"

#include "RF.h"
#include "BMP.h"
#include "Framework/ThreadManager.h"

VulkanLayer::VulkanLayer()
{

}

void VulkanLayer::OnAttach()
{
    primary = Render::CreateTexture("");
}

void VulkanLayer::OnDetach()
{
    
}

void VulkanLayer::OnGuiRender()
{
    auto *gui = Application::App()->GetGuiLayer();
    gui->UpdateTheme();

    ImGui::PushFont(gui->Bold());
    menuBar.OnUpdate([=]() -> void {
        if (ImGui::BeginMenu(lt["menu"]))
        {
            if (ImGui::MenuItem(lt["open"], "Ctrl + O"))
            {
                this->LoadObject();
            }
            if (ImGui::MenuItem(lt["save"], "Ctrl + S"))
            {
            
            }
            if (ImGui::MenuItem(lt["save_as"], "Ctrl+Alt+S"))
            {

            }
            if (ImGui::MenuItem(lt["exit"], "Ctrl + W"))
            { 
                Application::App()->Close();
            }
            if (ImGui::MenuItem(lt["open"], "Ctrl + W"))
            {
            
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(lt["edit"]))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(lt["view"]))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(lt["graphics"]))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(lt["help"]))
        {
            ImGui::EndMenu();
        }
        });
    ImGui::PopFont();

    if (Settings.showDemoWindow)
    {
        ImGui::ShowDemoWindow(&Settings.showDemoWindow);
    }

    viewport.OnUpdate(primary);

    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin(lt["debug_tools"]);

        static char title[128] = { 0 };
        sprintf(
            title,
            "Immortal Editor (Graphics API: %s, Physical Device: %s) %.3f ms/frame (%.1f FPS)", 
            Render::Api(),
            Render::GraphicsRenderer(),
            1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate
           );
        Application::SetTitle(title);
        ImGui::End();
    }
}

bool VulkanLayer::LoadObject()
{
    auto res = FileDialogs::OpenFile(FileDialogs::ImageFilter);
    if (res.has_value())
    {
        primary = Render::CreateTexture(res.value());
        return true;
    }

    return false;
}

void VulkanLayer::OnUpdate()
{
    
}
