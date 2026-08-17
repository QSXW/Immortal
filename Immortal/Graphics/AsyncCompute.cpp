#include "AsyncCompute.h"
#include "Coroutine.h"

#include <exception>
#include <future>

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

static void InvokeExecutionCompletedTasks(std::vector<std::pair<uint64_t, URef<AsyncTask>>> &tasks)
{
	for (auto &[sync, executionCompleted] : tasks)
	{
		try
		{
			(*executionCompleted.InterpretAs<ExecutionCompletedTask>())();
		}
		catch (const std::exception &exception)
		{
			LOG::ERR("Async compute completion {} failed: {}", sync, exception.what());
		}
		catch (...)
		{
			LOG::ERR("Async compute completion {} failed with an unknown exception", sync);
		}
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
                    if (!commandBuffer)
                    {
                        LOG::ERR("Async compute recording task was submitted outside a recording batch");
                        break;
                    }
                    RecordingTask *recordingTask = task.InterpretAs<RecordingTask>();
                    recordingTask->Recording(commandBuffer);
					recording++;
                    break;
                }

                case AsyncTaskType::BeginRecording:
                {
                    if (!queue)
                    {
                        LOG::ERR("Async compute cannot begin recording before a queue is assigned");
                        break;
                    }
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
                            commandBuffer = device->CreateCommandBuffer(queue->GetType());
							gpuEvent = device->CreateGPUEvent();
							if (!commandBuffer || !gpuEvent)
							{
								LOG::ERR("Async compute failed to allocate a command buffer or GPU event");
								delete commandBuffer;
								delete gpuEvent;
								commandBuffer = nullptr;
								gpuEvent = nullptr;
								break;
							}
							CLOG_DEBUG("Allocate CommandBuffer@{}", (void *)commandBuffer);
                        }
                    }
					else
					{
						LOG::ERR("Async compute received a nested begin-recording request");
						break;
                    }

                    status = AsyncTaskType::BeginRecording;
                    commandBuffer->Begin();
                    break;
                }

                case AsyncTaskType::EndRecording:
                {
					if (!commandBuffer)
					{
						LOG::ERR("Async compute cannot end recording without a command buffer");
						break;
					}
					if (status == AsyncTaskType::EndRecording)
					{
						LOG::ERR("Async compute received a duplicate end-recording request");
						break;
					}
                    
                    status = AsyncTaskType::EndRecording;
                    commandBuffer->End();
                    break;
                }

				case AsyncTaskType::Submiting:
                {
					status = AsyncTaskType::Submiting;
					SLASSERT(commandBuffer && "CommandBuffer is not able to submit!");
					SLASSERT(gpuEvent && "GPUEvent is not able to submit!");
					SLASSERT(queue && "Queue is not able to submit!");

					if (recording)
                    {
						queue->Submit(commandBuffer, gpuEvent);
						if (!executionCompletedTasks.empty())
						{
							uint64_t syncValue = gpuEvent->GetSyncPoint();
							auto onCompletedTasks = std::make_shared<std::vector<std::pair<uint64_t, URef<AsyncTask>>>>(std::move(executionCompletedTasks));
							executionCompletedThread.Enqueue([=, this] {
								gpuEvent->Wait(syncValue, kMaxTimeOut);
								InvokeExecutionCompletedTasks(*onCompletedTasks);
							});
						}
                    }
					else if (!executionCompletedTasks.empty())
					{
						auto onCompletedTasks = std::make_shared<std::vector<std::pair<uint64_t, URef<AsyncTask>>>>(std::move(executionCompletedTasks));
						InvokeExecutionCompletedTasks(*onCompletedTasks);
                    }
					recording = 0;

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
					executionCompletedTasks.emplace_back(std::pair{nextSyncValue++, std::move(task)});
                    break;
                }

                case AsyncTaskType::Terminate:
                {
					if (queue)
					{
						queue->WaitIdle();
					}
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
	auto queueBarrier = std::make_shared<std::promise<void>>();
	auto queueBarrierFuture = queueBarrier->get_future();
	SubmitStandaloneCompletion([queueBarrier] {
		queueBarrier->set_value();
	});
	queueBarrierFuture.wait();
}

void AsyncComputeThread::SubmitStandaloneCompletion(std::function<void()> callback)
{
	if (!callback)
	{
		return;
	}

	// This explicit recording task makes the batch submit a queue fence, keeping
	// the callback asynchronous and ordered after submissions queued before it.
	AsyncRecordingScope completionBatch{ this };
	Execute<RecordingTask>([](CommandBuffer *) {});
	Execute<ExecutionCompletedTask>(std::move(callback));
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
