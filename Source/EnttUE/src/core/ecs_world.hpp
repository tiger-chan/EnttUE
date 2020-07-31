#pragma once

#include "CoreMinimal.h"
#include "ecs_registry.hpp"

namespace tc
{
class world {
    private:
	ecs_registry registry;
};

} // namespace tc
