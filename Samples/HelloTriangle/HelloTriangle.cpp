#include "Graphics/LightGraphics.h"
#include "Shared/Log.h"

using namespace Immortal;

    const char *shaderSource = R"(
//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = position;
    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
)";

struct Vertex
{
	float position[3];
	float color[4];
};

URef<Queue> queue;
URef<Swapchain> swapchain;

uint32_t bufferCount = 3;
uint32_t syncPoint = 0;
uint32_t syncValues[3] = {};
bool applicationExit = false;

void OnEvent(Event &e)
{
    if (e.GetType() == Event::Type::WindowResize)
    {
        // We need to waiting the queue to be idle before resize the swapchain
        queue->WaitIdle(0xffffff);
        auto &resizeEvent = (WindowResizeEvent &)e;
        swapchain->Resize(resizeEvent.Width(), resizeEvent.Height());
    }
    if (e.GetType() == Event::Type::WindowClose)
    {
		applicationExit = true;
    }
}

std::string ReadFileToString(const std::string &filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		return "";
	}
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	return content;
}    

int main(int, char **)
{
	LOG::Init();

	BackendAPI backendAPI = BackendAPI::D3D12;

	// Create a window
	uint32_t width  = 1280;
	uint32_t height = 720;
	URef<Window> window = Window::CreateInstance("Immortal Graphics HelloTriangle Example", width, height);

    // Bind OnEvent function into the window
	window->SetEventCallback(OnEvent);

	// Create physical device from GPU Id
	URef<Instance> instance = Instance::CreateInstance(backendAPI, window->GetType());

	// Create a logic device from the physical device
	URef<Device> device = instance->CreateDevice(0);

	// Create a graphics queue
	queue = device->CreateQueue(Queue::Type::Graphics);

    // Create a GPU event for CPU-GPU or GPU synchronization
	Ref<GPUEvent> gpuEvent = device->CreateGPUEvent();

    // Use the logic device to create a swapchain
	constexpr uint32_t swapchainBufferCount = 3;
	swapchain = device->CreateSwapchain(queue, window, Format::BGRA8, swapchainBufferCount, SwapchainMode::None);

    // Create command buffers for recording commands
    // The command buffer cannot be reused when it is executing, so we need to
    // create the same number of command buffer as swapchain buffer count
	URef<CommandBuffer> commandBuffers[swapchainBufferCount];
	for (size_t i = 0; i < swapchainBufferCount; i++)
	{
		commandBuffers[i] = device->CreateCommandBuffer();
	}

	// Create vertex and pixel(fragment) shader from shader source codes
    // The Vulkan and D3D12 both use the HLSL as the shader source language
	URef<Shader> vertexShader = device->CreateShader("Vertex", ShaderStage::Vertex, shaderSource, "VSMain");
	URef<Shader> pixelShader = device->CreateShader("Pixel", ShaderStage::Pixel, shaderSource, "PSMain");

	Shader *shaders[] = { vertexShader, pixelShader };
	URef<GraphicsPipeline> pipeline = device->CreateGraphicsPipeline();
	pipeline->Construct(
        // Graphics pipeline needs vertex shader and pixel shader, so we use an array to pass them
        shaders, 2,
        // Vertex Layout
        {
            { Format::VECTOR3, "POSITION" },
            { Format::VECTOR4, "COLOR"    }
        },
	    /* The render target format is B8G8R8A8 */
	    {{Format::BGRA8}});

    vertexShader.Reset();
	pixelShader.Reset();

    float aspectRatio = static_cast<float>(window->GetWidth()) / static_cast<float>(window->GetHeight());
    // Create vertex buffer
    Vertex triangleVertices[] =
    {
        { {   0.0f,  0.25f * aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    URef<Buffer> vertexBuffer = device->CreateBuffer(BufferType::Vertex, sizeof(triangleVertices));

    // copy the vertices data to the vertex buffer
	void *data = nullptr;
	vertexBuffer->Map(&data, sizeof(triangleVertices), 0);
    memcpy(data, triangleVertices, sizeof(triangleVertices));
	vertexBuffer->Unmap();

    // Show the window. The window is not shown by default after it was created.
    window->Show();

	uint32_t imageWidth = 4096;
	uint32_t imageHeight = 2176;
	Ref<Texture> texture = device->CreateTexture(Format::R16_UINT, imageWidth, imageHeight, 1, 1, TextureType::Storage);

    struct IntraPredictionPlanarParams
	{
		int x;
		int y;
		int w;
		int h;
		int logw;
		int logh;
		int top_offset;
		int left_offset;
		int need_pdbc;
	};

	std::vector<uint16_t> tops;
	tops.reserve(64 * 64);

	std::vector<uint16_t> lefts;
	lefts.reserve(64 * 64);

	std::vector<IntraPredictionPlanarParams> params;
	for (int y = 0; y < imageHeight; y += 64)
	{
		for (int x = 0; x < imageWidth; x += 64)
		{
			params.emplace_back(IntraPredictionPlanarParams {
				.x = x,
				.y = y,
			    .w = 64,
			    .h = 64,
			    .logw = (int) std::log2(64),
			    .logh = (int) std::log2(64),
			    .top_offset = (int)tops.size(),
			    .left_offset = (int) lefts.size(),
			    .need_pdbc = 0}
			);

			tops.resize(tops.size() + 64 + 1);
			for (int i = params.back().top_offset; i < tops.size(); i++)
			{
				tops[i] = 1 << (10 - 1);
			}
			lefts.resize(lefts.size() + 64 + 1);
			for (int i = params.back().left_offset; i < lefts.size(); i++)
			{
				lefts[i] = 1 << (10 - 1);
			}
		}
	}

    auto intraPlanarShaderSource = ReadFileToString("C:/SDK/C/Montage/Assets/Shaders/hlsl/intra_palanar.hlsl");

    URef<Shader> intraPlanarShader = device->CreateShader("intra_planar", ShaderStage::Compute, intraPlanarShaderSource, "pred_planar");
	URef<ComputePipeline> computePipeline = device->CreateComputePipeline(intraPlanarShader);

	auto dequantShaderSource = ReadFileToString("C:/SDK/C/Montage/Assets/Shaders/hlsl/dequant.hlsl");
	URef<Shader> dequantShader = device->CreateShader("dequant", ShaderStage::Compute, dequantShaderSource, "dequant");
	URef<ComputePipeline> dequantPipeline = device->CreateComputePipeline(dequantShader);

	Ref<Buffer> stagingCoeffs = device->CreateBuffer(BufferType::TransferSource, (SLALIGN(imageWidth, TextureAlignment) * sizeof(uint32_t)) * imageHeight, MemoryType::Host);
	Ref<Buffer> readbackCoeffs = device->CreateBuffer(BufferType::TransferDestination, (SLALIGN(imageWidth, TextureAlignment) * sizeof(uint32_t)) * imageHeight, MemoryType::Host);
	Ref<Texture> coeffs = device->CreateTexture(Format::R32_SINT, imageWidth, imageHeight, 1, 1, TextureType::Storage);

	Ref<Buffer> stagingScale = device->CreateBuffer(BufferType::TransferSource, (SLALIGN(imageWidth, TextureAlignment) * sizeof(int)) * imageHeight, MemoryType::Host);
	Ref<Buffer> scale = device->CreateBuffer(BufferType::Storage, stagingScale->GetSize(), MemoryType::Device, sizeof(int));

	{
		int *coeffs;
		stagingCoeffs->Map((void **) &coeffs, stagingCoeffs->GetSize(), 0);

		static int testCoeffs[32 * 8] = {
			+35, +4, -1,  -1, +2, +1, +1,  +0,
			 +0, -1, +2,  +0, +1, +0, +0,  +1,
			 +0, -1, +0,  +0, +0, +0, +0,  +0,
			 +0, +0, +0,  +0, +0, +0, +0,  +0,
		};

		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				coeffs[y * 512 + x] = testCoeffs[y * 32 + x];
			}
		}
		stagingCoeffs->Unmap();

		int *pScale;
		stagingScale->Map((void **) &pScale, stagingScale->GetSize(), 0);

		for (int y = 0; y < 240; y++)
		{
			for (int x = 0; x < 416; x++)
			{
				pScale[y * 512 + x] = 16;
			}
		}
		stagingScale->Unmap();
	}

	struct Constant
	{
		int width;
		int height;
	} c{texture->GetWidth(), texture->GetHeight()};

    uint32_t paramsSize = sizeof(IntraPredictionPlanarParams) * params.size();
	Ref<Buffer> stagingPredPlanarParams = device->CreateBuffer(BufferType::TransferSource, paramsSize, MemoryType::Host);
	Ref<Buffer> predPlanarParams = device->CreateBuffer(BufferType::Storage, paramsSize, MemoryType::Device, sizeof(IntraPredictionPlanarParams));

    Ref<DescriptorSet> descriptorSet = device->CreateDescriptorSet(computePipeline);

    Ref<Buffer> stagingTop = device->CreateBuffer(BufferType::TransferSource, tops.size() * sizeof(uint16_t), MemoryType::Host);
	Ref<Buffer> top = device->CreateBuffer(BufferType::Storage, stagingTop->GetSize(), MemoryType::Device, sizeof(uint16_t));
	stagingTop->Fill(tops.data(), tops.size() * sizeof(uint16_t), 0);

	Ref<Buffer> stagingLeft = device->CreateBuffer(BufferType::TransferSource, lefts.size() * sizeof(uint16_t), MemoryType::Host);
	Ref<Buffer> left = device->CreateBuffer(BufferType::Storage, stagingLeft->GetSize(), MemoryType::Device, sizeof(uint16_t));
	stagingLeft->Fill(lefts.data(), lefts.size() * sizeof(uint16_t), 0);

	struct DequantParams
	{
		int x;
		int y;
		int w;
		int h;
		int min_scan_x;
		int max_scan_x;
		int min_scan_y;
		int max_scan_y;
		int scale;
		int bd_shift;
		int log2_transform_range;
		int scale_m_offset;
	};

	std::vector<DequantParams> dequantParams;
	dequantParams.emplace_back(DequantParams{
	    .x = 0,
	    .y = 0,
	    .w = 32,
	    .h = 8,
	    .min_scan_x = 0,
	    .max_scan_x = 17,
	    .min_scan_y = 0,
	    .max_scan_y = 6,
		.scale = 5120,
	    .bd_shift = 10,
	    .log2_transform_range = 15,
		.scale_m_offset = 0,
	});
	
	uint32_t dequantParamsSize = sizeof(DequantParams) * dequantParams.size();
	Ref<Buffer> stagingDequantParams = device->CreateBuffer(BufferType::TransferSource, dequantParamsSize);
	Ref<Buffer> dequantParamsBuffer = device->CreateBuffer(BufferType::Storage, dequantParamsSize, MemoryType::Device, sizeof(DequantParams));

	descriptorSet->Set(0, texture);
	descriptorSet->Set(1, top);
	descriptorSet->Set(2, left);
	descriptorSet->Set(3, predPlanarParams);

	Ref<Buffer> stagingImage = device->CreateBuffer(BufferType::TransferDestination, SLALIGN(texture->GetWidth() * sizeof(uint16_t), TextureAlignment) * texture->GetHeight(), MemoryType::Host);

	Ref<DescriptorSet> dequantDescriptorSet = device->CreateDescriptorSet(dequantPipeline);
	dequantDescriptorSet->Set(0, coeffs);
	dequantDescriptorSet->Set(1, scale);
	dequantDescriptorSet->Set(2, dequantParamsBuffer);

	while (!applicationExit)
    {
        // Wait for the previsous frame
        gpuEvent->Wait(syncValues[syncPoint], 0xffffff);
        swapchain->PrepareNextFrame();

        auto &commandBuffer = commandBuffers[syncPoint];

        // begin recording commands
        commandBuffer->Begin();

        stagingPredPlanarParams->Fill(params.data(), paramsSize, 0);
		commandBuffer->MemoryCopy(predPlanarParams, 0, stagingPredPlanarParams, 0, paramsSize);
		commandBuffer->MemoryCopy(left, 0, stagingLeft, 0, left->GetSize());
		commandBuffer->MemoryCopy(top, 0, stagingTop, 0, top->GetSize());

		commandBuffer->MemoryCopy(scale, 0, stagingScale, 0, scale->GetSize());
		commandBuffer->CopyBufferToImage(coeffs, 0, stagingCoeffs, SLALIGN(coeffs->GetWidth(), TextureAlignment) * sizeof(uint32_t), 0);

		// intra prediction
		commandBuffer->SetPipeline(computePipeline);
		commandBuffer->SetDescriptorSet(descriptorSet);
		//commandBuffer->PushConstants(ShaderStage::Compute, &c, sizeof(c), 0);
		commandBuffer->Dispatch(params.size(), 1, 1);
		// commandBuffer->Dispatch(texture->GetWidth(), texture->GetHeight(), 1);
		commandBuffer->CopyImageToBuffer(stagingImage, texture, 0, SLALIGN(texture->GetWidth() * sizeof(uint16_t), TextureAlignment));

		// dequant
		stagingDequantParams->Fill(dequantParams.data(), dequantParamsSize, 0);
		commandBuffer->MemoryCopy(dequantParamsBuffer, 0, stagingDequantParams, 0, dequantParamsSize);
		commandBuffer->SetPipeline(dequantPipeline);
		commandBuffer->SetDescriptorSet(dequantDescriptorSet);
		commandBuffer->Dispatch(dequantParams.size(), 1, 1);
		commandBuffer->CopyImageToBuffer(readbackCoeffs, coeffs, 0, SLALIGN(texture->GetWidth(), TextureAlignment) * sizeof(int));

        // get the rende target from swapchain for that we're going draw the triangle into the Window
        RenderTarget *renderTarget = swapchain->GetCurrentRenderTarget();

        const ClearValue clearValue = {}; //{0.0f, 0.2f, 0.4f, 1.0f};
		commandBuffer->BeginRenderTarget(renderTarget, &clearValue);

        commandBuffer->SetPipeline(pipeline);
		Buffer *vertexBuffers[] = { vertexBuffer };
		commandBuffer->SetVertexBuffers(0, 1, vertexBuffers, sizeof(Vertex));
		commandBuffer->DrawInstanced(3, 1, 0, 0);

        commandBuffer->EndRenderTarget();

        // end recording commands
        commandBuffer->End();

		queue->Submit(commandBuffer, gpuEvent, swapchain);

        syncValues[syncPoint] = gpuEvent->GetSyncPoint();
        SLROTATE(syncPoint, bufferCount);

        // Submit the swapchain to the queue for presenting
        queue->Present(swapchain);

		int *pData;
		stagingImage->Map((void **) &pData, stagingImage->GetSize(), 0);
		stagingImage->Unmap();

		int *pCoeffs;
		readbackCoeffs->Map((void **) &pCoeffs, stagingImage->GetSize(), 0);
		readbackCoeffs->Unmap();
        // Poll and handle events
		window->ProcessEvents();
    }

    queue->WaitIdle();

    for (auto &commandBuffer : commandBuffers)
    {
		commandBuffer.Reset();
    }

    // Cleanup
	vertexBuffer.Reset();
	pipeline.Reset();
	gpuEvent.Reset();
	queue.Reset();
	swapchain.Reset();
	gpuEvent.Reset();
	device.Reset();
	instance.Reset();

    // We need to release the log because the Memory Allocator will track the memory usage
	LOG::Release();

    return 0;
}
