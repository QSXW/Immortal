#include "GuiLayer.h"

#include "Framework/Application.h"
#include "Render/Render.h"
#include "RenderContext.h"
#include "Pipeline.h"

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

static std::vector<uint32_t> ImGuiVertSpirv = {
    0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
    0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
    0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
    0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
    0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
    0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
    0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
    0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
    0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
    0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
    0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
    0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
    0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
    0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
    0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
    0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
    0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
    0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
    0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
    0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
    0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
    0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
    0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
    0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
    0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
    0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
    0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
    0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
    0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
    0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
    0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
    0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
    0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
    0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
    0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
    0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
    0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
    0x0000002d,0x0000002c,0x000100fd,0x00010038
};

static std::vector<uint32_t> ImGuiFragSpirv = {
    0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
    0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
    0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
    0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
    0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
    0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
    0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
    0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
    0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
    0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
    0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
    0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
    0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
    0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
    0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
    0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
    0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
    0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
    0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
    0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
    0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
    0x00010038
};

GuiLayer::GuiLayer(RenderContext *context) :
    context{ context }
{
    SLASSERT(context && "Render Context could not be NULL.");

    auto device = context->GetAddress<Device>();

    shader = new Shader{device, ImGuiVertSpirv, ImGuiFragSpirv};
    pipeline = new GraphicsPipeline{device, shader.Get()};
    pipeline->Enable(Pipeline::State::Blend);
    pipeline->Set({
        { Format::VECTOR2, "POSITION" },
        { Format::VECTOR2, "UV"       },
        { Format::RGBA8,   "COLOR"    },
    });
    pipeline->Create(context->GetAddress<RenderTarget>());
}

GuiLayer::~GuiLayer()
{
	descriptorPool.Reset();
	fonts.Reset();
    for (auto &buffer : buffers)
    {
		buffer.vertex.Reset();
		buffer.index.Reset();
    }
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void GuiLayer::OnAttach()
{
    Super::OnAttach();

    Application *app = Application::App();
    ImGui_ImplGlfw_InitForVulkan(rcast<GLFWwindow *>(app->GetNativeWindow()), true);

    auto device = context->GetAddress<Device>();
    descriptorPool = new DescriptorPool{ device, Limit::PoolSize };

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

    __CreateFontsTexture();
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
        ImGuiIO &io  = ImGui::GetIO();
        io.DisplaySize = ImVec2{ (float)resize->Width(), (float)resize->Height() };
    }
}

void GuiLayer::Begin()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    Super::__Begin();
}

