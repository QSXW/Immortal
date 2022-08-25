#include "GuiLayer.h"

#include "Framework/Application.h"
#include "Render/Render.h"
#include "RenderContext.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Immortal
{
namespace Vulkan
{

GuiLayer::GuiLayer(RenderContext *context) :
    context{ context }
{
    SLASSERT(context && "Render Context could not be NULL.");
}

GuiLayer::~GuiLayer()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void GuiLayer::OnAttach()
{
    Super::OnAttach();

    Application *app = Application::App();
    ImGui_ImplGlfw_InitForVulkan(rcast<GLFWwindow *>(app->GetNativeWindow()), true);

    auto device = context->GetAddress<Device>();
    descriptorPool.reset(new DescriptorPool{ device, Limit::PoolSize });

    ImGui_ImplVulkan_InitInfo initInfo{};
    auto queue = context->GetAddress<Queue>();

    initInfo.Instance        =  context->Get<Instance&>();
    initInfo.PhysicalDevice  =  context->Get<PhysicalDevice&>();
    initInfo.Device          = *device;
    initInfo.Queue           = *queue;
    initInfo.QueueFamily     = queue->Get<Queue::FamilyIndex>();
    initInfo.PipelineCache   = VK_NULL_HANDLE;
    initInfo.DescriptorPool  = *descriptorPool;
    initInfo.Allocator       = nullptr;
    initInfo.MinImageCount   = Swapchain::MaxFrameCount;
    initInfo.ImageCount      = context->FrameSize();
    initInfo.CheckVkResultFn = &Check;

    ImGui_ImplVulkan_LoadFunctions(nullptr, nullptr);

    auto renderTarget = context->GetAddress<RenderTarget>();
    if (ImGui_ImplVulkan_Init(&initInfo, renderTarget->Get<RenderPass>()))
    {
        LOG::INFO("Initialized GUI with success");
    }

    device->Transfer([&](auto *cmdbuf) -> void {
        ImGui_ImplVulkan_CreateFontsTexture(*cmdbuf);
        });
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void GuiLayer::OnDetach()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    Super::OnDetach();
}

void GuiLayer::OnEvent(Event &e)
{
    Super::OnEvent(e);
    if (e.GetType() == Event::Type::WindowResize)
    {
        auto resize = dcast<WindowResizeEvent *>(&e);
        ImGuiIO     &io  = ImGui::GetIO();
        io.DisplaySize = ImVec2{ (float)resize->Width(), (float)resize->Height() };
    }
}

void GuiLayer::OnGuiRender()
{
    Super::OnGuiRender();
}

void GuiLayer::Begin()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    Super::Begin();
}

void GuiLayer::End()
{
    Super::End();

    ImGuiIO &io  = ImGui::GetIO();

    const auto &[width, height] = context->Get<Extent2D>();

    io.DisplaySize = ImVec2{ (float)width, (float)height };

    ImDrawData *primaryDrawData = ImGui::GetDrawData();

    VkClearValue clearValues[] = {
        {{ .40f, 0.45f, 0.60f, 0.0f }},
        {{  .0f,  .0f,    .0f, 0.0f }}
    };

    context->Record([&](CommandBuffer *cmdbuf) -> void {
        auto renderTarget = context->GetAddress<RenderTarget>();
        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext                    = nullptr;
        beginInfo.renderPass               = renderTarget->Get<RenderPass>();
        beginInfo.framebuffer              = renderTarget->Get<Framebuffer>();
        beginInfo.clearValueCount          = 2;
        beginInfo.pClearValues             = clearValues;
        beginInfo.renderArea.extent.width  = io.DisplaySize.x;
        beginInfo.renderArea.extent.height = io.DisplaySize.y;
        beginInfo.renderArea.offset        = { 0, 0 };

        cmdbuf->BeginRenderPass(&beginInfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(primaryDrawData, *cmdbuf);

        cmdbuf->EndRenderPass();
        });

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

}
}
