#include "NiumaTPC/Character/States/PlayerStateContext.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "NiumaTPC/Character/NiumaCharacter.h"
#include "NiumaTPC/Character/Animation/IPlayerAnimationFacade.h"
#include "NiumaTPC/Character/RuntimeData/PlayerRuntimeData.h"
#include "NiumaTPC/Character/Config/Animation/PlayerLocomotionAnimationData.h"

void FPlayerStateContext::Initialize(
	ANiumaCharacter* InCharacter,
	UCharacterMovementComponent* InMovementComponent,
	FPlayerRuntimeData* InRuntimeData,
	IPlayerAnimationFacade* InAnimationFacade,
	UNiumaPlayerLocomotionAnimationData*
	InLocomotionAnimationData)
{
	Character = InCharacter;
	MovementComponent = InMovementComponent;
	RuntimeData = InRuntimeData;
	AnimationFacade = InAnimationFacade;
	LocomotionAnimationData = InLocomotionAnimationData;
}

void FPlayerStateContext::Reset()
{
	//Reset(智能指针使用)清空容器 / 指针引用，恢复为初始空无效状态
	LocomotionAnimationData.Reset();
	AnimationFacade.Reset();
	MovementComponent.Reset();
	Character.Reset();
	
	RuntimeData = nullptr;
}

void FPlayerStateContext::SetMovementComponent(UCharacterMovementComponent* InMovementComponent)
{
	MovementComponent = InMovementComponent;
}

void FPlayerStateContext::SetAnimationFacade(IPlayerAnimationFacade* InAnimationFacade)
{
	AnimationFacade = InAnimationFacade;
}


bool FPlayerStateContext::HasValidOwner() const 
{
	return Character.IsValid();
}

bool FPlayerStateContext::IsReady() const
{
	return
		Character.IsValid() &&
		MovementComponent.IsValid() &&
		RuntimeData != nullptr &&
		AnimationFacade.IsValid() &&
		LocomotionAnimationData.IsValid();
}

ANiumaCharacter* FPlayerStateContext::GetCharacter() const
{
	return Character.Get();
}

UCharacterMovementComponent* FPlayerStateContext::GetMovementComponent() const
{
	if (!HasValidOwner())
	{
		return nullptr;
	}

	return MovementComponent.Get();
}

FPlayerRuntimeData* FPlayerStateContext::GetRuntimeData() const
{
	if(!HasValidOwner())
	{
		return nullptr;
	}

	return RuntimeData;
}

IPlayerAnimationFacade* FPlayerStateContext::GetAnimationFacade() const
{
	if (!HasValidOwner())
	{
		return nullptr;
	}

	return AnimationFacade.Get();
}

void FPlayerStateContext::SetLocomotionAnimationData(
	UNiumaPlayerLocomotionAnimationData*
	InLocomotionAnimationData)
{
	LocomotionAnimationData =
		InLocomotionAnimationData;
}

const UNiumaPlayerLocomotionAnimationData*
FPlayerStateContext::GetLocomotionAnimationData() const
{
	if (!HasValidOwner())
	{
		return nullptr;
	}

	return LocomotionAnimationData.Get();
}

