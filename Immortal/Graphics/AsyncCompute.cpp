#include "AsyncCompute.h"
#include "Coroutine.h"

namespace Immortal
{

AsyncComputeThread::AsyncComputeThread(Device *device) :
    thread{}
{
    thread = std::move(Thread{[=, this] {
        uint64_t recording = 0;
        uint64_t nextSyncValue = 1;
        Queue *queue = nullptr;
        CommandBuffer *commandBuffer = nullptr;
        GPUEvent *gpuEvent = nullptr;

        std::queue<std::pair<GPUEvent *, CommandBuffer *>> commandBuffers;

        while (true)
        {
            URef<AsyncTask> task;
            {
				semaphore.wait();
                if (!tasks.try_dequeue(task))
                {
					continue;
                }

     //           std::unique_lock lock{mutex};
     //           condition.wait(lock, [=, this] {
					//return !tasks();
     //           });

     //           task = std::move(tasks.front());
     //           tasks.pop();
            }

            switch (task->GetType())
            {
                case AsyncTaskType::SetQueue:
                {
                    SetQueueTask *setQueueTask = task.InterpretAs<SetQueueTask>();
                    queue = setQueueTask->GetQueue();
                    break;
                }

                case AsyncTaskType::QueueOperation:
                {
                    QueueTask *queueTask = task.InterpretAs<QueueTask>();
                    queueTask->QueueOperation(queue);
                    break;
                }

                case AsyncTaskType::Recording:
                {
                    recording++;
                    RecordingTask *recordingTask = task.InterpretAs<RecordingTask>();
                    recordingTask->Recording(nextSyncValue, commandBuffer);
                    break;
                }

                case AsyncTaskType::BeginRecording:
                {
                    if (!commandBuffer)
                    {
                        if (!commandBuffers.empty())
                        {
                            auto &[pGPUEvent, pCommandBuffer] = commandBuffers.front();
							if (pGPUEvent->GetCompletionValue() >= pGPUEvent->GetSyncPoint())
                            {
                                commandBuffer = pCommandBuffer;
                                gpuEvent      = pGPUEvent;
								commandBuffers.pop();
                            }
                        }

                        if (!commandBuffer)
                        {
							SLASSERT(queue != nullptr && "The queue must have set before invoke any recording tasks");
                            commandBuffer = device->CreateCommandBuffer(queue->GetType());
							gpuEvent = device->CreateGPUEvent();
							LOG::INFO("Allocate command buffer {}", (void *) commandBuffer);
                        }
                    }

                    commandBuffer->Begin();
                    break;
                }

                case AsyncTaskType::EndRecording:
                {
                    SLASSERT(commandBuffer && "CommandBuffer isn't begined!");
                    commandBuffer->End();
                    break;
                }

                case AsyncTaskType::Submiting:
                {
                    if (!recording)
                    {
                        break;
                    }
                    recording = 0;
                    SLASSERT(commandBuffer && "CommandBuffer is not able to submit!");
                    queue->Submit(commandBuffer, gpuEvent);

                    if (!executionCompletedTasks.empty())
                    {
						auto onCompletedTasks = std::make_shared <std::vector<std::pair<uint64_t, URef<AsyncTask>>>>(std::move(executionCompletedTasks));
						//Async::Execute([=, this] {
						//	gpuEvent->Wait(std::numeric_limits<uint64_t>::max());
						//	for (auto &[sync, executionCompleted] : *onCompletedTasks)
						//	{
						//		executionCompleted.InterpretAs<ExecutionCompletedTask>()->Invoke();
						//	}
						//});

                        Coroutine h = [=, this]() -> Coroutine
						{
						    gpuEvent->Wait(std::numeric_limits<uint64_t>::max());
						    for (auto &[sync, executionCompleted] : *onCompletedTasks)
						    {
							    executionCompleted.InterpretAs<ExecutionCompletedTask>()->Invoke();
						    }
							co_return;
						}();
						h.resume();
						h.destroy();
                    }

                    commandBuffers.push({ gpuEvent, commandBuffer });
					commandBuffer = nullptr;
                    gpuEvent      = nullptr;

                    break;
                }

                case AsyncTaskType::ExecutionCompleted:
                {
					executionCompletedTasks.emplace_back(std::pair{nextSyncValue, std::move(task)});
                    break;
                }

                case AsyncTaskType::Terminate:
                {
                    if (commandBuffer)
                    {
                        delete commandBuffer;
                    }

                    while (!commandBuffers.empty())
                    {
						auto &[gpuEvent, commandBuffer] = commandBuffers.front();
						delete gpuEvent;
						delete commandBuffer;
						commandBuffers.pop();
                    }

                    //std::unique_lock lock{ mutex };
                    //tasks = {};
                    return;
                }

                default:
                    break;
            }
            task.Reset();
        }
    }});
    thread.SetDescription("AsyncComputeThread");
    thread.Start();
}

bool AsyncComputeThread::IsExecutionCompleted(uint64_t value)
{
    //uint64_t completion = gpuEvent->GetCompletionValue();
    //return completion >= value;
	return true;
}

void AsyncComputeThread::WaitIdle()
{
	//uint64_t completion = gpuEvent->GetCompletionValue();
 //   if (completion < gpuEvent->GetSyncPoint())
 //   {
	//	gpuEvent->Wait(0xffffffff);
 //   }
}

void AsyncComputeThread::Join()
{
	thread.Join();
}

}
