#include "convert_to_entity.h"
#include "EngineUtils.h"
#include "Engine.h"
#include "component_authoring.h"
#include "components/actor_reference.h"
#include "logging.hpp"

UConvertToEntityComponent::UConvertToEntityComponent()
{
	world_type = AEcsWorldActor::StaticClass();
}

void UConvertToEntityComponent::BeginPlay()
{
	handle = register_world();

	register_components();

	Super::BeginPlay();
}

tc::entity_handle UConvertToEntityComponent::register_world()
{
	UWorld *world = GEngine->GetWorldFromContextObject(GetOwner(), EGetWorldErrorMode::Assert);

	world_actor = nullptr;
	for (TActorIterator<AEcsWorldActor> iter{ world }; iter; ++iter) {
		if (!IsValid(*iter)) {
			continue;
		}

		world_actor = *iter;
		break;
	}

	if (world_actor == nullptr) {
		UE_LOG(LogEnttUE, Warning, TEXT("ECS World was not found spawning a new instance"));
		world_actor = world->SpawnActor<AEcsWorldActor>(world_type);
	}

	return tc::entity_handle{ world_actor->get_world().registry().create() };
}

void UConvertToEntityComponent::register_components()
{
	check(world_actor);
	auto &reg = world_actor->get_world().registry();

	reg.emplace_or_replace<FActorReference>(handle.id, GetOwner());

	auto iter = TFieldIterator<FStructProperty>(GetOwner()->GetClass());
	for (; iter; ++iter) {
		FStructProperty *prop = *iter;
		if (!prop->Struct->IsChildOf(FComponentAuthoring::StaticStruct())) {
			continue;
		}

		auto *comp = prop->ContainerPtrToValuePtr<FComponentAuthoring>(GetOwner());

		if (!comp) {
			continue;
		}

		comp->add_to_ecs(reg, handle);
	}
}
