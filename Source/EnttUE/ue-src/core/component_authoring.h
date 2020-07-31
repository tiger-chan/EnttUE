#pragma once

#include "CoreMinimal.h"
#include "core/entity_handle.hpp"
#include "core/ecs_registry.hpp"
#include "component_authoring.generated.h"

USTRUCT()
struct ENTTUE_API FComponentAuthoring
{
	GENERATED_BODY()
	
	public:
	virtual void add_to_ecs(tc::ecs_registry& registry, tc::entity_handle handle) {};
};
