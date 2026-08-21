#pragma once

#include "CoreMinimal.h"
#include "NiumaTPC/Character/StateMachine/StateBase.h"

/**
 * 状态机类，用于管理和切换不同的状态
 */
class FStateMachine
{
public:
	
	FStateBase* GetCurrentState() const;

	/// <summary>
	/// 初始化状态机，设置初始状态
	/// </summary>
	/// <param name="startingState">初始状态</param>
	void Initialize(FStateBase* startingState);

	/// <summary>
	/// 改变状态机的当前状态，调用当前状态的Exit方法，并调用新状态的Enter方法
	/// </summary>
	/// <param name="newState">下一个状态</param>
	void ChangeState(FStateBase* newState);

	void Shutdown();

private:
	FStateBase* CurrentState = nullptr;

	bool bIsShuttingDown = false;
};
