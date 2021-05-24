#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>

#include "Immortal/Core/Log.h"

namespace Immortal {

	namespace Profiler
	{
		using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

		struct Result
		{
			std::string Name;
			FloatingPointMicroseconds Start;
			std::chrono::microseconds ElapsedTime;
			std::thread::id ThreadID;
		};

		struct Session
		{
			std::string Name;
		};

		class Profiler
		{
		private:
			Profiler(const Profiler &) = delete;
			Profiler(Profiler &&) = delete;

		public:
			void BeginSession(const std::string &name, const std::string &filepath = "profiler.json")
			{
				/*The class lock_guard is a mutex wrapper that provides a convenient RAII-style mechanism for owning a mutex for the duration of a scoped block.
				When a lock_guard object is created, it attempts to take ownership of the mutex it is given. When control leaves the scope in which the lock_guard
				object was created, the lock_guard is destructed and the mutex is released. The lock_guard class is non-copyable.*/
				std::lock_guard lock(mMutex);

				if (mCurrentSession)
				{
					if (Log::GetCoreLogger())
					{
						IM_CORE_ERROR("Profiler::BeginSession('{0}') when session '{1}' alread open.", name, mCurrentSession->Name);
					}
					InternalEndSession();
				}
				
				mOutputStream.open(filepath);
				if (mOutputStream.is_open())
				{
					mCurrentSession = new Session({ name });
					WriteHead();
				}
				else
				{
					if (Log::GetCoreLogger())
					{
						IM_CORE_ERROR("Profiler could not open the results file: '{0}'.", filepath);
					}
				}
			}

			void EndSession()
			{
				std::lock_guard lock(mMutex);
				InternalEndSession();
			}

			void WriteProfile(const Result &result) noexcept
			{
				std::lock_guard lock(mMutex);
				std::stringstream json;

				json << std::setprecision(3) << std::fixed;
				json << ",{";
				json << "\"cat\":\"function\",";
				json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
				json << "\"name\":\"" << result.Name << "\",";
				json << "\"ph\":\"X\",";
				json << "\"pid\":0,";
				json << "\"tid\":" << result.ThreadID << ",";
				json << "\"ts\":" << result.Start.count();
				json << "}";

				if (mCurrentSession)
				{
					mOutputStream << json.str();
					mOutputStream.flush();
				}
			}

			static Profiler &Get()
			{
				static Profiler instance;
				return instance;
			}

		private:
			Profiler()
				: mCurrentSession(nullptr)
			{

			}

			~Profiler()
			{
				EndSession();
			}

			void WriteHead()
			{
				mOutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
				mOutputStream.flush();
			}

			void WriteFooter()
			{
				mOutputStream << "]}";
				mOutputStream.flush();
			}

			void InternalEndSession()
			{
				if (mCurrentSession)
				{
					WriteFooter();
					mOutputStream.close();
					delete mCurrentSession;
					mCurrentSession = nullptr;
				}
			}

		private:
			std::mutex mMutex;
			Session *mCurrentSession;
			std::ofstream mOutputStream;
		};

		class Timer
		{
		public:
			Timer(const char *name)
				: mName(name), mStopped(false)
			{
				mStartTimepoint = std::chrono::steady_clock::now();
			}

			~Timer()
			{
				if (!mStopped)
				{
					Stop();
				}
			}

			void Stop()
			{
				auto endTimepoint = std::chrono::steady_clock::now();
				auto highResStart = FloatingPointMicroseconds{ mStartTimepoint.time_since_epoch() };
				auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(mStartTimepoint).time_since_epoch();
				Profiler::Get().WriteProfile({ mName, highResStart, elapsedTime, std::this_thread::get_id() });

				mStopped = true;
			}

		private:
			const char *mName;
			std::chrono::time_point<std::chrono::steady_clock> mStartTimepoint;
			bool mStopped;
		};
	
		namespace Utils {

			template <size_t N>
			struct ChangeResult
			{
				char Data[N];
			};

			template <size_t N, size_t K>
			constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
			{
				ChangeResult<N> result = {};

				size_t srcIndex = 0;
				size_t dstIndex = 0;
				while (srcIndex < N)
				{
					size_t matchIndex = 0;
					while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
						matchIndex++;
					if (matchIndex == K - 1)
						srcIndex += matchIndex;
					result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
					srcIndex++;
				}
				return result;
			}
		}

	}
}

#define IM_PROFILE 1
#if IM_PROFILE
// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define IM_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define IM_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define IM_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define IM_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define IM_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define IM_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define IM_FUNC_SIG __func__
#else
#define IM_FUNC_SIG "IM_FUNC_SIG unknown!"
#endif

#define IM_PROFILE_BEGIN_SESSION(name, filepath) ::Immortal::Profiler::Profiler::Get().BeginSession(name, filepath)
#define IM_PROFILE_END_SESSION() ::Immortal::Profiler::Profiler::Get().EndSession()
#define IM_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Immortal::Profiler::Utils::CleanupOutputString(name, "__cdecl ");\
											   ::Immortal::Profiler::Timer timer##line(fixedName##line.Data)
#define IM_PROFILE_SCOPE_LINE(name, line) IM_PROFILE_SCOPE_LINE2(name, line)
#define IM_PROFILE_SCOPE(name) IM_PROFILE_SCOPE_LINE(name, __LINE__)
#define IM_PROFILE_FUNCTION() IM_PROFILE_SCOPE(IM_FUNC_SIG)
#else
#define IM_PROFILE_BEGIN_SESSION(name, filepath)
#define IM_PROFILE_END_SESSION()
#define IM_PROFILE_SCOPE(name)
#define IM_PROFILE_FUNCTION()
#endif