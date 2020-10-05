#pragma once

#include <algorithm>
#include <vector>
#include <atomic>
#include "CoreMinimal.h"
#include "fwd.hpp"
#include "job_processor.hpp"
#include "executable_task.hpp"
#include "Async/Async.h"
#include "HAL/CriticalSection.h"

namespace tc
{
class async_job_processor : public job_processor {
    public:
	using job_processor::job_processor;
	using job_task_t = job_processor::job_task;

	struct running_task_t {
		struct thread_handle {
			job_base::iterator task;
			TFuture<void> future;
		};
		entt::id_type job_id;
		std::vector<thread_handle> handles;
		job_base::iterator post_complete;
	};

	struct pending_task_t {
		job_base::iterator task;
		entt::id_type job_id;
	};

    protected:
	void run_tasks(ecs_registry &reg) final
	{
		if (current_processes.empty()) {
			set_complete();
			return;
		}
		FScopeLock lock(&mutex_);
		start_current_tasks(reg);

		if (running_tasks.empty()) {
			current_processes.clear();
			pending_tasks.clear();
			set_complete();
		}
	}

	void halt_intern() final
	{
		is_cancelled.store(true);

		for (auto &task : running_tasks) {
			for (auto &handle : task.handles) {
				handle.future.Wait();
			}
		}

		current_processes.clear();
		pending_tasks.clear();

		is_cancelled.store(false);
	}

	TFuture<void> spawn_task(job_base::iterator &iter, ecs_registry &reg, entt::id_type id)
	{
		return Async((*iter)->execution_method, run_task(*iter, reg),
			     complete_task(reg, iter, id));
	}

	TFunction<void()> run_task(TSharedPtr<job_task_t> &task, ecs_registry &reg)
	{
		return [task, fwd = std::forward_as_tuple(reg)]() mutable {
			return std::apply([task](auto &&... args) { task->run(args...); },
					  std::move(fwd));
		};
	}

	TFunction<void()> complete_task(ecs_registry &reg, job_base::iterator &iter,
					entt::id_type id)
	{
		return [this, iter, id, fwd = std::forward_as_tuple(reg)]() mutable {
			std::apply(
				[this, &iter, id](auto &&... args) {
					using namespace std;
					// Test if all other tasks grouped to this on are complete
					// this is to prevent starting another task in the
					// job before all that should complete are done.

					// Test to see if there are any more tasks to complete
					// If there aren't mark run complete.

					// Move to the next sub task in the current
					// job or change to the next viable job.
					FScopeLock lock(&mutex_);

					auto predicate = [id](const auto &t) {
						return t.job_id == id;
					};

					auto task = find_if(begin(running_tasks),
							    end(running_tasks), predicate);

					complete_current_subtask(task, iter, id);

					if (!task->handles.empty()) {
						return;
					}

					if (is_cancelled.load()) {
						return;
					}

					auto &job = *get_job(id);
					if (task->post_complete == end(job)) {
						for (auto &process : current_processes) {
							process.dependencies.erase(task->job_id);
						}
					} else {
						pending_tasks.emplace_back(
							pending_task_t{ task->post_complete, id });
					}

					iter_swap(task,
						  begin(running_tasks) + running_tasks.size() - 1);
					running_tasks.pop_back();

					// find if there are any pending processes that are ready to be processed.
					start_pending_tasks(args...);

					sort_graph();

					start_current_tasks(args...);

					if (running_tasks.empty()) {
						pending_tasks.clear();
						set_complete();
						return;
					}
				},
				std::move(fwd));
		};
	}

	FCriticalSection mutex_;
	std::vector<running_task_t> running_tasks;
	std::vector<pending_task_t> pending_tasks;

    private:
	running_task_t run_task_helper(job_base::iterator task, job_base::iterator end,
				       entt::id_type id)
	{
		running_task_t handle{ id };
		while (task != end) {
			bool can_run = can_run_task(task);

			if (!can_run) {
				break;
			}

			for (auto &t : handle.handles) {
				if (!(*task)->can_run((*t.task)->data)) {
					can_run = false;
					break;
				}
			}

			if (!can_run) {
				break;
			}

			handle.handles.emplace_back(running_task_t::thread_handle{ task });

			if (++task == end) {
				break;
			}

			if (!(*task)->can_parallelize) {
				break;
			}
		}
		handle.post_complete = task;

		return handle;
	}

	bool can_run_task(job_base::iterator &task) const
	{
		for (auto &running_task : running_tasks) {
			for (auto &t : running_task.handles) {
				if (!(*task)->can_run((*t.task)->data)) {
					return false;
				}
			}
		}

		return true;
	}

	void complete_current_subtask(std::vector<running_task_t>::iterator task,
				      job_base::iterator &iter, entt::id_type id)
	{
		using namespace std;

		if (task == end(running_tasks)) {
			return;
		}

		auto handle_predicate = [&iter](const auto &h) { return h.task == iter; };

		task->handles.erase(remove_if(begin(task->handles), end(task->handles),
					      handle_predicate),
				    end(task->handles));
	}

	template <typename... Args> void start_pending_tasks(Args &&... args)
	{
		for (int32_t i = pending_tasks.size() - 1; 0 <= i; --i) {
			auto &pending = pending_tasks[i];
			auto t = pending.task;

			auto &pending_job = *get_job(pending.job_id);
			auto pending_task =
				run_task_helper(t, std::end(pending_job), pending.job_id);
			if (pending_task.handles.empty()) {
				continue;
			}

			auto &h = running_tasks.emplace_back(std::move(pending_task));
			for (auto &handle : h.handles) {
				handle.future = spawn_task(handle.task, args..., h.job_id);
			}

			std::iter_swap(std::begin(pending_tasks) + i,
				       std::begin(pending_tasks) + pending_tasks.size() - 1);
			pending_tasks.pop_back();
		}
	}

	template <typename... Args> void start_current_tasks(Args &&... args)
	{
		using namespace std;
		while (!current_processes.empty()) {
			auto &process = current_processes.back();
			if (!process.dependencies.empty()) {
				break;
			}

			auto &new_job = *get_job(process.id);
			auto new_task = run_task_helper(begin(new_job), end(new_job), process.id);

			current_processes.pop_back();
			if (new_task.handles.empty()) {
				pending_tasks.emplace_back(
					pending_task_t{ begin(new_job), process.id });

				continue;
			}

			auto &new_running_task = running_tasks.emplace_back(std::move(new_task));
			for (auto &h : new_running_task.handles) {
				h.future = spawn_task(h.task, args..., new_running_task.job_id);
			}
		}
	}

	std::atomic<bool> is_cancelled = false;
};
} // namespace tc
