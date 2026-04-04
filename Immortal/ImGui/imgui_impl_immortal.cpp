
/**
 * Copyright (C) 2023, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 *
 */

#include "imgui_impl_immortal.h"

using namespace Immortal;

#define DEFAULT_VERTEX_SIZE  5000
#define DEFAULT_INDEX_SIZE   10000
#define MAX_FRAMES_IN_FLIGHT 6

#ifdef _WIN32
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC BackupWndProc;

static LRESULT CALLBACK ImGui_ImplImmortal_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
		return true;
    }
	return ::CallWindowProcW(BackupWndProc, hWnd, msg, wParam, lParam);
}
#endif

struct ImGui_ImplImmortal_Data
{
    Device                 *device;
    Swapchain              *swapchain;
    Queue                  *queue;
    URef<Sampler>           fontSampler;
    URef<GraphicsPipeline>  pipeline;
    Ref<Texture>            fontTexture;
    URef<DescriptorSet>     descriptorSet;
    uint32_t                swapchainBufferCount;
};

class ImGui_ImplImmortal_FrameContext
{
public:
    ImGui_ImplImmortal_FrameContext() :
        indexBuffer{},
        vertexBuffer{},
        descriptorSets{}
    {

    }

    ~ImGui_ImplImmortal_FrameContext()
    {
        for (auto &descriptorSet : freeDescriptorSets)
        {
			delete descriptorSet;
        }
		for (auto &[texture, descriptorSet] : descriptorSets)
		{
			delete descriptorSet;
		}
        freeDescriptorSets.clear();
		descriptorSets.clear();
    }

    URef<Buffer> indexBuffer;
    URef<Buffer> vertexBuffer;
	URef<Buffer> uploadBuffer;
    void RefreshDescriptorSet()
    {
        uint32_t rest = freeDescriptorSets.size() - allocated;
		if (allocated > 0)
        {
            memmove(freeDescriptorSets.data(), freeDescriptorSets.data() + allocated, rest * sizeof(freeDescriptorSets[0]));
            freeDescriptorSets.resize(rest);
        }
        for (auto &[texture, descriptorSet] : descriptorSets)
        {
            freeDescriptorSets.emplace_back(descriptorSet);
        }
        descriptorSets.clear();
        allocated = 0;
    }

    DescriptorSet *AllocateDescriptorSet(Device *device, Pipeline *pipeline, Texture *texture)
    {
		DescriptorSet *descriptorSet = nullptr;
        if (allocated < freeDescriptorSets.size())
        {
			descriptorSet = freeDescriptorSets[allocated];
			freeDescriptorSets[allocated++] = nullptr;
        }
        else
        {
			descriptorSet = device->CreateDescriptorSet(pipeline);
        }

        descriptorSets[texture] = descriptorSet;
        return descriptorSet;
    }

    uint32_t allocated = 0;
    std::vector<DescriptorSet *> freeDescriptorSets;
    std::unordered_map<Texture *, DescriptorSet*> descriptorSets;
};

struct ImGui_ImplImmortal_ViewportData
{
    ImGui_ImplImmortal_ViewportData(uint32_t bufferCount) :
        window{},
        swapchain{},
        gpuEvent{},
        commandBuffers{},
        syncValues{},
        syncPoint{},
        NumFramesInFlight{ bufferCount },
        FrameIndex{},
	    frameCtx{}
    {
		frameCtx = new ImGui_ImplImmortal_FrameContext[NumFramesInFlight];
    }

    ~ImGui_ImplImmortal_ViewportData()
    {
		delete []frameCtx;
    }

    URef<Window>                                          window;
    URef<Swapchain>                                       swapchain;
    URef<GPUEvent>                                        gpuEvent;
    URef<CommandBuffer>                                   commandBuffers[3];
    uint64_t                                              syncValues[3];
    uint32_t                                              syncPoint;
    uint32_t                                              NumFramesInFlight;
    uint32_t                                              FrameIndex;
	ImGui_ImplImmortal_FrameContext                      *frameCtx;
};

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static ImGui_ImplImmortal_Data *ImGui_ImplImmortal_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplImmortal_Data *)ImGui::GetIO().BackendRendererUserData : nullptr;
}

