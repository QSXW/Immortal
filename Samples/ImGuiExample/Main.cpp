// Dear ImGui: standalone example application for Immortal Graphics API

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>

#define IMGUI_UNLIMITED_FRAME_RATE

#include "Graphics/LightGraphics.h"
#include "ImGui/imgui_impl_immortal.h"
using namespace Immortal;

URef<Queue> queue;
URef<Swapchain> swapchain;
URef<GPUEvent> pEvent;

uint32_t bufferCount = 3;
uint32_t syncPoint = 0;
uint32_t syncValues[3] = {};
bool applicationExit = false;

void OnEvent(Event &e)
{
    if (e.GetType() == Event::Type::WindowResize)
    {
        queue->WaitIdle(0xffffff);
        auto &resizeEvent = (WindowResizeEvent &)e;
        swapchain->Resize(resizeEvent.Width(), resizeEvent.Height());
    }
    if (e.GetType() == Event::Type::WindowClose)
    {
		applicationExit = true;
    }
}

int main(int, char **)
{
    LOG::Init();

    BackendAPI backendAPI = BackendAPI::Vulkan;

    // Create a window
	URef<Window> window = Window::CreateInstance("Immortal Graphics ImGui Example", 1920, 1080, backendAPI == BackendAPI::OpenGL ? WindowType::GLFW : WindowType::None);
	window->SetEventCallback(OnEvent);

    // Create physical device from GPU Id
	URef<Instance> instance = Instance::CreateInstance(backendAPI, window->GetType());

    // Create a logic device from the physical device
	URef<Device> device = instance->CreateDevice(0);

    // Create a graphics queue
    queue = device->CreateQueue(Queue::Type::Graphics);

    pEvent = device->CreateGPUEvent("RenderingEvent");

    URef<CommandBuffer> commandBuffers[3];
    for (size_t i = 0; i < 3; i++)
    {
        commandBuffers[i] = device->CreateCommandBuffer();
    }

    // Use the logic device to create a swapchain
    swapchain = device->CreateSwapchain(queue, window, Format::BGRA8, 3 /* buffer count */, SwapchainMode::VerticalSync);

    // Show the window. The window is not shown by default after it was created.
    window->Show();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplImmortal_Init(device, queue, swapchain, 3);

    decltype(&ImGui_ImplWin32_NewFrame) NewWindowFrame;
	decltype(&ImGui_ImplGlfw_Shutdown)  ShutDownWindow;
    if (window->GetType() == WindowType::Win32)
	{
		ImGui_ImplWin32_Init(window->GetBackendHandle());
		NewWindowFrame = ImGui_ImplWin32_NewFrame;
		ShutDownWindow = ImGui_ImplWin32_Shutdown;
	}
    else
    {
		ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)window->GetBackendHandle(), true);
		NewWindowFrame = ImGui_ImplGlfw_NewFrame;
		ShutDownWindow = ImGui_ImplGlfw_Shutdown;
    }

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	while (!applicationExit)
    {
        swapchain->PrepareNextFrame();

        io.DisplaySize.x = window->GetWidth();
        io.DisplaySize.y = window->GetHeight();

        // Start the Dear ImGui frame
        ImGui_ImplImmortal_NewFrame();
		NewWindowFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };

        pEvent->Wait(syncValues[syncPoint], 0xffffff);
        auto &commandBuffer = commandBuffers[syncPoint];
        commandBuffer->Begin();
        RenderTarget *renderTarget = swapchain->GetCurrentRenderTarget();
        commandBuffer->BeginRenderTarget(renderTarget, clear_color_with_alpha);
        ImGui_ImplImmortal_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        commandBuffer->EndRenderTarget();
        commandBuffer->End();

        CommandBuffer *ppCommandBuffer[] = { commandBuffer};
        GPUEvent      *ppGPUEvent[]      = { pEvent       };
		queue->Submit(ppCommandBuffer, 1, ppGPUEvent, 1, swapchain);

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            if (window->GetType() == WindowType::GLFW)
            {
				glfwMakeContextCurrent((GLFWwindow *)window->GetBackendHandle());
            }
        }

        syncValues[syncPoint] = pEvent->GetSyncPoint();
        SLROTATE(syncPoint, bufferCount);

        // Submit the swapchain to the queue for presenting
        queue->Present(swapchain);

        // Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		window->ProcessEvents();
    }

    queue->WaitIdle();

    ShutDownWindow();
    ImGui_ImplImmortal_Shutdown();
    ImGui::DestroyContext();

    for (auto &commandBuffer : commandBuffers)
    {
		commandBuffer.Reset();
    }

    // Cleanup
	queue.Reset();
	swapchain.Reset();
	pEvent.Reset();
	device.Reset();
	instance.Reset();

	LOG::Release();

    return 0;
}
