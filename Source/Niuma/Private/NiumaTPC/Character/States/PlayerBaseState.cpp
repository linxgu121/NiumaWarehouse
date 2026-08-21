#include "NiumaTPC/Character/States/PlayerBaseState.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "NiumaTPC/Character/Animation/IPlayerAnimationFacade.h"
#include "NiumaTPC/Character/NiumaCharacter.h"
#include "NiumaTPC/Character/RuntimeData/PlayerRuntimeData.h"
#include "NiumaTPC/Character/States/PlayerStateContext.h"
#include "NiumaTPC/Character/Config/Animation/PlayerLocomotionAnimationData.h"

FPlayerBaseState::FPlayerBaseState(
	FPlayerStateContext& InContext,
	EPlayerStateType InStateType)
	: Context(InContext),
	  StateType(InStateType)
{
	ensureMsgf(StateType != EPlayerStateType::None, TEXT("PlayerBaseState must have object"));
}

FPlayerBaseState::~FPlayerBaseState() = default;

bool FPlayerBaseState::HasValidOwner() const
{
	return Context.HasValidOwner();
}

FPlayerStateContext& FPlayerBaseState::GetContext() const
{
	return Context;
}

ANiumaCharacter* FPlayerBaseState::GetCharacter() const 
{
	return Context.GetCharacter();
}

UCharacterMovementComponent* FPlayerBaseState::GetMovementComponent() const
{
	return Context.GetMovementComponent();
}

EPlayerStateType FPlayerBaseState::GetStateType() const
{
	return StateType;
}

FPlayerRuntimeData* FPlayerBaseState::GetRuntimeData() const
{
	return Context.GetRuntimeData();
}

IPlayerAnimationFacade* FPlayerBaseState::GetAnimationFacade() const
{
	return Context.GetAnimationFacade();
}

const UNiumaPlayerLocomotionAnimationData* FPlayerBaseState::GetLocomotionAnimationData() const
{
	return Context.GetLocomotionAnimationData();
}

