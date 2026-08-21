#include "NiumaTPC/Character/Arbitration/Locomotion/PlayerLocomotionIntentProcessor.h"

#include "NiumaTPC/Character/RuntimeData/Locomotion/PlayerLocomotionIntent.h"
#include "NiumaTPC/Character/RuntimeData/PlayerRuntimeData.h"

void FPlayerLocomotionIntentProcessor::Update(
    FPlayerRuntimeData& RuntimeData,
    const FPlayerLocomotionIntent& Intent,
    const FPlayerLocomotionPermissions& Permissions) const
{
    const EPlayerLocomotionState NewState = ResolveLocomotionState(Intent, Permissions);

    // 无法移动或处于输入死区时，向下游提供标准零输入。
    RuntimeData.MoveInput =
        NewState == EPlayerLocomotionState::Idle
        ? FVector2D::ZeroVector
        : Intent.MoveInput;


    CommitLocomotionState(RuntimeData, NewState);
}

EPlayerLocomotionState FPlayerLocomotionIntentProcessor::ResolveLocomotionState(
    const FPlayerLocomotionIntent& Intent,
    const FPlayerLocomotionPermissions& Permissions) const
{
    if (!Permissions.bCanMove || !Intent.HasMoveInput())
    {
        return EPlayerLocomotionState::Idle;
    }

    // Sprint优先级高于Walk，与原项目一致。
    if (Intent.bSprintHeld &&
        Permissions.bCanSprint)
    {
        return EPlayerLocomotionState::Sprint;
    }

    if (Intent.bWalkHeld)
    {
        return EPlayerLocomotionState::Walk;
    }

    return EPlayerLocomotionState::Jog;
}

void FPlayerLocomotionIntentProcessor::CommitLocomotionState(
    FPlayerRuntimeData& RuntimeData,
    EPlayerLocomotionState NewState) const
{
    if (RuntimeData.CurrentLocomotionState == NewState)
    {
        return;
    }

    RuntimeData.LastLocomotionState =
        RuntimeData.CurrentLocomotionState;

    RuntimeData.CurrentLocomotionState =
        NewState;
}
