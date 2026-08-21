#include "NiumaTPC/Character/States/Locomotion/PlayerIdleState.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "NiumaTPC/Character/Motion/PlayerLocomotionType.h"
#include "NiumaTPC/Character/RuntimeData/PlayerRuntimeData.h"
#include "NiumaTPC/Character/States/PlayerStateContext.h"

FPlayerIdleState::FPlayerIdleState(FPlayerStateContext& InContext) : FPlayerBaseState(InContext, EPlayerStateType::Idle)
{
}

void FPlayerIdleState::Enter()
{
	UE_LOG(LogTemp, Display, TEXT("[PlayerIdleState] Enter"));

	FPlayerRuntimeData* RuntimeData = GetRuntimeData();

    if (!RuntimeData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PlayerIdleState] ""RuntimeData无效"));

        return;
    }

}

void FPlayerIdleState::LogicUpdate(float DeltaTime)
{
    FPlayerRuntimeData* RuntimeData = GetRuntimeData();

    UCharacterMovementComponent* Movement = GetMovementComponent();

    if (!RuntimeData || !Movement)
    {
        return;
    }

    RuntimeData->CurrentSpeed = Movement->Velocity.Size2D();
}

void FPlayerIdleState::PhysicsUpdate(float DeltaTime)
{
    // 物理移动逻辑以后交给CharacterMovement阶段
}

void FPlayerIdleState::Exit()
{
    UE_LOG(LogTemp, Display, TEXT("[PlayerIdleState] Exit"));
}
