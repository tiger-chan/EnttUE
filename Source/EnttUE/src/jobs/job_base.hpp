#pragma once

#include "CoreMinimal.h"
#include "entt/core/fwd.hpp"
#include "fwd.hpp"
#include "core/fwd.hpp"
#include "core/linked_list.hpp"

namespace tc
{
struct ENTTUE_API job_handle {
	entt::id_type id;
	job_base *handle;
};

class ENTTUE_API job_base {
	friend class job_processor;

    public:
	using job_task = executable_task<ecs_registry &>;
	class job_task_list : public linked_list<TSharedPtr<job_task>> {
	};
	using iterator = job_task_list::iterator;

	job_base();

	virtual ~job_base() = default;

	inline const job_handle &handle()
	{
		return job_;
	}

	TArray<entt::id_type> dependencies() const;

	virtual void create() = 0;

	virtual void schedule() = 0;

	void add_job_dependency(job_handle handle);

	inline tc::world &world()
	{
		return *world_;
	}

	iterator begin()
	{
		return tasks_.begin();
	}

	iterator end()
	{
		return tasks_.end();
	}

    protected:
	void add_task(TSharedPtr<job_task> task)
	{
		tasks_.emplace_back(std::move(task));
	}

    private:
	tc::world *world_;
	job_handle job_;
	TMap<entt::id_type, job_handle> dependencies_;
	job_task_list tasks_;
};
} // namespace tc
