#pragma once

#include "CoreMinimal.h"

#include "PlayerMoveDirection.generated.h"

UENUM(BlueprintType)
/*
* 角色移动朝向
*/
enum class EPlayerMoveDirection : uint8
{
	None = 0,

	Forward = 1,
	ForwardRight = 2,
	Right = 3,
	BackwardRight = 4,
	Backward = 5,
	BackwardLeft = 6,
	Left = 7,
	ForwardLeft = 8
};
