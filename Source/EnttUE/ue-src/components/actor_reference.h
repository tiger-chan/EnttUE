#pragma once

#include "GameFramework/Actor.h"
#include "actor_reference.generated.h"

USTRUCT(BlueprintType)
struct ENTTUE_API FActorReference {
	GENERATED_BODY()

    public:
	TWeakObjectPtr<AActor> actor;
};
