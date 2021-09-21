#include "impch.h"
#include "GuiLayer.h"

#include "Framework/Application.h"
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

GuiLayer::GuiLayer(RenderContext::Super *context) :
    context{ dcast<RenderContext *>(context) }
{
    device     = &this->context->Get<Device>();
    renderPass = &this->context->Get<RenderPass>();
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

    constexpr int size = SL_ARRAY_LEN(poolSizes);
    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets       = 1000 * size;
    createInfo.poolSizeCount = size;
    createInfo.pPoolSizes    = poolSizes;

    Check(vkCreateDescriptorPool(device->Handle(), &createInfo, nullptr, &descriptorPool));

    ImGui_ImplVulkan_InitInfo initInfo{};
    auto &queue = context->Get<Queue*>();

    initInfo.Instance        = context->Get<VkInstance>();
    initInfo.PhysicalDevice  = context->Get<VkPhysicalDevice>();
    initInfo.Device          = device->Handle();
    initInfo.QueueFamily     = queue->Get<Queue::FamilyIndex>();
    initInfo.Queue           = queue->Handle();
    initInfo.PipelineCache   = pipelineCache;
    initInfo.DescriptorPool  = descriptorPool;
    initInfo.Allocator       = nullptr;
    initInfo.MinImageCount   = 2;
    initInfo.ImageCount      = context->Get<RenderContext::Frames>().size();
    initInfo.CheckVkResultFn = &Check;

    ImGui_ImplVulkan_LoadFunctions(nullptr, nullptr);
    if (ImGui_ImplVulkan_Init(&initInfo, renderPass->Handle()))
    {
        LOG::INFO("Initialized GUI with success");
    }

    commandBuffer = context->Get<CommandBuffer *>();

    Check(commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->Handle());
    Check(commandBuffer->End());

    Check(queue->Submit(VkSubmitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO }, VK_NULL_HANDLE));

    Check(device->Wait());
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

    ImGuiIO     &io  = ImGui::GetIO();
    Application *app = Application::App();

    io.DisplaySize = ImVec2{ ncast<float>(app->Width()), ncast<float>(app->Height()) };

    ImDrawData* primaryDrawData = ImGui::GetDrawData();

    VkClearValue ClearValue{};
    ClearValue.color.float32[0] = 0.18f;
    ClearValue.color.float32[1] = 0.18f;
    ClearValue.color.float32[2] = 0.18f;
    ClearValue.color.float32[3] = 1.00f;

    VkRenderPassBeginInfo info = {};
    info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass               = renderPass->Handle();
    info.framebuffer              = context->CurrentFramebuffer()->Handle();
    info.renderArea.extent.width  = io.DisplaySize.x;
    info.renderArea.extent.height = io.DisplaySize.y;
    info.clearValueCount          = 1;
    info.pClearValues             = &ClearValue;

    vkCmdBeginRenderPass(commandBuffer->Handle(), &info, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(primaryDrawData, commandBuffer->Handle());

    vkCmdEndRenderPass(commandBuffer->Handle());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

}
}
