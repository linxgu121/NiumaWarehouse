#pragma once

#include "NiumaTPC/Character/StateMachine/StateBase.h"
#include "NiumaTPC/Character/States/Types/PlayerStateType.h"

class ANiumaCharacter;
class UCharacterMovementComponent;
class IPlayerAnimationFacade;
class UNiumaPlayerLocomotionAnimationData;

struct FPlayerRuntimeData;
struct FPlayerStateContext;

class FPlayerBaseState : public FStateBase
{
public:
	explicit FPlayerBaseState(FPlayerStateContext& InContext, EPlayerStateType InStateType);

	virtual ~FPlayerBaseState() override;

	EPlayerStateType GetStateType() const;
protected:
	bool HasValidOwner() const;

	FPlayerStateContext& GetContext() const;

	ANiumaCharacter* GetCharacter() const;

	UCharacterMovementComponent* GetMovementComponent() const;

	FPlayerRuntimeData* GetRuntimeData() const;

	IPlayerAnimationFacade* GetAnimationFacade() const;

	const UNiumaPlayerLocomotionAnimationData* GetLocomotionAnimationData() const;


private:
	//Context对象固定存在，但它内部的组件允许失效和改绑
	FPlayerStateContext& Context;

	//初始化角色状态
	EPlayerStateType StateType = EPlayerStateType::None;
};


