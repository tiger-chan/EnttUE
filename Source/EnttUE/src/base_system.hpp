#pragma once

#include "CoreMinimal.h"
#include "scheduler/task_scheduler.hpp"

namespace tc
{
	template<typename... args_t>
	class base_system
	{
	public:
		virtual ~base_system() = default;

		virtual void schedule(task_scheduler<args_t...>& scheduler) = 0;

	private:
	};
	
} // namespace tc
