// Dear ImGui: standalone example application for Immortal Graphics API

#include <Immortal.h>
#include "ImGui/imgui_impl_immortal.h"
using namespace Immortal;

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#define IMGUI_UNLIMITED_FRAME_RATE

URef<Queue> queue;
URef<Swapchain> swapchain;
URef<GPUEvent> pEvent;

uint32_t bufferCount = 3;
uint32_t syncPoint = 0;
uint64_t syncValues[3] = {};
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
	Async::Init();

    // For more details about the rendering, please check HelloTriangleExample and HelloImGuiExample
    BackendAPI backendAPI = BackendAPI::Vulkan;

	URef<Window> window = Window::CreateInstance("Hello Video Player(d3d12va) - Immortal Graphics Example", 1920, 1080, backendAPI == BackendAPI::OpenGL ? WindowType::GLFW : WindowType::None);
	window->SetEventCallback(OnEvent);

	URef<Instance> instance = Instance::CreateInstance(backendAPI, window->GetType());

	URef<Device> device = instance->CreateDevice(0);
	Graphics::SetDevice(device);

    queue = device->CreateQueue(Queue::Type::Graphics);
	Graphics::Execute<SetQueueTask>(queue);

    pEvent = device->CreateGPUEvent("RenderingEvent");

    URef<CommandBuffer> commandBuffers[3];
    for (size_t i = 0; i < 3; i++)
    {
        commandBuffers[i] = device->CreateCommandBuffer();
    }
    swapchain = device->CreateSwapchain(queue, window, Format::BGRA8, 3 /* buffer count */, SwapchainMode::VerticalSync);

    window->Show();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplImmortal_Init(device, window, queue, swapchain, 3);

    decltype(&ImGui_ImplGlfw_NewFrame) NewWindowFrame;
	decltype(&ImGui_ImplGlfw_Shutdown)  ShutDownWindow;

#ifdef _WIN32
    if (window->GetType() == WindowType::Win32)
	{
		ImGui_ImplWin32_Init(window->GetBackendHandle());
		NewWindowFrame = ImGui_ImplWin32_NewFrame;
		ShutDownWindow = ImGui_ImplWin32_Shutdown;
	}
    else
#endif
    {
		ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)window->GetBackendHandle(), true);
		NewWindowFrame = ImGui_ImplGlfw_NewFrame;
		ShutDownWindow = ImGui_ImplGlfw_Shutdown;
    }

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	Timer timer;
    URef<VideoPlayerComponent> videoPlayerComponent;
	URef<SpriteRendererComponent> sprite;
	Ref<Texture> texture;

    Ref<AudioDevice> audioDevice;
	float progress = 0.0f;

    std::string filepath = "Video Player Window";
	while (!applicationExit)
    {
		auto deltaTime = timer.tick<Timer::Seconds>();
        pEvent->Wait(syncValues[syncPoint], 0xffffff);
        swapchain->PrepareNextFrame();

		ImGui_ImplImmortal_NewFrame();
		NewWindowFrame();
		ImGui::NewFrame();

		ImGui::Begin("Video Player Window");
        if (ImGui::Button("Open video file"))
        {
			auto path = FileDialogs::OpenFile();
            if (path.has_value())
            {
				filepath = path.value();
                filepath = "/media/qsxw/新加卷/DigitalMedia/MotionGraphics/Trailer/LOL/The Climb _ League of Legends.mp4";
                bool isPaused = false;
                if (audioDevice)
                {
					audioDevice->OnPauseDown();
					audioDevice->Reset();
					isPaused = true;
                }
                else
                {
					audioDevice = new AudioDevice;
                }

				texture = {};

				Ref<Vision::FFCodec> codec      = new Vision::FFCodec;
				Ref<Vision::FFCodec> audioCodec = new Vision::FFCodec;
				Ref<Demuxer>    demuxer = new Vision::FFDemuxer;
				demuxer->Open(filepath, codec, audioCodec);
				videoPlayerComponent = new VideoPlayerComponent{ demuxer, codec, audioCodec };
				sprite = new SpriteRendererComponent;

                if (isPaused)
                {
					audioDevice->OnPauseRelease();
                }

				audioDevice->SetCallBack(
				    [&](Picture &picture) {
					    if (videoPlayerComponent)
					    {
						    Picture audioFrame = videoPlayerComponent->GetAudioFrame();
						    if (audioFrame)
						    {
							    videoPlayerComponent->PopAudioFrame();
						    }
						    picture = audioFrame;
					    }
				    });
            }
        }
        Graphics::Execute<AsyncTask>(AsyncTaskType::BeginRecording);
        if (videoPlayerComponent)
		{
			auto animator = videoPlayerComponent->GetAnimator();
			if (animator->TryMoveToNextFrame(deltaTime))
			{
				Picture picture = videoPlayerComponent->GetPicture();
				if (picture)
				{
					videoPlayerComponent->PopPicture();
					sprite->UpdateSprite(picture);

					texture = sprite->Sprite;

					auto current = picture.GetTimestamp();
					progress = (float) current / animator->TotalFrames();
				}
			}
        }

		Graphics::Execute<AsyncTask>(AsyncTaskType::EndRecording);
		Graphics::Execute<AsyncTask>(AsyncTaskType::Submiting);

        auto windowSize = ImGui::GetWindowSize();
        if (texture)
        {
			ImGui::SameLine();
			ImGui::Text("%s", filepath.c_str());
            ImVec2 size = {};
			constexpr float xfactor = 1280.0;
			constexpr float yfactor = 720.0;
			auto width  = texture->GetWidth();
			auto height = texture->GetHeight();

            auto aspectRatio = height / (float)width;
            if (width > height)
            {
				size.x = windowSize.x;
				size.y = size.x * aspectRatio;
            }
            else
            {
				size.y = yfactor;
				size.x = yfactor / height * width;
            }

		    ImGui::Image((ImTextureID)(uint64_t)texture.Get(), size);
        }

        static float f;
		static float lastProgress;
		f = progress;
		ImGui::SetNextItemWidth(windowSize.x);
        if (ImGui::SliderFloat("###", &f, 0, 1.0f))
        {
			if (videoPlayerComponent && std::abs(f - progress) > 0.01f && f != lastProgress)
            {
				progress = f;
				Animator *animator = videoPlayerComponent->GetAnimator();
				audioDevice->OnPauseDown();
				videoPlayerComponent->Seek(animator->TotalSeconds() * progress, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
				audioDevice->Reset();
				audioDevice->OnPauseRelease();
            }
			progress = f;
			lastProgress = f;
        }

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

        io.DisplaySize.x = window->GetWidth();
        io.DisplaySize.y = window->GetHeight();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Render();
		const float clearValue[4] = { clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w };

        auto &commandBuffer = commandBuffers[syncPoint];
        commandBuffer->Begin();
        RenderTarget *renderTarget = swapchain->GetCurrentRenderTarget();
		commandBuffer->BeginRenderTarget(renderTarget, clearValue);
        ImGui_ImplImmortal_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        commandBuffer->EndRenderTarget();
        commandBuffer->End();

        CommandBuffer *ppCommandBuffer[] = { commandBuffer};
        GPUEvent      *ppGPUEvent[]      = { pEvent       };
		queue->Submit(ppCommandBuffer, 1, ppGPUEvent, 1, swapchain);

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

        queue->Present(swapchain);
		window->ProcessEvents();
    }

    queue->WaitIdle();

	videoPlayerComponent.Reset();
	sprite.Reset();
	audioDevice.Reset();
	Graphics::Release();

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
