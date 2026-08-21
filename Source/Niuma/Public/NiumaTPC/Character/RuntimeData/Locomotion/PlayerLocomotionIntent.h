#pragma once

#include "CoreMinimal.h"

/// <summary>
/// 角色移动意图快照
/// </summary>
struct FPlayerLocomotionIntent
{
	FVector2D MoveInput = FVector2D::ZeroVector;

	bool bWalkHeld = false;
	bool bSprintHeld = false;

	static constexpr float MoveInputThresholdSquared = 0.01f;

	bool HasMoveInput() const
	{
		return MoveInput.SizeSquared() > MoveInputThresholdSquared;
	}

};

/// <summary>
/// 当前游戏规则允许玩家做什么
/// </summary>
struct FPlayerLocomotionPermissions
{
	bool bCanMove = true;
	bool bCanSprint = true;
};