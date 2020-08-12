#pragma once

#include "GameFramework/Actor.h"
#include "entt/entity/registry.hpp"
#include "core/component_authoring.h"
#include "marshal_to_actor.generated.h"

USTRUCT(BlueprintType)
struct ENTTUE_API FMarshalToActor {
	GENERATED_BODY()

    public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool world_space = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool sweep = false;
};

USTRUCT(BlueprintType)
struct ENTTUE_API FMarshalToActorAuthoring : public FComponentAuthoring {
	GENERATED_BODY()

    public:
	void add_to_ecs(tc::ecs_registry &registry, tc::entity_handle handle) override
	{
		registry.emplace_or_replace<FMarshalToActor>(handle.id, value);
	}

	FMarshalToActor value;
};