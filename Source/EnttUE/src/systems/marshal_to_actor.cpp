#include "marshal_to_actor.hpp"
#include "components/actor_reference.h"
#include "components/actor_transform.h"
#include "components/position.h"
#include "components/scale.h"
#include "components/rotation.h"
#include "components/marshal_to_actor.h"
#include "marshal_to_ecs.hpp"
#include "logging.hpp"

namespace tc
{
void marshal_to_actor_system::on_create()
{
	auto marshal_to_ecs = world().get_or_create_job<marshal_to_ecs_system>();
	add_job_dependency(marshal_to_ecs->handle());
}

void marshal_to_actor_system::on_schedule()
{
	entities()
		.updated<FPosition>()
		.where<FActorTransform, FActorReference>()
		.add_read<FPosition>()
		.add_write<FActorTransform>()
		.schedule(&marshal_to_actor_system::position_packing);

	entities()
		.updated<FScale>()
		.where<FActorTransform, FActorReference>()
		.add_read<FScale>()
		.add_write<FActorTransform>()
		.schedule(&marshal_to_actor_system::scale_packing);

	entities()
		.updated<FRotation>()
		.where<FActorTransform, FActorReference>()
		.add_read<FRotation>()
		.add_write<FActorTransform>()
		.schedule(&marshal_to_actor_system::rotation_packing);

	entities()
		.updated<FActorTransform>()
		.where<FMarshalToActor, FActorReference>()
		.add_read<FActorTransform, FMarshalToActor>()
		.add_write<FActorReference, FActorTransform>()
		.schedule(&marshal_to_actor_system::marshal_to_actor,
			  EAsyncExecution::TaskGraphMainThread);
}

DECLARE_CYCLE_STAT(TEXT("ECS: Location Packing"), STAT_PositionPacking, STATGROUP_ECS);
void marshal_to_actor_system::position_packing(entt::observer &observer, tc::ecs_registry &reg)
{
	SCOPE_CYCLE_COUNTER(STAT_PositionPacking);
	for (const auto &id : observer) {
		FPosition &value = reg.get<FPosition>(id);
		reg.patch<FActorTransform>(id, [&value](FActorTransform &val) {
			val.SetLocation(value);
		});

		auto &tmp = reg.get<FActorTransform>(id);
	}
	observer.clear();
	UE_LOG(LogEnttUE, VeryVerbose, TEXT("Ran: Marshal to Actor - Location packing"));
}

DECLARE_CYCLE_STAT(TEXT("ECS: Scale Packing"), STAT_ScalePacking, STATGROUP_ECS);
void marshal_to_actor_system::scale_packing(entt::observer &observer, tc::ecs_registry &reg)
{
	SCOPE_CYCLE_COUNTER(STAT_ScalePacking);
	for (const auto &id : observer) {
		FScale &value = reg.get<FScale>(id);
		reg.patch<FActorTransform>(id, [&value](FActorTransform &val) {
			val.SetScale3D(value);
		});
	}
	observer.clear();
	UE_LOG(LogEnttUE, VeryVerbose, TEXT("Ran: Marshal to Actor - Scale packing"));
}

DECLARE_CYCLE_STAT(TEXT("ECS: Rotation Packing"), STAT_RotionPacking, STATGROUP_ECS);
void marshal_to_actor_system::rotation_packing(entt::observer &observer, tc::ecs_registry &reg)
{
	SCOPE_CYCLE_COUNTER(STAT_RotionPacking);
	for (const auto &id : observer) {
		FRotation &value = reg.get<FRotation>(id);
		reg.patch<FActorTransform>(id, [&value](FActorTransform &val) {
			val.SetRotation(value);
		});
	}
	observer.clear();
	UE_LOG(LogEnttUE, VeryVerbose, TEXT("Ran: Marshal to Actor - Rotation packing"));
}

DECLARE_CYCLE_STAT(TEXT("ECS: Marshal Transform - Actor"), STAT_MarshalToActor, STATGROUP_ECS);
void marshal_to_actor_system::marshal_to_actor(entt::observer &observer, tc::ecs_registry &reg)
{
	SCOPE_CYCLE_COUNTER(STAT_MarshalToActor);
	for (const auto &id : observer) {
		FMarshalToActor &marshalling = reg.get<FMarshalToActor>(id);

		TWeakObjectPtr<AActor> &ref = reg.get<FActorReference>(id).actor;
		check(ref.IsValid());
		AActor *actor = ref.Get();

		FActorTransform &transform = reg.get<FActorTransform>(id);
		FHitResult results;

		TFunction<bool(const FTransform &, bool, FHitResult *)> func;

		if (marshalling.world_space) {
			func = [actor](const FTransform &t, bool s, FHitResult *r) -> bool {
				actor->SetActorTransform(t, s, r);
				return r->bBlockingHit;
			};
		} else {
			func = [actor](const FTransform &t, bool s, FHitResult *r) -> bool {
				actor->SetActorRelativeTransform(t, s, r);
				return r->bBlockingHit;
			};
		}

		if (func(transform, marshalling.sweep, &results) && marshalling.sweep) {
			transform.SetLocation(results.Location);
		}
	}
	observer.clear();
	UE_LOG(LogEnttUE, VeryVerbose, TEXT("Ran: Marshal to Actor"));
}
} // namespace tc
