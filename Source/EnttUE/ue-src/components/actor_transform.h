#pragma once

#include "CoreMinimal.h"
#include "core/component_authoring.h"
#include "entt/entity/registry.hpp"
#include "actor_transform.generated.h"

USTRUCT(BlueprintType)
struct ENTTUE_API FActorTransform : public FTransform {
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct ENTTUE_API FActorTransformAuthoring : public FComponentAuthoring {
	GENERATED_BODY()

    public:
	void add_to_ecs(tc::ecs_registry &registry,
			tc::entity_handle handle) override
	{
		registry.emplace_or_replace<FActorTransform>(handle.id, value);
	}

	FActorTransform value;
};
