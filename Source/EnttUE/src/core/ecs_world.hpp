#pragma once

#include "CoreMinimal.h"
#include "ecs_registry.hpp"

namespace tc
{
class world {
    public:
	ecs_registry &get_registry()
	{
		return registry;
	}
	
	const ecs_registry &get_registry() const
	{
		return registry;
	}

    private:
	ecs_registry registry;
};

} // namespace tc
