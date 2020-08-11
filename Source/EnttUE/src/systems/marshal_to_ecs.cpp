#include "marshal_to_ecs.hpp"
#include "components/actor_reference.h"
#include "components/actor_transform.h"
#include "components/position.h"
#include "components/scale.h"
#include "components/rotation.h"
#include "components/marshal_to_ecs.h"

namespace tc
{
void marshal_to_ecs_system::on_create()
{
}

void marshal_to_ecs_system::on_schedule()
{
	entities()
		.entered_group<FMarshalToEcs, FActorReference>()
		.add_read<FActorReference>()
		.add_write<FActorTransform>()
		.schedule([](auto &observer, ecs_registry &reg) {
			for (const auto &id : observer) {
				TWeakObjectPtr<AActor> &ref = reg.get<FActorReference>(id).actor;
				check(ref.IsValid());
				AActor *actor = ref.Get();

				const FTransform &transform = actor->GetActorTransform();
				reg.emplace_or_replace<FActorTransform>(id, transform);
			}
			observer.clear();
			UE_LOG(LogEnttUE, Log, TEXT("Ran: Marshal to ECS"));
		});

	entities()
		.entered_group<FActorTransform, FPosition>()
		.add_read<FActorTransform>()
		.add_write<FPosition>()
		.schedule_parallel([](auto &observer, ecs_registry &reg) {
			for (const auto &id : observer) {
				FActorTransform &transform = reg.get<FActorTransform>(id);
				reg.patch<FPosition>(id, [&transform](FPosition &val) {
					val = FPosition{ transform.GetLocation() };
				});
			}
			observer.clear();
			UE_LOG(LogEnttUE, Log, TEXT("Ran: Marshal to ECS - Location unpacking"));
		});

	entities()
		.entered_group<FActorTransform, FScale>()
		.add_read<FActorTransform>()
		.add_write<FScale>()
		.schedule_parallel([](auto &observer, ecs_registry &reg) {
			for (const auto &id : observer) {
				FActorTransform &transform = reg.get<FActorTransform>(id);
				reg.patch<FScale>(id, [&transform](FScale &val) {
					val = FScale{ transform.GetScale3D() };
				});
			}
			observer.clear();
			UE_LOG(LogEnttUE, Log, TEXT("Ran: Marshal to ECS - Scale unpacking"));
		});

	entities()
		.entered_group<FActorTransform, FRotation>()
		.add_read<FActorTransform>()
		.add_write<FRotation>()
		.schedule_parallel([](auto &observer, ecs_registry &reg) {
			for (const auto &id : observer) {
				FActorTransform &transform = reg.get<FActorTransform>(id);
				reg.patch<FRotation>(id, [&transform](FRotation &val) {
					val = FRotation{ transform.GetRotation() };
				});
			}
			observer.clear();
			UE_LOG(LogEnttUE, Log, TEXT("Ran: Marshal to ECS - Rotation unpacking"));
		});
}
} // namespace tc
