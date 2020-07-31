#pragma once

#include <atomic>
#include "CoreMinimal.h"
#include "Async/Async.h"
#include "task_scheduler.hpp"

namespace tc
{
template <typename... task_args_t>
class async_task_scheduler : public task_scheduler<task_args_t...> {
	using graph_t = task_graph<task_args_t...>;
	using task_t = task<task_args_t...>;

	struct async_task_handle {
		task_handle *handle;
		TFuture<void> future;
	};

    protected:
	void execute_intern(task_args_t &&... args) override
	{
		tasks_stage.store(0);

		if (ordered_tasks.Num() == 0) {
			// We have completed all tasks.
			set_complete();
			return;
		}

		auto &groups = ordered_tasks[tasks_stage.load()];

		current_tasks.Empty(groups.Num());
		for (int32_t i = 0; i < groups.Num(); ++i) {
			task_handle &handle = groups[i];
			current_tasks.Emplace(async_task_handle{
				&handle,
				Async(EAsyncExecution::TaskGraph,
				      run_task(handle.task,
					       std::forward<task_args_t &&>(
						       args)...),
				      complete_task(std::forward<task_args_t &&>(
					      args)...)) });
		}
	}

	auto run_task(TSharedPtr<task_t> task, task_args_t &&... args)
	{
		return [task, passed_args = std::forward_as_tuple(
				      args...)]() mutable {
			return std::apply(
				[task](auto &&... args) { (*task)(args...); },
				std::move(passed_args));
		};
	}

	TFunction<void()> complete_task(task_args_t &&... args)
	{
		return [this, passed_args = std::forward_as_tuple(
				      args...)]() mutable {
			std::apply(
				[this](auto &&... args) {
					for (auto &task : current_tasks) {
						if (!task.future.IsReady()) {
							return;
						}
					}

					++tasks_stage;
					int32_t stage = tasks_stage.load();
					if (ordered_tasks.Num() == stage) {
						// We have completed all tasks.
						set_complete();
						return;
					}

					auto &groups = ordered_tasks[stage];
					current_tasks.Empty(groups.Num());
					for (int32_t i = 0; i < groups.Num();
					     ++i) {
						task_handle &handle = groups[i];
						current_tasks.Emplace(async_task_handle{
							&handle,
							Async(EAsyncExecution::
								      TaskGraph,
							      run_task(
								      handle.task,
								      std::forward<
									      task_args_t
										      &&>(
									      args)...),
							      complete_task(std::forward<
									    task_args_t
										    &&>(
								      args)...)) });
					}
				},
				std::move(passed_args));
		};
	}

    private:
	std::atomic<int32_t> tasks_stage = 0;
	TArray<async_task_handle> current_tasks;
};
} // namespace tc