void ImGui_ImplImmortal_CreateDeviceObjects()
{
    ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();

    if (!bd->fontSampler)
    {
        bd->fontSampler = bd->device->CreateSampler(Filter::Bilinear, AddressMode::Wrap, CompareOperation::Never, -1000.0f, 1000.0f);
        IM_ASSERT(bd->fontSampler && "Failed to create font sampler");
    }

    static const std::string kShaderSource = R"(
struct PushConstant
{
    float4x4 uProjectionMatrix;
};

[[vk::push_constant]] PushConstant push_constant;

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

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(push_constant.uProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
    output.col = input.col;
    output.uv  = input.uv;

    return output;
}

[[vk::binding(0, 0)]] Texture2D texture0 : register(t0);
[[vk::binding(1, 0)]] SamplerState sampler0 : register(s1);

float4 PSMain(PS_INPUT input) : SV_Target
{
    float4 out_col = input.col * texture0.Sample(sampler0, input.uv);
    return out_col;
}

)";

        static const std::string kHLSL5_0ShaderSource = R"(
struct PushConstant
{
    float4x4 uProjectionMatrix;
};

PushConstant push_constant;

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

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(push_constant.uProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
    output.col = input.col;
    output.uv  = input.uv;

    return output;
}

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s1);

float4 PSMain(PS_INPUT input) : SV_Target
{
    float4 out_col = input.col * texture0.Sample(sampler0, input.uv);
    return out_col;
}

)";

const GLchar *GLSL450CoreVertexShaderSource = R"(
        #version 450
        layout (location = 0) in vec2 Position;
        layout (location = 1) in vec2 UV;
        layout (location = 2) in vec4 Color;

        layout (push_constant) uniform UBO
        {
            mat4 ProjMtx;
        } ubo;

        layout (location = 0) out vec2 Frag_UV;
        layout (location = 1) out vec4 Frag_Color;
        void main()
        {
            Frag_UV = UV;
            Frag_Color = Color;
            gl_Position = ubo.ProjMtx * vec4(Position.xy,0,1);
        }
)";

    const GLchar *GLSL450CoreFragmentShaderSource = R"(
        #version 450
        layout (location = 0) in vec2 Frag_UV;
        layout (location = 1) in vec4 Frag_Color;

        layout (binding = 0) uniform sampler2D Texture;

        layout (location = 0) out vec4 Out_Color;

        void main()
        {
            Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
        }
)";

    const std::string *shaderSource = &kShaderSource;
#ifndef IMGUI_DISABLE_SDF
        static std::string kSDFShaderSource = R"(
struct PushConstant
{
    float4x4 uProjectionMatrix;
};
[[vk::push_constant]] PushConstant push_constant;

struct VS_INPUT
{
    float2 pos             : POSITION;
    float2 uv              : TEXCOORD0;
    float4 innerColor      : COLOR0;
    float4 startOuterColor : COLOR1;
    float4 endOuterColor   : COLOR2;
    float a                : COLOR3;
    float b                : COLOR4;
    float w                : COLOR5;
};

struct PS_INPUT
{
    float4 pos              : SV_POSITION;
    float2 uv               : TEXCOORD0;
    float4 innerColor       : COLOR0;
    float4 startOuterColor  : COLOR1;
    float4 endOuterColor    : COLOR2;
    nointerpolation float a : COLOR3;
    nointerpolation float b : COLOR4;
    nointerpolation float w : COLOR5;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.pos             = mul(push_constant.uProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
    output.uv              = input.uv;
    output.innerColor      = input.innerColor;
    output.startOuterColor = input.startOuterColor;
    output.endOuterColor   = input.endOuterColor;
    output.a               = input.a;
    output.b               = input.b;
    output.w               = input.w;
    return output;
}

[[vk::binding(0, 0)]] Texture2D texture0 : register(t0);
[[vk::binding(1, 0)]] SamplerState sampler0 : register(s1);

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float stretch(float low, float high, float x)
{
    return clamp((x-low)/(high-low), 0.0, 1.0);
}

[earlydepthstencil] float4 PSMain(PS_INPUT input) : SV_Target
{
    if (input.a == 0.0)
        return input.innerColor * texture0.Sample(sampler0, input.uv);
    float distance;
    if (input.a >= 2.0)
    {
        input.a = input.a - 2.0;
        distance = 1.0 - clamp(length(input.uv), 0.0, 1.0);
    }
    else
    {
        float3 msd = texture0.Sample(sampler0, input.uv).rgb;      
        distance = median(msd.r, msd.g, msd.b); //texture0.Sample(sampler0, input.uv).a;
    }
    if (distance >= input.a + input.w) return input.innerColor;
    if (distance <= input.b - input.w) discard;
    float m = stretch(input.a - input.w, min(1.0, input.a + input.w), distance);
    if (input.a <= input.b)
        return float4(input.innerColor.rgb, input.innerColor.a * m);
    float outerMix = stretch(input.b, input.a, distance);
    float4 outer = lerp(input.endOuterColor, input.startOuterColor, outerMix);
    outer.a *= stretch(input.b - input.w, input.b + input.w, distance);
    float ia = m * input.innerColor.a;
    float oa = (1 - m) * outer.a;
    float a = ia + oa;
    return float4((input.innerColor.rgb * ia + outer.rgb * oa) / a, a);
}
)";

	bool sdf = (ImGui::GetIO().BackendFlags & ImGuiBackendFlags_SignedDistanceFonts) | (ImGui::GetIO().BackendFlags & ImGuiBackendFlags_SignedDistanceShapes);
    if (sdf)
    {
		shaderSource = &kSDFShaderSource;
    }
