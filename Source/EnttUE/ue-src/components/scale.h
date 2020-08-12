#pragma once

#include "CoreMinimal.h"
#include "entt/entity/registry.hpp"
#include "core/component_authoring.h"
#include "scale.generated.h"

USTRUCT(BlueprintType)
struct ENTTUE_API FScale : public FVector {
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct ENTTUE_API FScaleAuthoring : public FComponentAuthoring {
	GENERATED_BODY()

    public:
	void add_to_ecs(tc::ecs_registry &registry,
			tc::entity_handle handle) override
	{
		registry.emplace_or_replace<FScale>(handle.id, value);
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FScale value;
};
