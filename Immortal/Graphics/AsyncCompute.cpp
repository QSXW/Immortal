#include "AsyncCompute.h"

namespace Immortal
{

AsyncComputeThread::AsyncComputeThread(Device *device) :
    gpuEvent{},
    thread{}
{
    gpuEvent = device->CreateGPUEvent();
    thread = std::move(Thread{[=, this] {
        uint64_t recording = 0;
        uint64_t nextSyncValue = 1;
        Queue *queue = nullptr;
        CommandBuffer *commandBuffer = nullptr;

        std::unordered_map<uint64_t, CommandBuffer *> commandBuffers;

        while (true)
        {
            URef<AsyncTask> task;
            {
                std::unique_lock lock{mutex};
                condition.wait(lock, [=, this] {
                    return !tasks.empty();
                });

                task = std::move(tasks.front());
                tasks.pop();
            }

            while (!executionCompletedTasks.empty())
            {
				auto &front = executionCompletedTasks.front();
				auto &[sync, executionCompleted] = front;

                if (!IsExecutionCompleted(sync))
                {
					break;
                }

                executionCompleted.InterpretAs<ExecutionCompletedTask>()->Invoke();
				executionCompletedTasks.pop();
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
                            auto &[value, pCommandBuffer] = *commandBuffers.begin();
                            if (gpuEvent->GetCompletionValue() >= value)
                            {
                                commandBuffer = pCommandBuffer;
                                commandBuffers.erase(value);
                            }
                        }

                        if (!commandBuffer)
                        {
                            commandBuffer = device->CreateCommandBuffer();
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
                    commandBuffers[gpuEvent->GetSyncPoint()] = commandBuffer;
                    nextSyncValue = gpuEvent->GetSyncPoint() + 1;
                    commandBuffer = nullptr;
                    break;
                }

                case AsyncTaskType::ExecutionCompleted:
                {
					executionCompletedTasks.push(std::pair{nextSyncValue, std::move(task)});
                    break;
                }

                case AsyncTaskType::Terminate:
                {
                    if (commandBuffer)
                    {
                        delete commandBuffer;
                    }
                    for (auto &[value, commandBuffer] : commandBuffers)
                    {
                        delete commandBuffer;
                    }
                    std::unique_lock lock{ mutex };
                    tasks = {};
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
    uint64_t completion = gpuEvent->GetCompletionValue();
    return completion >= value;
}

void AsyncComputeThread::WaitIdle()
{
	uint64_t completion = gpuEvent->GetCompletionValue();
    if (completion < gpuEvent->GetSyncPoint())
    {
		gpuEvent->Wait(0xffffffff);
    }
}

void AsyncComputeThread::Join()
{
	thread.Join();
}

}
