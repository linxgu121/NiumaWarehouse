#include "NiumaTPC/Character/StateMachine/StateMachine.h"


FStateBase* FStateMachine::GetCurrentState() const
{
	return CurrentState;
}

void FStateMachine::Initialize(FStateBase* startingState)
{
	if (!startingState || CurrentState || bIsShuttingDown)
	{
		return;
	}

	CurrentState = startingState;
	CurrentState->Enter();
}

void FStateMachine::ChangeState(FStateBase* newState)
{
	if (!newState || CurrentState == newState || bIsShuttingDown) 
	{
		return;
	}

	if (CurrentState)
	{
		CurrentState->Exit();
	}

	CurrentState = newState;
	CurrentState->Enter();
}

void FStateMachine::Shutdown()
{
	if (bIsShuttingDown || !CurrentState)
	{
		return;
	}

	bIsShuttingDown = true;

	//暂存当前状态指针
	FStateBase* StateToExit = CurrentState;

	// Exit可能间接查询状态机，
	// 此时状态机应已经处于“无当前状态”
	// 先把 CurrentState 置空，再调 Exit()
	// 如果 Exit() 里又访问/修改状态机，此时 CurrentState 已经是 nullptr
	CurrentState = nullptr;

	StateToExit->Exit();

	bIsShuttingDown = false;
}

