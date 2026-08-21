#pragma once

#include "CoreMinimal.h"

#include "NiumaTPC/Character/Motion/PlayerLocomotionType.h"
#include "NiumaTPC/Character/Motion/PlayerMoveDirection.h"

/// <summary>
/// 数据黑板，存储玩家的运行时数据
/// </summary>
struct FPlayerRuntimeData
{
	//玩家二维移动输入
	FVector2D MoveInput = FVector2D::ZeroVector;

	//上一个运动状态
	EPlayerLocomotionState LastLocomotionState = EPlayerLocomotionState::Idle;

	//当前运动状态
	EPlayerLocomotionState CurrentLocomotionState = EPlayerLocomotionState::Idle;

	//世界空间中玩家的期望移动方向
	FVector DesiredWorldDirection = FVector::ZeroVector;

	//相对于角色朝向的移动角度
	float DesiredLocalMoveAngle = 0.0f;

	// 量化后的八方向移动意图，供起步、闪避、翻滚动画选择
	EPlayerMoveDirection DesiredMoveDirection = EPlayerMoveDirection::None;

	//当前速度
	float CurrentSpeed = 0.0f;

	//当前是否接触地面
	bool bIsGrounded = false;

	//本帧是否想要跳跃
	bool bWantsToJump = false;


	// 判断当前帧是否有移动输入
	bool HasMoveInput() const;

	//每帧结束时清空"一次性"输入（如 Jump、Attack 的按下事件）
	void ResetFrameIntents();

};

