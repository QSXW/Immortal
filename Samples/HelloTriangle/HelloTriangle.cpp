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

int main(int, char **)
{
	LOG::Init();

	BackendAPI backendAPI = BackendAPI::Vulkan;

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

    URef<Buffer> vertexBuffer = device->CreateBuffer(sizeof(triangleVertices), BufferType::Vertex);

    // copy the vertices data to the vertex buffer
	void *data = nullptr;
	vertexBuffer->Map(&data, sizeof(triangleVertices), 0);
    memcpy(data, triangleVertices, sizeof(triangleVertices));
	vertexBuffer->Unmap();

    // Show the window. The window is not shown by default after it was created.
    window->Show();

	while (!applicationExit)
    {
        // Wait for the previsous frame
        gpuEvent->Wait(syncValues[syncPoint], 0xffffff);
        swapchain->PrepareNextFrame();

        auto &commandBuffer = commandBuffers[syncPoint];

        // begin recording commands
        commandBuffer->Begin();

        // get the rende target from swapchain for that we're going draw the triangle into the Window
        RenderTarget *renderTarget = swapchain->GetCurrentRenderTarget();

        const float clearColor[4] = {}; //{0.0f, 0.2f, 0.4f, 1.0f};
		commandBuffer->BeginRenderTarget(renderTarget, clearColor);

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
