#pragma once

#include "Shared/Async.h"
#include "Queue.h"
#include "LightGraphics.h"

namespace Immortal
{

enum class AsyncTaskType
{
    Recording,
    Submiting,
    SetQueue,
    QueueOperation,
    BeginRecording,
    EndRecording,
    ExecutionCompleted,
    Terminate,
};

class AsyncTask
{
public:
	AsyncTask(AsyncTaskType type) :
        type{ type }
    {

    }

    virtual ~AsyncTask()
    {

    }

    void SetType(AsyncTaskType value)
    {
        type = value;
    }

    AsyncTaskType GetType() const
    {
        return type;
    }

protected:
	AsyncTaskType type;
};

class SetQueueTask : public AsyncTask
{
public:
    SetQueueTask(Queue *queue = nullptr) :
	    AsyncTask{ AsyncTaskType::SetQueue },
        queue{queue}
    {

    }

    virtual ~SetQueueTask() override
    {
		queue = nullptr;
    }

    Queue *GetQueue() const
    {
        return queue;
    }

protected:
    Queue *queue;
};

class RecordingTask : public AsyncTask
{
public:
    template <class T>
    RecordingTask(T callback) :
	    AsyncTask{ AsyncTaskType::Recording }
    {
		callbackWarpper = [=](uint64_t sync, CommandBuffer *commandBuffer) -> void {
			callback(sync, commandBuffer);
		};
    }

    virtual ~RecordingTask() override
    {

    }

    void Recording(uint64_t sync, CommandBuffer *commandBuffer)
    {
		callbackWarpper(sync, commandBuffer);
    }

protected:
    std::function<void(uint64_t, CommandBuffer *)> callbackWarpper;
};

class QueueTask : public AsyncTask
{
public:
    template <class T>
	QueueTask(T callback) :
	    AsyncTask{AsyncTaskType::QueueOperation}
	{
		callbackWarpper = [=](Queue *queue) -> void {
			callback(queue);
		};
	}

    virtual ~QueueTask() override
    {

    }

	void QueueOperation(Queue *queue)
	{
		callbackWarpper(queue);
	}

protected:
	std::function<void(Queue *)> callbackWarpper;
};

class ExecutionCompletedTask : public AsyncTask
{
public:
    template <class T>
    ExecutionCompletedTask(T &&callback) :
        AsyncTask{ AsyncTaskType::ExecutionCompleted }
    {
        callbackWarpper = [=]() -> void {
            callback();
        };
    }

    virtual ~ExecutionCompletedTask() override
    {

    }

    void Invoke()
    {
        callbackWarpper();
    }

    void operator()()
    {
		callbackWarpper();
    }

protected:
	std::function<void()> callbackWarpper;
};

class AsyncComputeThread
{
public:
    AsyncComputeThread(Device *device);

    bool IsExecutionCompleted(uint64_t value);

    void WaitIdle();

    void Join();

    void SetDescription(const std::string &description);

public:
    template <class T, class ... Args>
	void Execute(Args &&...args)
    {
		URef<AsyncTask> task = new T{std::forward<Args>(args)...};
		tasks.enqueue(std::move(task));
		semaphore.signal();
    }

    void Execute(AsyncTaskType type)
    {
		Execute<AsyncTask>(type);
    }

protected:
    Thread thread;

    ThreadPool executionCompletedThread{1};

    moodycamel::details::Semaphore semaphore;

    ConcurrentQueue<URef<AsyncTask>> tasks;

    std::vector<std::pair<uint64_t, URef<AsyncTask>>> executionCompletedTasks;
};

}
