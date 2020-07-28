#pragma once

#include "CoreMinimal.h"
#include "task_scheduler.hpp"
#include "async_task_scheduler.hpp"

namespace tc
{
	template <typename... args_t>
	TUniquePtr<task_scheduler<args_t...>> make_scheduler(bool run_parallel = false)
	{
		if (FPlatformProcess::SupportsMultithreading() && run_parallel)
		{
			return MakeUnique<async_task_scheduler<args_t...>>();
		}

		return MakeUnique<task_scheduler<args_t...>>();
	}
} // namespace tc
