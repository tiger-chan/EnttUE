#pragma once

#include "CoreMinimal.h"
#include "Async/Async.h"
#include "task_data_access.hpp"

namespace tc
{
template <typename... Args> struct executable_task {
	virtual ~executable_task() = default;
	virtual void run(Args &&...)
	{
	}

	bool can_run(const task_data_access &other) const
	{
		const auto &o_reads = other.get_reads();
		const auto &o_writes = other.get_writes();
		for (auto write : data.get_writes()) {
			if (o_reads.Contains(write) || o_writes.Contains(write)) {
				return false;
			}
		}

		for (auto read : data.get_reads()) {
			if (o_writes.Contains(read)) {
				return false;
			}
		}

		return true;
	}

	bool can_parallelize = false;
	EAsyncExecution execution_method = EAsyncExecution::TaskGraph;
	task_data_access data;
};
} // namespace tc