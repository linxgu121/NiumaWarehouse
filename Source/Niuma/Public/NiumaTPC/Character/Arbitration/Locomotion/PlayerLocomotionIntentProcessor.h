#pragma once

#include "CoreMinimal.h"

#include "NiumaTPC/Character/Motion/PlayerLocomotionType.h"

struct FPlayerLocomotionIntent;
struct FPlayerLocomotionPermissions;
struct FPlayerRuntimeData;

/// <summary>
/// 步态仲裁器
/// </summary>
class FPlayerLocomotionIntentProcessor final
{
public:
	void Update(
		FPlayerRuntimeData& RuntimeData,
		const FPlayerLocomotionIntent& Intent,
		const FPlayerLocomotionPermissions& Permissions) const;
private:
	/// <summary>
	/// 核心仲裁决策，根据意图 + 权限算出目标运动状态
	/// </summary>
	EPlayerLocomotionState ResolveLocomotionState(
		const FPlayerLocomotionIntent& Intent,
		const FPlayerLocomotionPermissions& Permission) const;

	/// <summary>
	/// 把最终状态写入运行时数据生效
	/// </summary>
	void CommitLocomotionState(
		FPlayerRuntimeData& RuntimeData,
		EPlayerLocomotionState NewState) const;
};