#include "convert_to_ecs.h"
#include "EngineUtils.h"
#include "Engine.h"
#include "component_authoring.h"
#include "components/actor_reference.h"

UConvertToEcsComponent::UConvertToEcsComponent()
{
	world_type = AEcsWorldActor::StaticClass();
}

void UConvertToEcsComponent::BeginPlay()
{
	handle = register_world();

	register_components();

	Super::BeginPlay();
}

tc::entity_handle UConvertToEcsComponent::register_world()
{
	UWorld *world = GEngine->GetWorldFromContextObject(
		GetOwner(), EGetWorldErrorMode::Assert);

	world_actor = nullptr;
	for (TActorIterator<AEcsWorldActor> iter{ world }; iter; ++iter) {
		if (!IsValid(*iter)) {
			continue;
		}

		world_actor = *iter;
		break;
	}

	if (world_actor == nullptr) {
		world_actor = world->SpawnActor<AEcsWorldActor>(world_type);
	}

	return tc::entity_handle{
		world_actor->get_world().get_registry().create()
	};
}

void UConvertToEcsComponent::register_components()
{
	check(world_actor);
	auto &reg = world_actor->get_world().get_registry();

	reg.emplace_or_replace<FActorReference>(handle.id, GetOwner());

	auto iter = TFieldIterator<FStructProperty>(GetOwner()->GetClass());
	for (; iter; ++iter) {
		FStructProperty *prop = *iter;
		if (prop->Struct !=
		    TBaseStructure<FComponentAuthoring>::Get()) {
			continue;
		}

		auto *comp = prop->ContainerPtrToValuePtr<FComponentAuthoring>(
			GetOwner());

		if (!comp) {
			continue;
		}

		comp->add_to_ecs(reg, handle);
	}
}