void GuiLayer::End()
{
    Super::__End();

    ImGuiIO &io = ImGui::GetIO();

    const auto &[width, height] = context->GetViewportSize();
    
    if (width > 0 && height > 0)
	{
		io.DisplaySize = ImVec2{(float) width, (float) height};

		context->Record([&](CommandBuffer *cmdbuf) -> void {
			auto renderTarget = context->GetAddress<RenderTarget>();
			context->Begin(renderTarget);
			__RenderDrawData(cmdbuf);
			context->End();
		});
    }

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void GuiLayer::__RenderDrawData(CommandBuffer *cmdbuf)
{
    ImDrawData *drawData = ImGui::GetDrawData();

    auto &[vertex, index] = buffers[sync];
	SLROTATE(sync, 3);

    int width  = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (width <= 0 || height <= 0)
    {
        return;
    }

    if (drawData->TotalIdxCount > 0)
    {
        size_t totalVertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        size_t totalIndexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

        if (!vertex || vertex->Size() < totalVertexSize)
        {
			__InvalidateRenderBuffer<Buffer::Type::Vertex>(vertex, totalVertexSize);
        }

		if (!index || index->Size() < totalIndexSize)
        {
			__InvalidateRenderBuffer<Buffer::Type::Index>(index, totalIndexSize);
        }

        uint8_t *pVertex;
		uint8_t *pIndex;
		vertex->Map(&pVertex);
		index->Map(&pIndex);
        for (int i = 0, voffset = 0, ioffset = 0; i < drawData->CmdListsCount; i++)
        {
            const ImDrawList *cmdList = drawData->CmdLists[i];
			uint32_t vertexSize = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
			uint32_t indexSize = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

            memcpy(pVertex + voffset, cmdList->VtxBuffer.Data, vertexSize);
			memcpy(pIndex + ioffset, cmdList->IdxBuffer.Data, indexSize);

			voffset += vertexSize;
			ioffset += indexSize;
        }
        vertex->Flush();
		index->Flush();
		vertex->Unmap();
		index->Unmap();

        cmdbuf->BindPipeline(*pipeline, pipeline->BindPoint);
        cmdbuf->BindVertexBuffers(vertex);
        cmdbuf->BindIndexBuffer(index, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
        
        struct {
            float scale[2];
            float translate[2];
        } pushConstant {
            .scale = {
                2.0f / drawData->DisplaySize.x,
                2.0f / drawData->DisplaySize.y,
            },
            .translate = {
                -1.0f - drawData->DisplayPos.x * (2.0f / drawData->DisplaySize.x),
                -1.0f - drawData->DisplayPos.y * (2.0f / drawData->DisplaySize.y),
            }
        };
        cmdbuf->PushConstants(pipeline->Layout(), Shader::Stage::Vertex, 0, sizeof(pushConstant), &pushConstant);

        ImVec2 clipOff = drawData->DisplayPos;         
        ImVec2 clipScale = drawData->FramebufferScale; 
        int globalVertexOffset = 0;
        int globalIndexOffset = 0;
        for (int i = 0; i < drawData->CmdListsCount; i++)
        {
            const ImDrawList *cmdList = drawData->CmdLists[i];
            for (int cmd_i = 0; cmd_i < cmdList->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd *pcmd = &cmdList->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != NULL)
                {
                    SLASSERT(false);
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pcmd->ClipRect.x - clipOff.x) * clipScale.x, (pcmd->ClipRect.y - clipOff.y) * clipScale.y);
                    ImVec2 clip_max((pcmd->ClipRect.z - clipOff.x) * clipScale.x, (pcmd->ClipRect.w - clipOff.y) * clipScale.y);

                    // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                    if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
                    if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
                    if (clip_max.x > width) { clip_max.x = (float)width;}
                    if (clip_max.y > height) { clip_max.y = (float)height; }
                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) { continue; }

                    // Apply scissor/clipping rectangle
                    VkRect2D scissor = {
                        .offset = {
                            .x = (int32_t)(clip_min.x),
                            .y = (int32_t)(clip_min.y),
                        },
                        .extent = {
                            .width  = (uint32_t)(clip_max.x - clip_min.x),
                            .height = (uint32_t)(clip_max.y - clip_min.y),
                        },
                    };
    
                     cmdbuf->SetScissor(0, 1, &scissor);

                    VkDescriptorSet descriptorSets[1] = { (VkDescriptorSet)pcmd->TextureId };
					cmdbuf->BindDescriptorSets(pipeline->BindPoint, pipeline->Layout() , 0, 1, descriptorSets, 0, nullptr);
                    cmdbuf->DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + globalIndexOffset, pcmd->VtxOffset + globalVertexOffset, 0);
                }
            }
            globalVertexOffset += cmdList->VtxBuffer.Size;
            globalIndexOffset += cmdList->IdxBuffer.Size;
        }
		cmdbuf->SetScissor(uint32_t(width), uint32_t(height));
    }
}

void GuiLayer::__CreateFontsTexture()
{
    int width; int height;
    uint8_t *pixels;

    auto &io = ImGui::GetIO();
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    Texture::Description desc = {
        Format::RGBA8,
        Wrap::Repeat,
        Filter::Linear,
    };

    auto device = context->GetAddress<Device>();
    fonts = new Texture{ device, U32(width), U32(height), pixels, desc };

    io.Fonts->SetTexID((ImTextureID)(uint64_t)*fonts);
}

}
}
