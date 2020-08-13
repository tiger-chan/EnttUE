#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "core/ecs_world.hpp"
#include "logging.hpp"
#include "ecs_world_actor.generated.h"

UCLASS()
class ENTTUE_API AEcsWorldActor : public AActor {
	GENERATED_BODY()
    public:
	AEcsWorldActor();

	tc::world &get_world()
	{
		check(world_.IsValid()) return *world_;
	}

	const tc::world &get_world() const
	{
		check(world_.IsValid()) return *world_;
	}

    protected:
	void BeginPlay() override;
	void Tick(float delta_time) override;
	virtual void register_systems(tc::world &world){
		UE_LOG(LogEnttUE, Warning, TEXT("%s must be overridden"), TEXT(__FUNCTION__));
	};

	bool run_systems_in_parallel = false;

    private:
	TSharedPtr<tc::world> world_;
};
