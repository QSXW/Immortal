#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <thread>
#include <queue>
#include <future>
#include <functional>
#include <atomic>

#include "Async.h"
#include "DLLLoader.h"
#include "IObject.h"
#include "Log.h"
