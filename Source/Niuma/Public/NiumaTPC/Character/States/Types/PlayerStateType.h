#pragma once

#include "CoreMinimal.h"

#include "PlayerStateType.generated.h"

UENUM(BlueprintType)
enum class EPlayerStateType: uint8
{
	None = 0,
	Idle = 1,
	MoveStart = 2,
	MoveLoop = 3,
	Stop = 4,
	

	Jump = 5,
	DoubleJump = 6,
	Fall = 7,
	Land = 8,
	
	Dodge = 9,
	Roll = 10,
	Vault = 11,

	AimIdle = 12,
	AimMove = 13
};
