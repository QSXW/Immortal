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

GuiLayer::GuiLayer(RenderContext *context, VkRenderPass renderPass) :
    context{ context },
    renderPass{ renderPass }
{
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

    auto &device = context->Get<VkDevice>();

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

    Check(vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool));

    ImGui_ImplVulkan_InitInfo initInfo{};
    auto &queue = context->Get<Queue*>();

    initInfo.Instance        = context->Get<VkInstance>();
    initInfo.PhysicalDevice  = context->Get<VkPhysicalDevice>();
    initInfo.Device          = device;
    initInfo.QueueFamily     = queue->Get<Queue::FamilyIndex>();
    initInfo.Queue           = queue->Handle();
    initInfo.PipelineCache   = pipelineCache;
    initInfo.DescriptorPool  = descriptorPool;
    initInfo.Allocator       = nullptr;
    initInfo.MinImageCount   = 2;
    initInfo.ImageCount      = context->Get<RenderContext::Frames>().size();
    initInfo.CheckVkResultFn = &Check;

    ImGui_ImplVulkan_LoadFunctions(nullptr, nullptr);
    ImGui_ImplVulkan_Init(&initInfo, renderPass);
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

    ImGuiIO &io = ImGui::GetIO();
    Application *app = Application::App();
    io.DisplaySize = ImVec2((float)app->Width(), (float)app->Height());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

}
}