#endif

    if (!bd->pipeline)
    {
        URef<Shader> vsShader;
        URef<Shader> psShader;

        const std::string vsShaderName = "ImguiVS";
        const std::string psShaderName = "ImGuiPS";

        if (bd->device->GetBackendAPI() == BackendAPI::OpenGL)
        {
            vsShader = bd->device->CreateShader(vsShaderName, Shader::Stage::Vertex, GLSL450CoreVertexShaderSource,   "VSMain");
            psShader = bd->device->CreateShader(psShaderName, Shader::Stage::Pixel,  GLSL450CoreFragmentShaderSource, "PSMain");
        }
        else if (bd->device->GetBackendAPI() == BackendAPI::D3D11)
        {
            vsShader = bd->device->CreateShader(vsShaderName, Shader::Stage::Vertex, kHLSL5_0ShaderSource, "VSMain");
            psShader = bd->device->CreateShader(psShaderName, Shader::Stage::Pixel,  kHLSL5_0ShaderSource, "PSMain");
        }
        else
        {
            // D3D12 and Vukan are using DirectX Shader Compiler
			vsShader = bd->device->CreateShader(vsShaderName, Shader::Stage::Vertex, *shaderSource, "VSMain");
			psShader = bd->device->CreateShader(psShaderName, Shader::Stage::Pixel, *shaderSource, "PSMain");
        }

        Shader *shaders[] = { vsShader, psShader };

        InputElementDescription inputElementDescription = {
            { Format::VECTOR2,    "POSITION", 0 },
            { Format::VECTOR2,    "TEXCOORD", 0 },
            { Format::RGBA8,      "COLOR",    0 },
#ifndef IMGUI_DISABLE_SDF
             { Format::RGBA8,      "COLOR",    1 },
             { Format::RGBA8,      "COLOR",    2 },
             { Format::R32_SFLOAT, "COLOR",    3 },
		     { Format::R32_SFLOAT, "COLOR",    4 },
		     { Format::R32_SFLOAT, "COLOR",    5 },
#endif
        };

        bd->pipeline = bd->device->CreateGraphicsPipeline();
        bd->pipeline->Enable(Pipeline::Blend);
        bd->pipeline->Disable(Pipeline::Depth);
		//bd->pipeline->Enable(Pipeline::MSAA4X);
        bd->pipeline->Construct(shaders, 2,
			inputElementDescription,
            { Format::BGRA8 } /* render target color attachments */
        );
    }
}

void ImGui_ImplImmortal_DestroyTexture(ImTextureData *tex)
{
	Texture *backend_tex = (Texture *) tex->BackendUserData;
	if (backend_tex == nullptr)
		return;

	ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();
	delete backend_tex;

	tex->SetTexID(ImTextureID_Invalid);
	tex->SetStatus(ImTextureStatus_Destroyed);
	tex->BackendUserData = nullptr;
}

