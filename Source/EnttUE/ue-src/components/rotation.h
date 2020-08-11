#pragma once

#include "CoreMinimal.h"
#include "entt/entity/registry.hpp"
#include "core/component_authoring.h"
#include "rotation.generated.h"

USTRUCT(BlueprintType)
struct ENTTUE_API FRotation : public FQuat {
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct ENTTUE_API FRotationAuthoring : public FComponentAuthoring {
	GENERATED_BODY()

    public:
	void add_to_ecs(tc::ecs_registry &registry,
			tc::entity_handle handle) override
	{
		registry.emplace_or_replace<FRotation>(handle.id, value);
	}

	FRotation value;
};
