#pragma once

#include "CoreMinimal.h"
#include "core/ecs_world.hpp"
#include "job_base.hpp"
#include "entity_query_builder.hpp"
#include "executable_task.hpp"
#include "core/linked_list.hpp"

namespace tc
{
template <typename System> class system_base : public job_base {
	template <typename Requirements, typename System, typename Type, typename... Args>
	friend struct job_requirements_builder;

    public:
	virtual ~system_base() = default;

	void create() final
	{
		static_cast<System *>(this)->on_create();
	}

	void schedule() final
	{
		static_cast<System *>(this)->on_schedule();
	}

    protected:
	auto entities()
	{
		return entity_query_builder<System, entt::type_list<>>(
			static_cast<System *>(this), &world().registry());
	}
};
} // namespace tc
