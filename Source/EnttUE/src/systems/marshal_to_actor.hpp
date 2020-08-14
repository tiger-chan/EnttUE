#pragma once

#include "CoreMinimal.h"
#include "jobs/system_base.hpp"

namespace tc
{
class ENTTUE_API marshal_to_actor_system : public system_base<marshal_to_actor_system> {
	friend class system_base<marshal_to_actor_system>;

    private:
	void on_create();
	void on_schedule();

	static void position_packing(entt::observer& observer, tc::ecs_registry& reg);
	static void scale_packing(entt::observer& observer, tc::ecs_registry& reg);
	static void rotation_packing(entt::observer& observer, tc::ecs_registry& reg);
	static void marshal_to_actor(entt::observer& observer, tc::ecs_registry& reg);
};

} // namespace tc
