#include "GuiLayer.h"

#ifndef _UNICODE
#define _UNICODE
#endif
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.cpp>

#include "Render/Render.h"
#include "Barrier.h"
#include "Event/ApplicationEvent.h"

namespace Immortal
{
namespace D3D12
{

static const std::string shaderSource = R"(
cbuffer push_constant : register(b0)
{
    float2 uScale;
    float2 uTranslate;
};

struct VS_INPUT
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
    float4 col : COLOR;
};

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos * uScale + uTranslate, 0, 1);
    output.col = input.col;
    output.uv  = input.uv;

    output.pos.y = -output.pos.y;
    return output;
}

float4 PSMain(PS_INPUT input) : SV_Target
{
    float4 out_col = input.col * texture0.Sample(sampler0, input.uv);
    return out_col;
}

)";

GuiLayer::GuiLayer(SuperRenderContext *superContext) :
    context{ dcast<RenderContext*>(superContext)}
{
    swapchain = context->GetAddress<Swapchain>();
}

GuiLayer::~GuiLayer()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
}

void GuiLayer::OnAttach()
{
    Super::OnAttach();

    auto &io       = ImGui::GetIO();
    Vector2 extent = context->Extent();
    io.DisplaySize = ImVec2{ extent.x, extent.y };

    auto window = context->GetAddress<Window>();
    ImGui_ImplWin32_Init(rcast<HWND>(window->Primitive()));

    Descriptor descriptor = context->AllocateShaderVisibleDescriptor();

    Ref<Shader> shader = new Shader{"imgui_shader_source.hlsl", shaderSource, Shader::Type::Graphics};
    pipeline = new GraphicsPipeline{ context, shader };
    pipeline->Set({
        { Format::VECTOR2, "POSITION" },
        { Format::VECTOR2, "TEXCOORD" },
        { Format::RGBA8,   "COLOR"    },
    });
    pipeline->Enable(Pipeline::State::Blend);
    pipeline->Disable(Pipeline::State::Depth);
    pipeline->Create(swapchain->GetRenderTarget());

    ImGui_ImplDX12_Init(
        *context->GetAddress<Device>(),
         context->FrameSize(),
         context->Get<DXGI_FORMAT>(),
	    *context->ShaderResourceViewDescritorHeap(),
	    descriptor.cpu,
	    descriptor.gpu
        );
}

void GuiLayer::Begin()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();

    if (!fonts)
    {
		__CreateFontsTexture();
    }

    Super::__Begin();
}

void GuiLayer::OnEvent(Event &e)
{
    if (e.GetType() == Event::Type::WindowResize)
    {
        auto resize = dcast<WindowResizeEvent *>(&e);
        ImGuiIO   &io  = ImGui::GetIO();
        io.DisplaySize = ImVec2{ (float)resize->Width(), (float)resize->Height() };
    }
    Super::OnEvent(e);
}

void GuiLayer::End()
{
    Super::__End();

    context->Submit([&](CommandList *commandList) {
		auto &io = ImGui::GetIO();

        auto extent = context->Extent();
		io.DisplaySize = { extent.x, extent.y };
        Barrier<BarrierType::Transition> barrier{
            swapchain->GetRenderTarget(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        };
        commandList->ResourceBarrier(&barrier);

        CPUDescriptor rtvDescritor = swapchain->GetDescriptor();
        commandList->ClearRenderTargetView(rtvDescritor, rcast<float *>(&clearColor));
        commandList->SetRenderTargets(&rtvDescritor, 1, false, nullptr);

		commandList->SetDescriptorHeaps(context->ShaderResourceViewDescritorHeap()->AddressOf(), 1);

        __RenderDrawData(commandList);

        barrier.Swap();
        commandList->ResourceBarrier(&barrier);

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault(nullptr, (void*)(commandList->Handle()));
        }
        });
}

void GuiLayer::__CreateFontsTexture()
{
    int width;
    int height;
    uint8_t *pixels;

    auto &io = ImGui::GetIO();
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    Texture::Description desc = {
        Format::RGBA8,
        Wrap::Repeat,
        Filter::Linear,
    };

    fonts = new Texture{context, U32(width), U32(height), pixels, desc};

    io.Fonts->SetTexID((ImTextureID)(uint64_t)*fonts);
}

void GuiLayer::__RenderDrawData(CommandList *commandList)
{
   ImDrawData *drawData = ImGui::GetDrawData();

    RenderBuffer &buffer = buffers;
    auto &[vertex, index] = buffer;

    auto [width, height] = drawData->DisplaySize;
	if (width <= 0.0f || height <= 0.0f)
    {
        return;
    }

    if (drawData->TotalIdxCount > 0)
    {
        size_t totalVertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        size_t totalIndexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

        if (!vertex || vertex->Size() < totalVertexSize)
        {
            vertex = new Buffer{ context, totalVertexSize, Buffer::Type::Vertex};
        }

        if (!index || index->Size() < totalIndexSize)
        {
            index = new Buffer{ context, totalIndexSize, Buffer::Type::Index};
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
        vertex->Unmap();
        index->Unmap();

        pipeline->Set(vertex);
        pipeline->Set(index);
        auto vertexView = pipeline->Get<Buffer::VertexView>();
        commandList->SetVertexBuffers(&vertexView);
        auto indexView = pipeline->Get<Buffer::IndexView>();
        indexView.Format = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        commandList->SetIndexBuffer(&indexView);

        commandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetPipelineState(*pipeline);
		commandList->SetGraphicsRootSignature(pipeline->Get<RootSignature&>());

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
		commandList->PushConstant(sizeof(pushConstant), &pushConstant, 0);

        const float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
        commandList->OMSetBlendFactor(blendFactor);

        D3D12_VIEWPORT viewport = {
		    .TopLeftX = 0,
		    .TopLeftY = 0,
		    .Width    = drawData->DisplaySize.x,
		    .Height   = drawData->DisplaySize.y,
		    .MinDepth = 0.0f,
		    .MaxDepth = 1.0f,
		};
		commandList->RSSetViewports(&viewport);

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
                    ImVec2 clipMin((pcmd->ClipRect.x - clipOff.x) * clipScale.x, (pcmd->ClipRect.y - clipOff.y) * clipScale.y);
                    ImVec2 clipMax((pcmd->ClipRect.z - clipOff.x) * clipScale.x, (pcmd->ClipRect.w - clipOff.y) * clipScale.y);

					if (clipMin.x < 0.0f)
					{
						clipMin.x = 0.0f;
					}
					if (clipMin.y < 0.0f)
					{
						clipMin.y = 0.0f;
					}
					if (clipMax.x > width)
					{
						clipMax.x = width;
					}
					if (clipMax.y > height)
					{
						clipMax.y = height;
					}
					if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
					{
						continue;
					}

                    const D3D12_RECT rect = {
                        .left   = (LONG)clipMin.x,
                        .top    = (LONG)clipMin.y,
                        .right  = (LONG)clipMax.x,
                        .bottom = (LONG)clipMax.y
                    };
					commandList->RSSetScissorRects(&rect);

                    D3D12_GPU_DESCRIPTOR_HANDLE descriptor = { .ptr = (UINT64) pcmd->TextureId };
                    commandList->SetGraphicsRootDescriptorTable(1, descriptor);
                    commandList->DrawIndexedInstanced(pcmd->ElemCount, 1, pcmd->IdxOffset + globalIndexOffset, pcmd->VtxOffset + globalVertexOffset, 0);
                }
            }
            globalVertexOffset += cmdList->VtxBuffer.Size;
            globalIndexOffset += cmdList->IdxBuffer.Size;
        }
    }
}

}
}
