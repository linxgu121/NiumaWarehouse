#pragma once

#include "CoreMinimal.h"

#include "PlayerLocomotionType.generated.h"


UENUM(BlueprintType)
enum class EPlayerLocomotionState : uint8
{
	Idle = 0,
	Walk = 1,
	Jog = 2,
	Sprint = 3,
	Crouch = 4
};