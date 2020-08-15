#pragma once

#include "CoreMinimal.h"
#include "core/fwd.hpp"
#include "fwd.hpp"
#include "entt/entity/observer.hpp"
#include "task.hpp"
#include "job_requirements_builder.hpp"

namespace tc
{
template <typename Type, entt::id_type Id> struct observer_wraper {
	template <typename Collector>
	observer_wraper(ecs_registry &reg, Collector collector) : observer{ reg, collector }
	{
	}
	entt::observer observer;
};

template <typename... Args> struct reactive_task : public task<ecs_registry &, Args...> {
	using work_t = TFunction<void(entt::observer &, ecs_registry &, Args &&...)>;
	using observer_request_t = TFunction<void(ecs_registry &, entt::observer *&)>;

	void run(ecs_registry &reg, Args &&... args)
	{
		entt::observer *observer = nullptr;
		observer_retrieval(reg, observer);
		work(*observer, reg, std::forward<Args>(args)...);
	}

	work_t work;
	observer_request_t observer_retrieval;
};

template <typename System, typename Type, typename... Args>
struct observer_job_requirements
	: public job_requirements_builder<observer_job_requirements<System, Type, Args...>, System, Type,
				  Args...> {
	using Super = job_requirements_builder<observer_job_requirements<System, Type, Args...>, System,
				       Type, Args...>;
	using Super::Super;
	friend struct Super;

    private:
	template <typename Func> void set_work(Type &task, Func func)
	{
		task.work = [func](entt::observer &o, ecs_registry &reg, Args &&... args) {
			func(o, reg, std::forward<Args>(args)...);
		};
	}
};

} // namespace tc