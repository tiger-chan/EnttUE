#pragma once

#include "CoreMinimal.h"
#include "fwd.hpp"
#include "job_processor.hpp"
#include "executable_task.hpp"

namespace tc
{
class sync_job_processor : public job_processor {
    public:
	using job_processor::job_processor;

    protected:
	void run_tasks(ecs_registry &reg) final
	{
		while (current_processes.size() > 0) {
			auto &job_details = current_processes.back();
			auto job = get_job(job_details.id);
			current_processes.pop_back();
			for (auto task = job->begin(); task != job->end(); ++task) {
				TSharedPtr<job_processor::job_task> i = (*task);
				i->run(reg);
			}
		}

		set_complete();
	}

	void halt_intern() final
	{
	}
};
} // namespace tc