bool ImGui_ImplImmortal_UpdateTexture(ImTextureData *tex, ImGui_ImplImmortal_FrameContext *fr, CommandBuffer *commandBuffer)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();

    auto &texture = (Texture * &)tex->BackendUserData;
	if (tex->Status == ImTextureStatus_WantCreate)
	{
		texture = bd->device->CreateTexture(Format::RGBA8, tex->Width, tex->Height, 1, 1, TextureType::TransferDestination);
		tex->SetTexID(WIMAGE(texture));
	}

	if (tex->Status == ImTextureStatus_WantCreate || tex->Status == ImTextureStatus_WantUpdates)
	{
		IM_ASSERT(tex->Format == ImTextureFormat_RGBA32);

		const int x = (tex->Status == ImTextureStatus_WantCreate) ? 0 : tex->UpdateRect.x;
		const int y = (tex->Status == ImTextureStatus_WantCreate) ? 0 : tex->UpdateRect.y;
		const int w = (tex->Status == ImTextureStatus_WantCreate) ? tex->Width : tex->UpdateRect.w;
		const int h = (tex->Status == ImTextureStatus_WantCreate) ? tex->Height : tex->UpdateRect.h;

		UINT srcPitch = w * tex->BytesPerPixel;
		UINT dstPitch = SLALIGN(srcPitch, TextureAlignment);
		UINT uploadSize = dstPitch * h;

		auto &buffer = fr->uploadBuffer;
		if (!buffer || uploadSize > buffer->GetSize())
		{
			buffer = bd->device->CreateBuffer(BufferType::TransferSource, uploadSize);
		}

		void *mapped = nullptr;
		buffer->Map(&mapped, uploadSize, 0);

		for (int i = 0; i < h; i++)
		{
			memcpy((void *) ((uintptr_t)mapped + i * dstPitch), tex->GetPixelsAt(x, y + i), srcPitch);
		}
		buffer->Unmap();

		//if (!commandBuffer)
		{
			URef<CommandBuffer> commandBuffer = bd->device->CreateCommandBuffer(QueueType::Transfer);
			URef<Queue> queue = bd->device->CreateQueue(QueueType::Transfer);

			commandBuffer->Begin();
			commandBuffer->CopyTextureRegion(texture, 0, w, h, x, y, 0, buffer, dstPitch, 0);
			commandBuffer->End();

			URef<GPUEvent> pEvent = bd->device->CreateGPUEvent("UploadBufferEvent");
			GPUEvent *submitEvents[] = {pEvent};

			CommandBuffer *commandBuffers[] = {commandBuffer};
			queue->Submit(commandBuffers, 1, submitEvents, 1);
			pEvent->Wait(0xffffffff);

			bd->descriptorSet = bd->device->CreateDescriptorSet(bd->pipeline);
			bd->descriptorSet->Set(0, texture);
			bd->descriptorSet->Set(1, bd->fontSampler);
		}
		//else
		//{
		//	commandBuffer->CopyTextureRegion(texture, 0, w, h, x, y, 0, buffer, dstPitch, 0);
		//}
		tex->SetStatus(ImTextureStatus_OK);
	}

	if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames >= (int) bd->swapchainBufferCount)
	{
		ImGui_ImplImmortal_DestroyTexture(tex);
	}

	return true;
}

IMGUI_IMPL_API bool ImGui_ImplImmortal_Init(Device *device, Window *window, Queue *queue, Swapchain *swapchain, uint32_t swapchainBufferCount, ImGuiBackendFlags flags)
{
    ImGuiIO &io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    ImGui_ImplImmortal_Data *bd = IM_NEW(ImGui_ImplImmortal_Data)();
    io.BackendRendererUserData = (void *)bd;
    io.BackendRendererName = "imgui_impl_immortal";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports; // We can create multi-viewports on the Renderer side (optional)

#ifdef IMGUI_ENABLE_MSDFGEN
	io.BackendFlags |= (flags & ImGuiBackendFlags_SignedDistanceFonts);
	io.BackendFlags |= (flags & ImGuiBackendFlags_SignedDistanceShapes);
#endif

	io.BackendFlags |= ImGuiBackendFlags_ProvocingVertexFirst;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

    bd->device    = device;
    bd->queue     = queue;
    bd->swapchain = swapchain;
    bd->swapchainBufferCount = swapchainBufferCount;

#ifdef _WIN32
    if (window->GetType() == WindowType::Win32)
    {
        HWND handle = (HWND)window->GetBackendHandle();
		BackupWndProc = (WNDPROC)::GetWindowLongPtrW(handle, GWLP_WNDPROC);
		::SetWindowLongPtrW(handle, GWLP_WNDPROC, (LONG_PTR)&ImGui_ImplImmortal_WndProc);
    }
#endif

    ImGui_ImplImmortal_CreateDeviceObjects();

    ImGuiViewport *mainViewport = ImGui::GetMainViewport();
    mainViewport->RendererUserData = IM_NEW(ImGui_ImplImmortal_ViewportData)(bd->swapchainBufferCount);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui_ImplImmortal_InitPlatformInterface();
    }

    return true;
}

