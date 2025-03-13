#include "Graphics/LightGraphics.h"
#include "Shared/Log.h"

using namespace Immortal;

    const char *shaderSource = R"(
/* Copyright (c) 2023, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

struct UBO
{
	float4x4 viewProjection;
	float4x4 model;
};

cbuffer ubo : register(b0) { UBO ubo; }

struct VertexOutput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
};

static const float4 positions[3] = {
	float4( 0.0, -1.0, 0.0, 1.0),
	float4(-1.0,  1.0, 0.0, 1.0),
	float4( 1.0,  1.0, 0.0, 1.0)
};

static const float4 colors[3] = {
	float4(0.0, 1.0, 0.0, 1.0),
	float4(0.0, 0.0, 1.0, 1.0),
	float4(1.0, 0.0, 0.0, 1.0)
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void MSMain(out indices uint3 triangles[1], out vertices VertexOutput vertices[3], uint3 DispatchThreadID : SV_DispatchThreadID)
{
	float4x4 mvp = mul(ubo.viewProjection, ubo.model);

	float4 offset = float4(0.0, 0.0, (float)DispatchThreadID, 0.0);

	SetMeshOutputCounts(3, 1);
	for (uint i = 0; i < 3; i++) {
		vertices[i].position = mul(mvp, positions[i] + offset);
		vertices[i].color = colors[i];
	}

	// SetMeshOutputCounts(3, 1);
	triangles[0] = uint3(0, 1, 2);
}

/* Copyright (c) 2023, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

struct VSOutput
{
    [[vk::location(0)]] float4 color : COLOR0;
};

float4 PSMain(VSOutput input) : SV_TARGET
{
    return input.color;
}

)";

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

struct SceneConstantBuffer
{
	float viewProjection[4][4];
	float model[4][4];
};

int main(int, char **)
{
	LOG::Init();

	BackendAPI backendAPI = BackendAPI::D3D12;

	// Create a window
	uint32_t width  = 1280;
	uint32_t height = 720;
	URef<Window> window = Window::CreateInstance("Immortal Graphics HelloMeshShader Example", width, height);

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
	URef<Shader> meshShader = device->CreateShader("Mesh", ShaderStage::Mesh, shaderSource, "MSMain");
	URef<Shader> pixelShader = device->CreateShader("Pixel", ShaderStage::Pixel, shaderSource, "PSMain");

	Shader *shaders[] = {meshShader, pixelShader};
	URef<GraphicsPipeline> pipeline = device->CreateGraphicsPipeline();
	pipeline->Construct(
        // Graphics pipeline needs vertex shader and pixel shader, so we use an array to pass them
        shaders, 2,
        // No Vertex Layout in mesh shader
        {},
	    /* The render target format is B8G8R8A8 */
	    {{Format::BGRA8}});

    meshShader.Reset();
	pixelShader.Reset();

	Ref<Buffer> stagingBuffer = device->CreateBuffer(BufferType::TransferSource, sizeof(SceneConstantBuffer));
	Ref<Buffer> constantBuffer = device->CreateBuffer(BufferType::ConstantBuffer, sizeof(SceneConstantBuffer), MemoryType::Device);

	constantBuffer->SetDebugName("SceneConstantBuffer");
	URef<DescriptorSet> descriptorSet = device->CreateDescriptorSet(pipeline); 

    // Show the window. The window is not shown by default after it was created.
    window->Show();

	while (!applicationExit)
    {
		SceneConstantBuffer sceneConstantBuffer = {
			.viewProjection = {
		        { 1.0f, 0.0f, 0.0f, 0.0f },
		        { 0.0f, 1.0f, 0.0f, 0.0f },
		        { 0.0f, 0.0f, 1.0f, 0.0f },
		        { 0.0f, 0.0f, 0.0f, 1.0f }
			},
			.model = {
				{ 1.0f, 0.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f, 1.0f }
			}
		};

		stagingBuffer->Fill(&sceneConstantBuffer, sizeof(SceneConstantBuffer), 0);

        // Wait for the previsous frame
        gpuEvent->Wait(syncValues[syncPoint], 0xffffff);
        swapchain->PrepareNextFrame();

        auto &commandBuffer = commandBuffers[syncPoint];

        // begin recording commands
        commandBuffer->Begin();

		commandBuffer->MemoryCopy(constantBuffer, 0, stagingBuffer, 0, sizeof(SceneConstantBuffer));
        
		// get the rende target from swapchain for that we're going draw the triangle into the Window
        RenderTarget *renderTarget = swapchain->GetCurrentRenderTarget();

        const ClearValue clearValue = {}; //{0.0f, 0.2f, 0.4f, 1.0f};
		commandBuffer->BeginRenderTarget(renderTarget, &clearValue);

        commandBuffer->SetPipeline(pipeline);

		descriptorSet->Set(0, constantBuffer);
		commandBuffer->SetDescriptorSet(descriptorSet);
		commandBuffer->DispatchMeshTasks(1, 1, 1);

        commandBuffer->EndRenderTarget();

        // end recording commands
        commandBuffer->End();

		queue->Submit(commandBuffer, gpuEvent, swapchain);

        syncValues[syncPoint] = gpuEvent->GetSyncPoint();
        SLROTATE(syncPoint, bufferCount);

        // Submit the swapchain to the queue for presenting
        queue->Present(swapchain);

        // Poll and handle events
		window->ProcessEvents();
    }

    queue->WaitIdle();

    for (auto &commandBuffer : commandBuffers)
    {
		commandBuffer.Reset();
    }

    // Cleanup
	stagingBuffer.Reset();
	constantBuffer.Reset();
	descriptorSet.Reset();
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
