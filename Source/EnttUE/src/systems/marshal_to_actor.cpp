#include "marshal_to_actor.hpp"
#include "components/actor_reference.h"
#include "components/actor_transform.h"
#include "components/position.h"
#include "components/scale.h"
#include "components/rotation.h"
#include "components/marshal_to_actor.h"
#include "marshal_to_ecs.hpp"

namespace tc
{
void marshal_to_actor_system::on_create()
{
	auto marshal_to_ecs = world().get_or_create_job<marshal_to_ecs_system>();
	marshal_to_ecs->add_job_dependency(handle());
}

void marshal_to_actor_system::on_schedule()
{
	entities()
		.updated<FPosition>()
		.where<FActorTransform, FActorReference>()
		.add_read<FPosition>()
		.add_write<FActorTransform>()
		.schedule([](auto &observer, ecs_registry &reg) {
			for (const auto &id : observer) {
				FPosition &value = reg.get<FPosition>(id);
				reg.patch<FActorTransform>(id, [&value](FActorTransform &val) {
					val.SetLocation(value);
				});
			}
			observer.clear();
			UE_LOG(LogEnttUE, Log, TEXT("Ran: Marshal to Actor - Location packing"));
		});

	entities()
		.updated<FScale>()
		.where<FActorTransform, FActorReference>()
		.add_read<FScale>()
		.add_write<FActorTransform>()
		.schedule([](auto &observer, ecs_registry &reg) {
			for (const auto &id : observer) {
				FScale &value = reg.get<FScale>(id);
				reg.patch<FActorTransform>(id, [&value](FActorTransform &val) {
					val.SetScale3D(value);
				});
			}
			observer.clear();
			UE_LOG(LogEnttUE, Log, TEXT("Ran: Marshal to Actor - Scale packing"));
		});

	entities()
		.updated<FRotation>()
		.where<FActorTransform, FActorReference>()
		.add_read<FRotation>()
		.add_write<FActorTransform>()
		.schedule([](auto &observer, ecs_registry &reg) {
			for (const auto &id : observer) {
				FRotation &value = reg.get<FRotation>(id);
				reg.patch<FActorTransform>(id, [&value](FActorTransform &val) {
					val.SetRotation(value);
				});
			}
			observer.clear();
			UE_LOG(LogEnttUE, Log, TEXT("Ran: Marshal to Actor - Rotation packing"));
		});

	entities()
		.updated<FActorTransform>()
		.where<FMarshalToActor, FActorReference>()
		.add_read<FActorTransform, FMarshalToActor>()
		.add_write<FActorReference, FActorTransform>()
		.schedule([](auto &observer, ecs_registry &reg) {
			for (const auto &id : observer) {
				FMarshalToActor &marshalling = reg.get<FMarshalToActor>(id);

				TWeakObjectPtr<AActor> &ref = reg.get<FActorReference>(id).actor;
				check(ref.IsValid());
				AActor *actor = ref.Get();

				FActorTransform &transform = reg.get<FActorTransform>(id);
				FHitResult results;

				TFunction<bool(const FTransform &, bool, FHitResult *)> func;

				if (marshalling.world_space) {
					func = [actor](const FTransform &t, bool s,
						       FHitResult *r) -> bool {
						actor->SetActorTransform(t, s, r);
						return r->bBlockingHit;
					};
				} else {
					func = [actor](const FTransform &t, bool s,
						       FHitResult *r) -> bool {
						actor->SetActorRelativeTransform(t, s, r);
						return r->bBlockingHit;
					};
				}

				if (func(transform, marshalling.sweep, &results) &&
				    marshalling.sweep) {
					transform.SetLocation(results.Location);
				}
			}
			UE_LOG(LogEnttUE, Log, TEXT("Ran: Marshal to Actor"));
			observer.clear();
		});
}
} // namespace tc