IMGUI_IMPL_API void ImGui_ImplImmortal_Shutdown()
{
    ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();
    bd->descriptorSet.Reset();
    bd->fontSampler.Reset();
    bd->fontTexture.Reset();
    bd->pipeline.Reset();

    for (ImTextureData *tex : ImGui::GetPlatformIO().Textures)
	{
		if (tex->RefCount == 1)
		{
			ImGui_ImplImmortal_DestroyTexture(tex);
		}
	}
    
    // Manually delete main viewport render data in-case we haven't initialized for viewports
    ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    if (ImGui_ImplImmortal_ViewportData *vd = (ImGui_ImplImmortal_ViewportData *)main_viewport->RendererUserData)
        IM_DELETE(vd);
    main_viewport->RendererUserData = nullptr;

    auto &io = ImGui::GetIO();
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
	io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures | ImGuiBackendFlags_RendererHasViewports);
    IM_DELETE(bd);
}

IMGUI_IMPL_API void ImGui_ImplImmortal_NewFrame()
{
    ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplImmorta_Init()?");

    if (!bd->fontSampler)
    {
        ImGui_ImplImmortal_CreateDeviceObjects();
    }
}

IMGUI_IMPL_API void ImGui_ImplImmortal_RenderDrawData(ImDrawData *drawData, CommandBuffer *commandBuffer)
{
    int width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (width <= 0 || height <= 0)
    {
        return;
    }

    ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();
    ImGui_ImplImmortal_ViewportData *vd = (ImGui_ImplImmortal_ViewportData *)drawData->OwnerViewport->RendererUserData;
    vd->FrameIndex++;
    ImGui_ImplImmortal_FrameContext *fr = &vd->frameCtx[vd->FrameIndex % bd->swapchainBufferCount];

    if (drawData->Textures != nullptr)
	{
		for (ImTextureData *tex : *drawData->Textures)
		{
			if (tex->Status != ImTextureStatus_OK)
			{
				ImGui_ImplImmortal_UpdateTexture(tex, fr, commandBuffer);
			}
		}
	}

    if (drawData->TotalIdxCount <= 0)
    {
        return;
    }

    fr->RefreshDescriptorSet();

    size_t totalVertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
    size_t totalIndexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

    if (!fr->vertexBuffer || fr->vertexBuffer->GetSize() < totalVertexSize)
    {
		fr->vertexBuffer = bd->device->CreateBuffer(BufferType::Vertex, totalVertexSize + DEFAULT_VERTEX_SIZE);
    }

    if (!fr->indexBuffer || fr->indexBuffer->GetSize() < totalIndexSize)
    {
		fr->indexBuffer = bd->device->CreateBuffer(BufferType::Index, totalIndexSize + DEFAULT_INDEX_SIZE);
    }

    if (bd->device->GetBackendAPI() == BackendAPI::OpenGL || bd->device->GetBackendAPI() == BackendAPI::D3D11)
    {
        for (int i = 0, vertexOffset = 0, indexOffset = 0; i < drawData->CmdListsCount; i++)
        {
            const ImDrawList *cmdList = drawData->CmdLists[i];
            uint32_t vertexSize = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
            uint32_t indexSize = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

            commandBuffer->MemoryCopy(fr->vertexBuffer, vertexSize, cmdList->VtxBuffer.Data, vertexOffset);
            commandBuffer->MemoryCopy(fr->indexBuffer, indexSize, cmdList->IdxBuffer.Data, indexOffset);

            vertexOffset += vertexSize;
            indexOffset += indexSize;
        }
    }
    else
    {
        uint8_t *pVertex = nullptr;
        uint8_t *pIndex  = nullptr;
        fr->vertexBuffer->Map((void **) &pVertex, totalVertexSize, 0);
        fr->indexBuffer->Map((void **) &pIndex, totalIndexSize, 0);
        for (int i = 0, vertexOffset = 0, indexOffset = 0; i < drawData->CmdListsCount; i++)
        {
            const ImDrawList *cmdList = drawData->CmdLists[i];
            uint32_t vertexSize = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
            uint32_t indexSize = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

            memcpy(pVertex + vertexOffset, cmdList->VtxBuffer.Data, vertexSize);
            memcpy(pIndex + indexOffset, cmdList->IdxBuffer.Data, indexSize);

            vertexOffset += vertexSize;
            indexOffset += indexSize;
        }
        fr->vertexBuffer->Unmap();
        fr->indexBuffer->Unmap();
    }

    commandBuffer->SetPipeline(bd->pipeline);

    Buffer *vertexBuffers[] = { fr->vertexBuffer };
    commandBuffer->SetVertexBuffers(0, 1, vertexBuffers, sizeof(ImDrawVert));
    commandBuffer->SetIndexBuffer(fr->indexBuffer, sizeof(ImDrawIdx) == 2 ? Format::UINT16 : Format::UINT32);

    float L = drawData->DisplayPos.x;
    float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
    float T = drawData->DisplayPos.y;
    float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
    float mvp[4][4] =
    {
        { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
        { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
        { 0.0f,         0.0f,           0.5f,       0.0f },
        { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
    };

    //float L = drawData->DisplayPos.x - 0.5f;
    //float R = drawData->DisplayPos.x + drawData->DisplaySize.x - 0.5f;
    //float T = drawData->DisplayPos.y - 0.5f;
    //float B = drawData->DisplayPos.y + drawData->DisplaySize.y - 0.5f;

    //float mvp[4][4] =
    //{
    //    { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
    //    { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
    //    { 0.0f,         0.0f,           1.0f,       0.0f },  // 深度范围 [0,1]
    //    { (R+L)/(L-R),  (T+B)/(B-T),    0.0f,       1.0f },  // 深度偏移 0
    //};

    commandBuffer->PushConstants(Shader::Stage::Vertex, &mvp, sizeof(mvp), 0);

    const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    commandBuffer->SetBlendFactor(blendFactor);

    Texture *lastTexture = nullptr;
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
                ImVec2 minClip((pcmd->ClipRect.x - clipOff.x) * clipScale.x, (pcmd->ClipRect.y - clipOff.y) * clipScale.y);
                ImVec2 maxClip((pcmd->ClipRect.z - clipOff.x) * clipScale.x, (pcmd->ClipRect.w - clipOff.y) * clipScale.y);

                if (minClip.x < 0.0f)
                {
                    minClip.x = 0.0f;
                }
                if (minClip.y < 0.0f)
                {
                    minClip.y = 0.0f;
                }
                if (maxClip.x > width)
                {
                    maxClip.x = (float) width;
                }
                if (maxClip.y > height)
                {
                    maxClip.y = (float) height;
                }
                if (maxClip.x <= minClip.x || maxClip.y <= minClip.y)
                {
                    continue;
                }

                Rect2D rect = {
                    .left   = uint32_t(minClip.x),
                    .top    = uint32_t(minClip.y),
                    .right  = uint32_t(maxClip.x),
                    .bottom = uint32_t(maxClip.y)
                };
                commandBuffer->SetScissors(1, &rect);

                Texture *texture = (Texture *) pcmd->GetTexID();
				if (!lastTexture || lastTexture != texture)
                {
                    DescriptorSet *descriptorSet = nullptr;
                    if (!texture || texture == bd->fontTexture)
                    {
                        texture       = bd->fontTexture;
                        descriptorSet = bd->descriptorSet;
                    }
                    else
                    {
                        if (fr->descriptorSets.find(texture) == fr->descriptorSets.end())
                        {
                            descriptorSet = fr->AllocateDescriptorSet(bd->device, bd->pipeline, texture);
                            descriptorSet->Set(0, texture);
                            descriptorSet->Set(1, bd->fontSampler);
                        }
                        else
                        {
                            descriptorSet = fr->descriptorSets[texture];
                        }
                    }
                    commandBuffer->SetDescriptorSet(descriptorSet);
                    lastTexture = texture;
                }
                commandBuffer->DrawIndexedInstance(pcmd->ElemCount, 1, pcmd->IdxOffset + globalIndexOffset, pcmd->VtxOffset + globalVertexOffset, 0);
            }
        }
        globalVertexOffset += cmdList->VtxBuffer.Size;
        globalIndexOffset += cmdList->IdxBuffer.Size;
    }

    Rect2D scissor = {0, 0, (uint32_t) width, (uint32_t) height};
    commandBuffer->SetScissors(1, &scissor);
}

