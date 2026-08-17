#pragma once

#include "Shared/Async.h"
#include "Queue.h"
#include "LightGraphics.h"

#include <functional>
#include <mutex>

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

    // Opens and submits a standalone recording batch, then invokes callback on
    // the serial completion thread. Never call this inside an existing batch.
    void SubmitStandaloneCompletion(std::function<void()> callback);

    void Join();

    void SetDescription(const std::string &description);

public:
    template <class T, class ... Args>
	void Execute(Args &&...args)
    {
		std::lock_guard<std::recursive_mutex> lock{ enqueueMutex };
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
    friend class AsyncRecordingScope;

    Thread thread;

    ThreadPool executionCompletedThread{1};

    moodycamel::details::Semaphore semaphore;

    ConcurrentQueue<URef<AsyncTask>> tasks;

    std::vector<std::pair<uint64_t, URef<AsyncTask>>> executionCompletedTasks;

    std::recursive_mutex enqueueMutex;
};

// Owns one standalone Begin/End/Submit recording batch. Application UI frames
// already keep Graphics' async thread inside a recording batch, so UI update
// code must enqueue directly instead of creating this scope.
class AsyncRecordingScope
{
public:
	explicit AsyncRecordingScope(AsyncComputeThread *thread) :
	    thread{ thread },
	    enqueueGuard{ thread->enqueueMutex }
    {
		thread->Begin();
    }

    ~AsyncRecordingScope()
    {
		thread->End();
		thread->Submit();
    }

protected:
	AsyncComputeThread *thread;

	std::unique_lock<std::recursive_mutex> enqueueGuard;
};

}
