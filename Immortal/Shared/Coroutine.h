#pragma once

#include <coroutine>
#include <iostream>

namespace Immortal
{

struct Promise;

struct Coroutine : std::coroutine_handle<Promise>
{
	using promise_type = Promise;
};

struct Promise
{
	Coroutine get_return_object()
	{
		return { Coroutine::from_promise(*this) };
	}

	std::suspend_always initial_suspend() noexcept
	{
		return {};
	}

	std::suspend_always final_suspend() noexcept
	{
		return {};
	}

	void return_void()
	{

	}

	void unhandled_exception()
	{
	
	}
};

}
