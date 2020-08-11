#pragma once

#include "GameFramework/Actor.h"
#include "entt/entity/registry.hpp"
#include "core/component_authoring.h"
#include "marshal_to_ecs.generated.h"

USTRUCT(BlueprintType)
struct ENTTUE_API FMarshalToEcs {
	GENERATED_BODY()

    public:
	bool world_space = true;
};

USTRUCT(BlueprintType)
struct ENTTUE_API FMarshalToEcsAuthoring : public FComponentAuthoring {
	GENERATED_BODY()

    public:
	void add_to_ecs(tc::ecs_registry &registry, tc::entity_handle handle) override
	{
		registry.emplace_or_replace<FMarshalToEcs>(handle.id, value);
	}

	FMarshalToEcs value;
};