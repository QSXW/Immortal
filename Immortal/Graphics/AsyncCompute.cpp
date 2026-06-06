#include "AsyncCompute.h"
#include "Coroutine.h"

namespace Immortal
{

const char *GetTaskTypeStr(AsyncTaskType type)
{
	switch (type)
    {
        case AsyncTaskType::Recording:         return "Recording";
        case AsyncTaskType::Submiting:         return "Submiting";
        case AsyncTaskType::SetQueue:          return "SetQueue";
        case AsyncTaskType::QueueOperation:    return "QueueOperation";
        case AsyncTaskType::BeginRecording:    return "BeginRecording";
        case AsyncTaskType::EndRecording:      return "EndRecording";
        case AsyncTaskType::ExecutionCompleted:return "ExecutionCompleted";
        case AsyncTaskType::Terminate:         return "Terminate";
        default:                               return "Unknown";
	}
}

AsyncComputeThread::AsyncComputeThread(Device *device) :
    ICLASS,
    thread{}
{
    thread = std::move(Thread{[=, this] {
        uint64_t recording = 0;
        uint64_t nextSyncValue = 1;
        Queue *queue = nullptr;
        CommandBuffer *commandBuffer = nullptr;
        GPUEvent *gpuEvent = nullptr;
        
        AsyncTaskType status = AsyncTaskType::ExecutionCompleted;
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
            }

            //LOG_INFO("AsyncComputeThread: Execute Task Type: {}", GetTaskTypeStr(task->GetType()));
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
                    recordingTask->Recording(commandBuffer);
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
							CLOG_DEBUG("Allocate CommandBuffer@{}", (void *)commandBuffer);
                        }
                    }
					else
					{
						SLASSERT(false && "Double command buffer begin!");
                    }

                    status = AsyncTaskType::BeginRecording;
                    commandBuffer->Begin();
                    break;
                }

                case AsyncTaskType::EndRecording:
                {
					SLASSERT(commandBuffer && "CommandBuffer isn't begined!");
					SLASSERT(status != AsyncTaskType::EndRecording && "CommandBuffer is ended!");
                    
                    status = AsyncTaskType::EndRecording;
                    commandBuffer->End();
                    break;
                }

                case AsyncTaskType::Submiting:
                {
					status = AsyncTaskType::Submiting;
                    if (!recording)
					{
						auto onCompletedTasks = std::make_shared<std::vector<std::pair<uint64_t, URef<AsyncTask>>>>(std::move(executionCompletedTasks));
						for (auto &[sync, executionCompleted] : *onCompletedTasks)
						{
							(*executionCompleted.InterpretAs<ExecutionCompletedTask>())();
						}
                    }
					{
						recording = 0;
						SLASSERT(commandBuffer && "CommandBuffer is not able to submit!");
						queue->Submit(commandBuffer, gpuEvent);
                        if (!executionCompletedTasks.empty())
                        {
						    uint64_t syncValue = gpuEvent->GetSyncPoint();
						    auto onCompletedTasks = std::make_shared <std::vector<std::pair<uint64_t, URef<AsyncTask>>>>(std::move(executionCompletedTasks));
						    executionCompletedThread.Enqueue([=, this] {
							    gpuEvent->Wait(syncValue, kMaxTimeOut);
							    for (auto &[sync, executionCompleted] : *onCompletedTasks)
							    {
								    (*executionCompleted.InterpretAs<ExecutionCompletedTask>())();
							    }
						    });
                        }
                    }

                    if (commandBuffers.size() < 4)
					{
						commandBuffers.push({gpuEvent, commandBuffer});
					    commandBuffer = nullptr;
                        gpuEvent      = nullptr;
                    }
                    else
					{
						CLOG_DEBUG("Release CommandBuffer@{}", (void *)commandBuffer);
						executionCompletedThread.Enqueue([gpuEvent, commandBuffer] {
							gpuEvent->Wait(kMaxTimeOut);
							delete gpuEvent;
							delete commandBuffer;
						});
						commandBuffer = nullptr;
						gpuEvent = nullptr;
                    }
                    break;
                }

                case AsyncTaskType::ExecutionCompleted:
                {
					executionCompletedTasks.emplace_back(std::pair{nextSyncValue, std::move(task)});
                    break;
                }

                case AsyncTaskType::Terminate:
                {
					queue->WaitIdle();
                    if (commandBuffer)
                    {
                        delete commandBuffer;
						commandBuffer = nullptr;
                    }

                    if (gpuEvent)
                    {
						delete gpuEvent;
						gpuEvent = nullptr;
                    }

                    while (!commandBuffers.empty())
                    {
						auto &[gpuEvent, commandBuffer] = commandBuffers.front();
						delete gpuEvent;
						delete commandBuffer;
						commandBuffers.pop();
                    }

                    return;
                }

                default:
                    break;
            }
            task.Reset();
        }
    }});
    thread.SetDescription("AsyncComputeThread");
}

AsyncComputeThread::~AsyncComputeThread()
{
	Execute<AsyncTask>(AsyncTaskType::Terminate);
	thread.Join();
}

bool AsyncComputeThread::IsExecutionCompleted(uint64_t value)
{
	return true;
}

void AsyncComputeThread::WaitIdle()
{

}

void AsyncComputeThread::Join()
{
	thread.Join();
}

void AsyncComputeThread::SetDescription(const std::string &description)
{
	thread.SetDebugDescription(description);
}

}
