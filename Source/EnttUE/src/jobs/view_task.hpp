#pragma once

#include "CoreMinimal.h"
#include "core/fwd.hpp"
#include "fwd.hpp"
#include "executable_task.hpp"
#include "job_requirements_builder.hpp"

namespace tc
{
template <typename... Args> struct view_task : public executable_task<ecs_registry &, Args...> {
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

namespace internal
{
template <typename...> struct entity_view_query_builder;

template <typename System, typename... Args>
struct entity_view_query_builder<System, entt::type_list<Args...>> {
	entity_view_query_builder(System *sys) : system{ sys }
	{
	}

	template <typename... Excluded> auto with_none() const noexcept
	{
		return entity_view_query_builder<System, entt::type_list<Args...>,
						 job_exclude_t<Excluded...>>{ system };
	};

	template <typename... Included> auto with_all() const noexcept
	{
		return entity_view_query_builder<System, entt::type_list<Args...>, job_exclude_t<>,
						 Included...>{ system };
	}

    private:
	System *system;
};

template <typename System, typename... Args, typename... Excluded, typename... Included>
struct entity_view_query_builder<System, entt::type_list<Args...>, job_exclude_t<Excluded...>,
				 Included...> {
	entity_view_query_builder(System *sys) : system{ sys }
	{
	}

	template <typename... ExExcluded> auto with_none() const noexcept
	{
		return entity_view_query_builder<System, entt::type_list<Args...>,
						 job_exclude_t<Excluded..., ExExcluded...>,
						 Included...>{ system };
	};

	template <typename... ExIncluded> auto with_all() const noexcept
	{
		return entity_view_query_builder<System, entt::type_list<Args...>,
						 job_exclude_t<Excluded...>, Included...,
						 ExIncluded...>{ system };
	}

	using requirement_t =
		view_job_requirements<System, view_task<Args...>, entt::type_list<Args...>,
				      entt::type_list<Included...>, entt::type_list<Excluded...>>;

	template <typename... Reads> requirement_t add_read()
	{
		return requirement_t{ system, make_task() }.add_read<Reads...>();
	}

	template <typename... Writes> requirement_t add_write()
	{
		return requirement_t{ system, make_task() }.add_write<Writes...>();
	}

    private:
	TSharedPtr<view_task<Args...>> make_task()
	{
		return MakeShared<view_task<Args...>>();
	}

	System *system;
};

}; // namespace internal

} // namespace tc
