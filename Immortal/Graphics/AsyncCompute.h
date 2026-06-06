#pragma once

#include "Shared/Async.h"
#include "Queue.h"
#include "LightGraphics.h"

#define IMMORTAL_ASYNC_COMPUTE_STACK_TRACE 0

#if IMMORTAL_ASYNC_COMPUTE_STACK_TRACE
#include <stacktrace>
#endif

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
	    type{type}
#if IMMORTAL_ASYNC_COMPUTE_STACK_TRACE
	    , stacktrace{std::stacktrace::current()}
#endif
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
#if IMMORTAL_ASYNC_COMPUTE_STACK_TRACE
	std::stacktrace stacktrace;
#endif
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
		callbackWarpper = [=](CommandBuffer *commandBuffer) -> void {
			callback(commandBuffer);
		};
    }

    virtual ~RecordingTask() override
    {

    }

    void Recording(CommandBuffer *commandBuffer)
    {
		callbackWarpper(commandBuffer);
    }

protected:
    std::function<void(CommandBuffer *)> callbackWarpper;
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
        callbackWarpper = [=]() mutable -> void {
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

class AsyncComputeThread : public IClass
{
public:
    AsyncComputeThread(Device *device);

    ~AsyncComputeThread();

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

    void Begin()
    {
		Execute(AsyncTaskType::BeginRecording);
    }

    void End()
    {
		Execute(AsyncTaskType::EndRecording);
    }

    void Submit()
    {
		Execute(AsyncTaskType::Submiting);
    }

protected:
    Thread thread;

    ThreadPool executionCompletedThread{1};

    moodycamel::details::Semaphore semaphore;

    ConcurrentQueue<URef<AsyncTask>> tasks;

    std::vector<std::pair<uint64_t, URef<AsyncTask>>> executionCompletedTasks;
};

class AsyncTaskLock
{
public:
	explicit AsyncTaskLock(AsyncComputeThread *thread) :
	    thread{ thread }
    {
		thread->Begin();
    }

    ~AsyncTaskLock()
    {
		thread->End();
		thread->Submit();
    }

protected:
	AsyncComputeThread *thread;
};

}
