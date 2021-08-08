#pragma once

#include <chrono>

namespace Immortal {

	class Timestep
	{
	public:
		Timestep(float time = 0.0f)
			: mTime(time)
		{

		}

		operator float() const { return mTime; }

		__forceinline float DeltaTime() const { return mTime; }
		__forceinline float Seconds() const { return mTime; }
		__forceinline float Milliseconds() const { return mTime * 1000.0f; }

	private:
		float mTime;
	};

	class Timer
	{
	public:
		using Seconds      = std::ratio<1>;
		using Milliseconds = std::ratio<1, 1000>;
		using Microseconds = std::ratio<1, 1000000>;
		using Nanoseconds  = std::ratio<1, 1000000000>;

		using Clock             = std::chrono::steady_clock;
		using DefaultResolution = Milliseconds;

	public:
		Timer() : mStartTime{ Clock::now() }, mPreviousTick{ mStartTime } { }
		
		virtual ~Timer() = default;

		void Start()
		{
			if (!mRunning)
			{
				mRunning = true;
				mStartTime = Clock::now();
			}
		}
		
		void Lap()
		{
			mLapping = true;
			mLapTime = Clock::now();
		}

		bool Running() const
		{
			return mRunning;
		}

		template <class T = DefaultResolution>
		double Stop()
		{
			if (!mRunning)
			{
				return 0;
			}

			mRunning = false;
			mLapping  = false;

			auto now = Clock::now();
			auto duration = std::chrono::duration<double, T>(now - mStartTime);
			mStartTime = now;
			mLapTime = now;

			return duration.count();
		}
		
		template <class T = DefaultResolution>
		double elapsed()
		{
			if (!mRunning)
			{
				return 0;
			}

			Clock::time_point start = mStartTime;

			if (mLapping)
			{
				start = mLapTime;
			}

			return std::chrono::duration<double, T>(Clock::now() - start).count();
		}

		template <class T = DefaultResolution>
		double tick()
		{
			auto now = Clock::now();
			auto duration = std::chrono::duration<double, T>(now - mPreviousTick);
			mPreviousTick = now;
			return duration();
		}

	private:
		bool mRunning{ false };
		bool mLapping{ false };

		Clock::time_point mStartTime;
		Clock::time_point mLapTime;
		Clock::time_point mPreviousTick;
	};

}