static void ImGui_ImplImmortal_CreateWindow(ImGuiViewport *viewport)
{
    ImGui_ImplImmortal_Data         *bd = ImGui_ImplImmortal_GetBackendData();
    ImGui_ImplImmortal_ViewportData *vd = IM_NEW(ImGui_ImplImmortal_ViewportData)(3);
    viewport->RendererUserData          = vd;

    WindowType type = WindowType::GLFW;
#ifdef _WIN32
    if (bd->device->GetBackendAPI() != BackendAPI::OpenGL)
    {
        type = WindowType::Win32;
    }
#endif

    vd->window    = Window::CreateInstance(viewport->PlatformHandle, type);
    vd->swapchain = bd->device->CreateSwapchain(bd->queue, vd->window, Format::BGRA8, bd->swapchainBufferCount, SwapchainMode::VerticalSync);
    vd->gpuEvent  = bd->device->CreateGPUEvent();
    for (uint32_t i = 0; i < bd->swapchainBufferCount; i++)
    {
        vd->commandBuffers[i] = bd->device->CreateCommandBuffer();
    }
}

static void ImGui_ImplImmortal_DestroyWindow(ImGuiViewport *viewport)
{
    ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();
    if (ImGui_ImplImmortal_ViewportData *vd = (ImGui_ImplImmortal_ViewportData *) viewport->RendererUserData)
    {
        bd->queue->WaitIdle();
        vd->window.Reset();
        vd->swapchain.Reset();
        for (size_t i = 0; i < vd->NumFramesInFlight; i++)
        {
            vd->commandBuffers[i].Reset();
        }
        IM_DELETE(vd);
    }
    viewport->RendererUserData = nullptr;
}

