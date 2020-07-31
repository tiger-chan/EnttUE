#pragma once

#include "CoreMinimal.h"
#include "entt/entity/entity.hpp"

namespace tc
{
struct entity_handle {
	entt::entity id = entt::null;
};

} // namespace tc
