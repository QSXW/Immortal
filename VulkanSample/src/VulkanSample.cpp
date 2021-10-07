#include "VulkanSample.h"
#include "Platform/Vulkan/Common.h"

VulkanLayer::VulkanLayer()
{

}

void VulkanLayer::OnAttach()
{
    primary = Render::CreateTexture("null.jpg");
}

void VulkanLayer::OnDetach()
{
    
}

void VulkanLayer::OnGuiRender()
{
    static bool p_open             = true;
    static bool optionalPadding    = false;
    static bool optionalFullScreen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    if (optionalFullScreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }
    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    {
        window_flags |= ImGuiWindowFlags_NoBackground;
    }
    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!optionalPadding)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }
    ImGui::Begin("DockSpace Demo", &p_open, window_flags);

    if (!optionalPadding)
    {
        ImGui::PopStyleVar();
    }
    if (optionalFullScreen)
    {
        ImGui::PopStyleVar(2);
    }

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    float minWindowSize = style.WindowMinSize.x;

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    style.WindowMinSize.x = minWindowSize;

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu(lt["menu"].c_str()))
        {
            if (ImGui::MenuItem(lt["open"].c_str(), "Ctrl+O"))
            {

            }

            if (ImGui::MenuItem(U8("新建场景")))
            {

            }
            if (ImGui::MenuItem(U8("保存场景")))
            {

            }
            if (ImGui::MenuItem(U8("加载场景")))
            {

            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S"))
            {
            
            }
            if (ImGui::MenuItem(U8("保存为..."), "Ctrl+Alt+S"))
            {

            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_EXIT, "Ctrl+W"))
            { 
                Application::App()->Close();
            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W"))
            {
            
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_EDIT))
        {
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O"))
            {
            
            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S"))
            {

            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W"))
            { 
            
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_VIEW))
        {
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O"))
            {
            
            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S"))
            {
            
            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W"))
            {

            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_GRAPHICS))
        {
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O"))
            {
            
            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S"))
            {

            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W"))
            {

            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_HELP))
        {
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O"))
            {

            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S"))
            {

            }
            if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W"))
            {
            
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (Settings.showDemoWindow)
            ImGui::ShowDemoWindow(&Settings.showDemoWindow);

    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoTitleBar);

        auto [x, y] = ImGui::GetContentRegionAvail();

        ImGui::Image(
            (ImTextureID)(primary->Handle()),
            { x, y }
            // { ncast<float>(primary->Width()), ncast<float>(primary->Height()) }
            );

        ImGui::End();
        ImGui::PopStyleVar();
    }

    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Debug Tools");

        ImGui::Text("This is some useful text.");
        ImGui::Checkbox("Demo Window", &Settings.showDemoWindow);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*)&Settings.clearColor); 

        static const char *api = Render::Api();
        static const char *videoCard = Render::GraphicsRenderer();

        static char buf[255] = { 0 };
        sprintf(buf, "Immortal Editor (Graphics API: %s, Physical Device: %s) %.3f ms/frame (%.1f FPS)", 
            api, videoCard, 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        std::string title{ buf };
        Application::SetTitle(title);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::End(); /* Dock place */
}

void VulkanLayer::OnUpdate()
{
    
}
