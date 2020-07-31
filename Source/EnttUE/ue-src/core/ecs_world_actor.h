#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "core/ecs_world.hpp"
#include "ecs_world_actor.generated.h"

UCLASS(ABSTRACT)
class ENTTUE_API AEcsWorldActor : public AActor {
	GENERATED_BODY()
    public:
    protected:
	void BeginPlay() override;
	virtual void register_systems(tc::world &world)
		PURE_VIRTUAL(AEcsWorldActor::register_system, );

    private:
	TSharedPtr<tc::world> ecs_world;
};