static void ImGui_ImplImmortal_SetWindowSize(ImGuiViewport *viewport, ImVec2 size)
{
    ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();
    if (ImGui_ImplImmortal_ViewportData *vd = (ImGui_ImplImmortal_ViewportData *)viewport->RendererUserData)
    {
        bd->queue->WaitIdle();
        vd->swapchain->Resize(size.x, size.y);
    }
}

static void ImGui_ImplImmortal_RenderWindow(ImGuiViewport *viewport, void *render_arg)
{
    ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();
    if (ImGui_ImplImmortal_ViewportData *vd = (ImGui_ImplImmortal_ViewportData *) viewport->RendererUserData)
    {
        vd->swapchain->PrepareNextFrame();

        vd->gpuEvent->Wait(vd->syncValues[vd->syncPoint], 0xffffff);
        CommandBuffer *commandBuffer = vd->commandBuffers[vd->syncPoint];

        const ClearValue clearValue = {};
        RenderTarget *renderTarget = vd->swapchain->GetCurrentRenderTarget();

        commandBuffer->Begin();
		commandBuffer->BeginRenderTarget(renderTarget, &clearValue);
		ImGui_ImplImmortal_RenderDrawData(viewport->DrawData, commandBuffer);
        commandBuffer->EndRenderTarget();
        commandBuffer->End();

        GPUEvent *gpuEvents[] = { vd->gpuEvent };
        bd->queue->Submit(&commandBuffer, 1, gpuEvents, 1, vd->swapchain);
    }
}

static void ImGui_ImplImmortal_SwapBuffers(ImGuiViewport *viewport, void *render_arg)
{
    ImGui_ImplImmortal_Data *bd = ImGui_ImplImmortal_GetBackendData();
    if (ImGui_ImplImmortal_ViewportData *vd = (ImGui_ImplImmortal_ViewportData *) viewport->RendererUserData)
    {
        vd->syncValues[vd->syncPoint] = vd->gpuEvent->GetSyncPoint();
        SLROTATE(vd->syncPoint, vd->NumFramesInFlight);

        bd->queue->Present(vd->swapchain);
    }
}

IMGUI_IMPL_API void ImGui_ImplImmortal_InitPlatformInterface()
{
    ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_CreateWindow  = ImGui_ImplImmortal_CreateWindow;
    platform_io.Renderer_DestroyWindow = ImGui_ImplImmortal_DestroyWindow;
    platform_io.Renderer_SetWindowSize = ImGui_ImplImmortal_SetWindowSize;
    platform_io.Renderer_RenderWindow  = ImGui_ImplImmortal_RenderWindow;
    platform_io.Renderer_SwapBuffers   = ImGui_ImplImmortal_SwapBuffers;
}

IMGUI_IMPL_API void ImGui_ImplImmortal_ShutdownPlatformInterface()
{
    ImGui::DestroyPlatformWindows();
}
