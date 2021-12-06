#include "VulkanSample.h"

VulkanLayer::VulkanLayer()
{

}

void VulkanLayer::OnAttach()
{
    primary = Render::Preset()->WhiteTexture;
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
        if (ImGui::BeginMenu(WordsMap::Get("menu")))
        {
            if (ImGui::MenuItem(WordsMap::Get("open"), "Ctrl + O"))
            {
                this->LoadObject();
            }
            if (ImGui::MenuItem(WordsMap::Get("save"), "Ctrl + S"))
            {
            
            }
            if (ImGui::MenuItem(WordsMap::Get("save_as"), "Ctrl+AWordsMap::Get<+S"))
            {

            }
            if (ImGui::MenuItem(WordsMap::Get("exit"), "Ctrl + W"))
            { 
                Application::App()->Close();
            }
            if (ImGui::MenuItem(WordsMap::Get("open"), "Ctrl + W"))
            {
            
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(WordsMap::Get("edit")))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(WordsMap::Get("view")))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(WordsMap::Get("graphics")))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(WordsMap::Get("help")))
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

        ImGui::Begin(WordsMap::Get("debug_tools"));

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
        primary = Render::Create<Texture>(res.value());
        return true;
    }

    return false;
}

void VulkanLayer::OnUpdate()
{
    
}
