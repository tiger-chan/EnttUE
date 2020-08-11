#include "ecs_world_actor.h"

AEcsWorldActor::AEcsWorldActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEcsWorldActor::BeginPlay()
{
	world_ = MakeShared<tc::world>(run_systems_in_parallel);

	register_systems(*world_);

	Super::BeginPlay();
}

void AEcsWorldActor::Tick(float delta_time)
{
	assert(world_.IsValid());

	world_->execute(delta_time);

	Super::Tick(delta_time);
}
