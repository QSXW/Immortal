#include "Graphics/LightGraphics.h"
#include "Shared/Log.h"

using namespace Immortal;

    const char *shaderSource = R"(
//=================================================================================================================================
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//=================================================================================================================================

// ================================================================================================================================
// D3D12 Hello Work Graphs shaders
// 
// Defines a work graph that is a chain of 3 nodes with different launch modes.
// The nodes log output to a UAV that's just an array of uints.
// 
// The C++ code seeds the graph with 4 records "entryRecord".
// The second node writes to location UAV[entryRecordIndex] and the third node writes to location UAV[4 + entryNodeIndex],
// so 8 uints modified in total.  These are printed to the console by the calling C++ code.  A simple tweak you can do
// in the C++ code is make it log more uints to the console if you want to play around with making the graph do more.
// 
// The app asks D3D to autopopulate the graph based on all nodes available, so you can play around with adding
// and changing nodes without having to change the C++ code, unless you want to tweak how the graph is seeded or how
// results are printed to console.
// 
// ================================================================================================================================
RWStructuredBuffer<uint> UAV : register(u0);        // 16MB byte buffer from global root sig

struct entryRecord
{
	uint gridSize : SV_DispatchGrid;
	uint recordIndex;
};

struct secondNodeInput
{
	uint entryRecordIndex;
	uint incrementValue;
};

struct thirdNodeInput
{
	uint entryRecordIndex;
};

static const uint c_numEntryRecords = 4;

// --------------------------------------------------------------------------------------------------------------------------------
// firstNode is the entry node, a broadcasting launch node.
//
// For each entry record, a dispatch grid is spawned with grid size from inputData.gridSize.
//
// Grid size can also be fixed for the node instead of being part of the input record,
// using [NodeDispatchGrid(x,y,z)]
//
// Each thread group sends 2 records to secondNode asking it to do some work.
// --------------------------------------------------------------------------------------------------------------------------------
[Shader("node")]
    [NodeLaunch("broadcasting")]
    [NodeMaxDispatchGrid(16, 1, 1)]        // Contrived value, input records from the app only top out at grid size of 4.
                                           // This declaration should be as accurate as possible, but not too small (undefined behavior).
    [NumThreads(2, 1, 1)] void
    firstNode(
        DispatchNodeInputRecord<entryRecord> inputData,
        [MaxRecords(2)] NodeOutput<secondNodeInput> secondNode,
        uint threadIndex : SV_GroupIndex,
                           uint dispatchThreadID : SV_DispatchThreadID) {
	    // Methods for allocating output records must be called at thread group scope (uniform call across the group)
	    // Allocations can be per thread as well: GetThreadNodeOutputRecords(...), but the call still has to be
	    // group uniform albeit with thread-varying arguments.  Thread execution doesn't have to be synchronized (Barrier call not needed).
	    GroupNodeOutputRecords<secondNodeInput> outRecs =
	        secondNode.GetGroupNodeOutputRecords(2);

	    // In a future language version, \"->\" will be available instead of \".Get()\" below to access record members
	    outRecs[threadIndex].entryRecordIndex = inputData.Get().recordIndex;                 // inputData is constant for all threads in a dispatch grid,
	                                                                                         // broadcast from input record
	    outRecs[threadIndex].incrementValue = dispatchThreadID * 2 + threadIndex + 1;        // tell consumer how much to increment UAV[entryRecordIndex]
	    outRecs.OutputComplete();                                                            // Call must be group uniform.  Thread execution doesn't have to be synchronized (Barrier call not needed).
    }

        // --------------------------------------------------------------------------------------------------------------------------------
        // secondNode is thread launch, so one thread per input record.
        //
        // Logs to the UAV and then sends a task to thirdNode
        // --------------------------------------------------------------------------------------------------------------------------------
        [Shader("node")]
        [NodeLaunch("thread")] void secondNode(
            ThreadNodeInputRecord<secondNodeInput> inputData,
            [MaxRecords(1)] NodeOutput<thirdNodeInput> thirdNode)
{
	// In a future language version, \"->\" will be available instead of \".Get()\" to access record members

	// UAV[entryRecordIndex] (as uint) is the sum of all outputs from upstream node for graph entry [entryRecordIndex]
	InterlockedAdd(UAV[inputData.Get().entryRecordIndex], inputData.Get().incrementValue);

	// For every thread send a task to thirdNode
	ThreadNodeOutputRecords<thirdNodeInput> outRec = thirdNode.GetThreadNodeOutputRecords(1);
	outRec.Get().entryRecordIndex = inputData.Get().entryRecordIndex;
	outRec.OutputComplete();
}

groupshared uint g_sum[c_numEntryRecords];

