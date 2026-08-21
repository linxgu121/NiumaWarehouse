#pragma once

#include "CoreMinimal.h"

#include "PlayerFootPhase.generated.h"

UENUM(BlueprintType)
enum class EPlayerFootPhase : uint8
{
	LeftFootDown = 0,
	RightFootDown = 1
};
