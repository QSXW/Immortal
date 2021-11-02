#include "impch.h"
#include "GuiLayer.h"

#include "Framework/Application.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <Render/Render.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Immortal
{
namespace Vulkan
{

GuiLayer::GuiLayer(RenderContext::Super *context) :
    context{ dcast<RenderContext *>(context) }
{
    device     = this->context->Get<Device *>();
    renderPass = this->context->Get<RenderPass *>();
    SLASSERT(context && "Render Context could not be NULL.");
}

GuiLayer::~GuiLayer()
{

}

void GuiLayer::OnAttach()
{
    Super::OnAttach();

    Application *app = Application::App();
    ImGui_ImplGlfw_InitForVulkan(rcast<GLFWwindow *>(app->GetNativeWindow()), true);

    auto &swapchain = context->Get<Swapchain &>();
    auto format = swapchain.Get<VkFormat>();

    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    constexpr int size = SLLEN(poolSizes);
    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets       = 1000 * size;
    createInfo.poolSizeCount = size;
    createInfo.pPoolSizes    = poolSizes;

    Check(vkCreateDescriptorPool(device->Handle(), &createInfo, nullptr, &descriptorPool));

    ImGui_ImplVulkan_InitInfo initInfo{};
    auto queue = context->Get<Queue*>();

    initInfo.Instance        = context->Get<VkInstance>();
    initInfo.PhysicalDevice  = context->Get<VkPhysicalDevice>();
    initInfo.Device          = *device;
    initInfo.QueueFamily     = queue->Get<Queue::FamilyIndex>();
    initInfo.Queue           = *queue;
    initInfo.PipelineCache   = pipelineCache;
    initInfo.DescriptorPool  = descriptorPool;
    initInfo.Allocator       = nullptr;
    initInfo.MinImageCount   = 3;
    initInfo.ImageCount      = context->Get<RenderContext::Frames&>().size();
    initInfo.CheckVkResultFn = &Check;

    ImGui_ImplVulkan_LoadFunctions(nullptr, nullptr);
    if (ImGui_ImplVulkan_Init(&initInfo, renderPass->Handle()))
    {
        LOG::INFO("Initialized GUI with success");
    }
    
    device->Upload([&](auto *cmdbuf) -> void {
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
    if (e.Type() == EventType::WindowResize)
    {
        auto resize = dcast<WindowResizeEvent *>(&e);
        ImGuiIO     &io  = ImGui::GetIO();
        io.DisplaySize = ImVec2{ (float)resize->Width(), (float)resize->Height() };
    }
}

void GuiLayer::OnGuiRender()
{

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

    auto &[width, height] = context->Get<Extent2D>();

    io.DisplaySize = ImVec2{ (float)width, (float)height };

    ImDrawData* primaryDrawData = ImGui::GetDrawData();

    VkClearValue clearValue[] = {
        {{ .40f, 0.45f, 0.60f, 0.0f }},
        {{  .0f,  .0f,    .0f, 0.0f }}
    };

    context->Record(CommandBuffer::Usage::OneTimeSubmit, [&](auto *cmdbuf, auto *framebuffer) -> void {
        VkRenderPassBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext                    = nullptr;
        beginInfo.renderPass               = *renderPass;
        beginInfo.framebuffer              = *framebuffer;
        beginInfo.renderArea.extent.width  = io.DisplaySize.x;
        beginInfo.renderArea.extent.height = io.DisplaySize.y;
        beginInfo.clearValueCount          = 2;
        beginInfo.pClearValues             = clearValue;
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