// --------------------------------------------------------------------------------------------------------------------------------
// thirdNode is coalescing launch, so thread groups are launched with up to 32 records as input in an array.
//
// The thread group size happens to match this max input array size of 32, but doesn't have to.
// --------------------------------------------------------------------------------------------------------------------------------
[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(32,1,1)]
void thirdNode(
    [MaxRecords(32)] GroupNodeInputRecords<thirdNodeInput> inputData,
    uint threadIndex : SV_GroupIndex)
{
	// Check how many records we got
	// It could be less than the max declared if the system doesn't have that many
	// work items left, or if it doesn't want to wait for more records to arrive at this node before
	// flushing the current work.
	if (threadIndex >= inputData.Count())
		return;

	for (uint i = 0; i < c_numEntryRecords; i++)
	{
		g_sum[i] = 0;
	}

	// New way to do barriers by parameter.
	// This instance is like GroupMemoryBarrierWithGroupSync();
	Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);

	InterlockedAdd(g_sum[inputData[threadIndex].entryRecordIndex], 1);

	Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);

	if (threadIndex > 0)
		return;

	for (uint l = 0; l < c_numEntryRecords; l++)
	{
		uint recordIndex = c_numEntryRecords + l;
		InterlockedAdd(UAV[recordIndex], g_sum[l]);
	}
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
	URef<Window> window = Window::CreateInstance("Immortal Graphics HelloWorkGraph Example", width, height);

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

	const uint32_t bufSizeInUints = 16777216;
	const uint32_t bufSize = bufSizeInUints * sizeof(uint32_t);
	std::vector<uint32_t> initialData(bufSizeInUints, 0);

	Ref<Buffer> gpuBuffer = device->CreateBuffer(BufferType::Storage, bufSize, MemoryType::Device, sizeof(uint32_t));
	Ref<Buffer> stagingBuffer = device->CreateBuffer(BufferType::TransferSource, bufSize);
	stagingBuffer->Fill(initialData.data(), bufSize, 0);

	Ref<Buffer> readbackBuffer = device->CreateBuffer(BufferType::TransferDestination, bufSize);

	URef<Shader> workgraphLibrary = device->CreateShader("HelloWorkGraph", ShaderStage::WorkGraph, shaderSource, {});
	URef<ComputePipeline> pipeline = device->CreateComputePipeline(workgraphLibrary);
	Ref<DescriptorSet> decriptorSet = device->CreateDescriptorSet(pipeline);
	decriptorSet->Set(0, gpuBuffer);

	auto &commandBuffer = commandBuffers[0];
	window->Show();
	while (true)
	{
		swapchain->PrepareNextFrame();

		commandBuffer->Begin();
		commandBuffer->SetPipeline(pipeline);
		commandBuffer->SetDescriptorSet(decriptorSet);
		// Generate graph inputs
		struct EntryRecord        // equivalent to the definition in HLSL code
		{
			uint32_t gridSize;        // : SV_DispatchGrid;
			uint32_t recordIndex;
		};

		std::vector<EntryRecord> inputData;
		UINT numRecords = 4;
		inputData.resize(numRecords);
		for (UINT recordIndex = 0; recordIndex < numRecords; recordIndex++)
		{
			inputData[recordIndex].gridSize = recordIndex + 1;
			inputData[recordIndex].recordIndex = recordIndex;
		}

		DispatchGraphDescription desc = {
		    .mode = DispatchMode::NodeCpuInput,
		    .nodeCpuInput = {
		        .entrypointIndex     = 0,
		        .numRecords          = numRecords,
		        .pRecords            = inputData.data(),
		        .recordStrideInBytes = sizeof(EntryRecord),
		    }};

		commandBuffer->MemoryCopy(gpuBuffer, 0, stagingBuffer, 0, bufSize);
		commandBuffer->DispatchGraph(&desc);

		ClearValue clearValue = {};
		commandBuffer->BeginRenderTarget(swapchain->GetCurrentRenderTarget(), &clearValue);
		commandBuffer->EndRenderTarget();

		commandBuffer->End();
		queue->Submit(commandBuffer, gpuEvent, swapchain);
		queue->Present(swapchain);
		queue->WaitIdle();

		commandBuffer->Begin();
		commandBuffer->MemoryCopy(readbackBuffer, 0, gpuBuffer, 0, bufSize);
		commandBuffer->End();
		queue->Submit(commandBuffer, gpuEvent);
		queue->WaitIdle();

		uint32_t *pDataOutput;
		readbackBuffer->Map((void **)&pDataOutput, bufSize, 0);
		if (pDataOutput)
		{
			UINT numUintsToPrint = numRecords * 2;
			std::cout << ">>> Dumping " << numUintsToPrint << " uints from UAV:\n";
			for (UINT i = 0; i < numUintsToPrint; i++)
			{
				std::cout << "    UAV[" << i << "] = 0x" << std::hex << pDataOutput[i] << std::endl;
			}
			readbackBuffer->Unmap();
			std::cout << "\n"
				  << "==================================================================================\n"
				  << " Execution complete\n"
				  << "=================================================================================="
				  << std::endl;
		}
	
		window->ProcessEvents();
	}
    for (auto &commandBuffer : commandBuffers)
    {
		commandBuffer.Reset();
    }

    // Cleanup
	gpuBuffer.Reset();
	readbackBuffer.Reset();
	decriptorSet.Reset();
	stagingBuffer.Reset();
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
