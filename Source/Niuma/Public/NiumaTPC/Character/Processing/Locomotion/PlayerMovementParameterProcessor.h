#pragma once

#include "CoreMinimal.h"

enum class EPlayerMoveDirection : uint8;

struct FPlayerRuntimeData;

/*
* 根据已经仲裁完成的移动输入，计算移动方向派生参数。
* 不负责决定Idle、Walk、Jog或Sprint。
*/
class FPlayerMovementParameterProcessor final 
{
public:
	/*
	* 将二维输入向量结合权威朝向
	* 转换为世界空间中的三维移动方向
	*/
	void Update(
		FPlayerRuntimeData& RuntimeData,
		float AuthorityYawDegrees,
		const FVector& CharacterForward) const;

private:
	/*
	* 将二维输入向量结合权威朝向
	* 转换为世界空间中的三维移动方向
	*/
	FVector CalculateWorldDirection(
		const FVector2D& MoveInput,
		float AuthorityYawDegrees) const;

	/*
	* 计算世界移动方向相对于角色当前面向的带符号夹角
	*/
	float CalculateLocalMoveAngle(
		const FVector& CharacterForward,
		const FVector& WorldDirection) const;

	/*
	* 将连续的角度量化为八方向离散枚举
	*/
	EPlayerMoveDirection QuantizeDirection(
		float LocalMoveAngle) const;
};
