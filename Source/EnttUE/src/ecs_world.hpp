#pragma once

#include "CoreMinimal.h"
#include "entt/entt.hpp"

namespace tc
{
	using ecs_registry = entt::registry;

	class ecs_world
	{
	private:
		ecs_registry registry;
	};
	
} // namespace tc
