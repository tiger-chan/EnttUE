#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "core/ecs_world.hpp"
#include "ecs_world_actor.generated.h"

UCLASS()
class ENTTUE_API AEcsWorldActor : public AActor {
	GENERATED_BODY()
    public:
	tc::world &get_world()
	{
		check(ecs_world.IsValid()) return *ecs_world;
	}
	
	const tc::world &get_world() const
	{
		check(ecs_world.IsValid()) return *ecs_world;
	}

    protected:
	void BeginPlay() override;
	virtual void register_systems(tc::world &world) { };

    private:
	TSharedPtr<tc::world> ecs_world;
};
