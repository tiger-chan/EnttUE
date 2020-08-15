#pragma once

#include "CoreMinimal.h"
#include "core/fwd.hpp"
#include "fwd.hpp"
#include "task.hpp"
#include "job_requirements_builder.hpp"

namespace tc
{
template <typename... Args> struct view_task : public task<ecs_registry &, Args...> {
	using work_t = TFunction<void(ecs_registry &, Args &&...)>;

	void run(ecs_registry &reg, Args &&... args)
	{
		work(reg, std::forward<Args>(args)...);
	}

	work_t work;
};

template <typename System, typename Type, typename...> struct view_job_requirements;

template <typename System, typename Type, typename... Args, typename... Included,
	  typename... Excluded>
struct view_job_requirements<System, Type, entt::type_list<Args...>, entt::type_list<Included...>,
			     entt::type_list<Excluded...>>
	: public job_requirements_builder<
		  view_job_requirements<System, Type, entt::type_list<Args...>,
					entt::type_list<Included...>, entt::type_list<Excluded...>>,
		  System, Type, Args...> {
	using Super = job_requirements_builder<
		view_job_requirements<System, Type, entt::type_list<Args...>,
				      entt::type_list<Included...>, entt::type_list<Excluded...>>,
		System, Type, Args...>;
	using Super::Super;
	friend struct Super;

    private:
	template <typename Func> void set_work(Type &task, Func func)
	{
		task.work = [func](ecs_registry &reg, Args &&... args) {
			func(reg.view<Included...>(entt::exclude<Excluded...>), reg,
			     std::forward<Args>(args)...);
		};
	}
};

} // namespace tc
