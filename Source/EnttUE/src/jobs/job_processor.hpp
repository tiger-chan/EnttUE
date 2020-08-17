#pragma once

#include "CoreMinimal.h"
#include "core/fwd.hpp"
#include "fwd.hpp"
#include "task_data_access.hpp"
#include "job_base.hpp"
#include "directed_graph.hpp"
#include "logging.hpp"

namespace tc
{
class job_processor {
    public:
	using job_t = job_base;
	using job_ptr_t = TSharedPtr<job_t>;
	using job_task = job_base::job_task;

	job_processor() = default;
	virtual ~job_processor()
	{
		halt();
	}

	job_processor(const job_processor &) = delete;
	job_processor(job_processor &&) = delete;
	job_processor &operator=(const job_processor &) = delete;
	job_processor &operator=(job_processor &&) = delete;

	bool running() const
	{
		return is_running;
	}

	void execute(ecs_registry &reg)
	{
		if (is_running) {
			return;
		}
		is_running = true;
		setup_tasks();

		run_tasks(reg);
	}

	void halt()
	{
		if (!is_running) {
			return;
		}

		halt_intern();

		set_complete();
	}

	template <typename Job> void register_job(world *world)
	{
		static_assert(std::is_convertible_v<Job *, job_t *>,
			      "Can only register jobs inheriting from job_base");
		entt::id_type id = entt::type_info<Job>::id();
		if (jobs_.Contains(id)) {
			return;
		}

		job_ptr_t job = MakeShared<Job>();
		job->world_ = world;
		job->job_.id = id;
		checkf(!processing_registration.Contains(id),
		       TEXT("Circular Reference found while registering  (%d)"), id);

		processing_registration.Emplace(id);
		// not sure if this phase is needed yet...
		job->create();

		// Schedule the tasks that are going to take place in this job.
		job->schedule();

		jobs_.Emplace(id, std::move(job));

		processing_registration.Remove(id);

		is_dirty = true;
	}

	template <typename Job> Job *get_or_create_job(world *world)
	{
		register_job<Job>(world);
		entt::id_type id = entt::type_info<Job>::id();
		return static_cast<Job *>(jobs_[id].Get());
	}

    protected:
	virtual void run_tasks(ecs_registry &reg) = 0;

	virtual void halt_intern()
	{
	}

	inline void set_complete()
	{
		is_running = false;
	}

	void sort_graph()
	{
		system_graph_.sort(std::begin(current_processes), std::end(current_processes));
	}

	job_ptr_t &get_job(entt::id_type id)
	{
		return jobs_[id];
	}

	std::vector<sortable_graph::data_t> current_processes;

    private:
	void setup_tasks()
	{
		if (is_dirty) {
			/* sort */
			system_graph_ = sortable_graph(jobs_.Num());

			for (auto &job : jobs_) {
				auto &task = *job.Value;
				auto &handle = task.handle();
				system_graph_.add(handle.id, task.dependencies());
			}

			// UE_LOG(LogEnttUE, Warning, TEXT("Graph Adjacencies:\n\n%s"),
			//        *FString(system_graph_.to_string().c_str()));

			is_dirty = false;
		}

		current_processes = system_graph_.sorted_nodes();
	}

	bool is_dirty = false;
	bool is_running = false;
	TMap<entt::id_type, job_ptr_t> jobs_;
	sortable_graph system_graph_{};
	TSet<entt::id_type> processing_registration;
};

} // namespace tc
