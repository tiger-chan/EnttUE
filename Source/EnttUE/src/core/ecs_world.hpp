#pragma once

#include "CoreMinimal.h"
#include "fwd.hpp"
#include "entt/entity/registry.hpp"
#include "jobs/sync_job_processor.hpp"
#include "jobs/async_job_processor.hpp"
#include "tick_context.hpp"

namespace tc
{
class world {
    public:
	world(bool run_parallel = false) : processor_{ tc::make_processor(run_parallel) }
	{
	}

	void execute(float delta_time)
	{
		if (!processor_->running()) {
			auto &tick = registry_.set<tc::tick_context>();
			tick.delta_time = delta_time;
			processor_->execute(registry_);
		}
	}

	ecs_registry &registry()
	{
		return registry_;
	}

	const ecs_registry &registry() const
	{
		return registry_;
	}

	template <typename Job> void register_job()
	{
		processor_->register_job<Job>(this);
	}

	template <typename Job> Job *get_or_create_job()
	{
		return processor_->get_or_create_job<Job>(this);
	}

    private:
	ecs_registry registry_;

	TUniquePtr<job_processor> processor_;
};

} // namespace tc
