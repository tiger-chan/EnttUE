#include "ecs_world_actor.h"

void AEcsWorldActor::BeginPlay()
{
	ecs_world = MakeShared<tc::world>();

	Super::BeginPlay();
}
