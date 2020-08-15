#pragma once

#include "CoreMinimal.h"
#include "core/fwd.hpp"
#include "fwd.hpp"
#include "reactive_task.hpp"
#include "view_task.hpp"

namespace tc
{
template <typename System, typename... Args>
struct entity_query_builder<System, entt::type_list<Args...>> {
	entity_query_builder(System *sys, ecs_registry *registry)
		: system{ sys }, registry_{ registry }
	{
	}

	template <typename... Excluded> auto with_none() const noexcept
	{
		return internal::entity_view_query_builder<System, entt::type_list<Args...>,
					      internal::job_exclude_t<Excluded...>>{ system };
	};

	template <typename... Included> auto with_all() const noexcept
	{
		return internal::entity_view_query_builder<System, entt::type_list<Args...>, internal::job_exclude_t<>,
					      Included...>{ system };
	}

	template <typename Type> auto updated() const noexcept
	{
		return (internal::entity_reactive_query_builder<System, entt::type_list<Args...>>{ system,
										     registry_ })
			.updated<Type>();
	}

	template <typename... Included, typename... Excluded>
	auto entered_group(internal::job_exclude_t<Excluded...> = {}) const noexcept
	{
		return (internal::entity_reactive_query_builder<System, entt::type_list<Args...>>{ system,
										     registry_ })
			.entered_group<Included...>(internal::job_exclude_t<Excluded...>{});
	}

    private:
	System *system;
	ecs_registry *registry_;
};
} // namespace tc
