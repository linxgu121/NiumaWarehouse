#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakInterfacePtr.h"

#include "NiumaTPC/Character/Animation/IPlayerAnimationFacade.h"

class ANiumaCharacter;
class UNiumaPlayerLocomotionAnimationData;
class UCharacterMovementComponent;
struct FPlayerRuntimeData;


/// <summary>
/// Context 集中保存依赖
/// </summary>
struct FPlayerStateContext 
{
public:
	void Initialize(
		ANiumaCharacter* InCharacter,
		UCharacterMovementComponent* InMovementComponent,
		FPlayerRuntimeData* InRuntimeData,
		IPlayerAnimationFacade* InAnimationFacade,
		UNiumaPlayerLocomotionAnimationData*
		InLocomotionAnimationData);

	void Reset();

	void SetMovementComponent(UCharacterMovementComponent* InMovementComponent);

	void SetAnimationFacade(IPlayerAnimationFacade* InAnimationFacade);

	/// <summary>
	/// 判断当前状态上下文绑定的角色对象是否正常有效
	/// </summary>
	bool HasValidOwner() const;

	bool IsReady() const;

	ANiumaCharacter* GetCharacter() const;

	UCharacterMovementComponent* GetMovementComponent() const;

	FPlayerRuntimeData* GetRuntimeData() const;

	IPlayerAnimationFacade* GetAnimationFacade() const;

	void SetLocomotionAnimationData(
		UNiumaPlayerLocomotionAnimationData*
		InLocomotionAnimationData);

	const UNiumaPlayerLocomotionAnimationData*
		GetLocomotionAnimationData() const;

private:
	//TWeakObjectPtr<UObjectClass>：UE 专属弱引用智能指针，专门用于弱持有 UObject 对象
	TWeakObjectPtr<ANiumaCharacter> Character;

	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

	TWeakObjectPtr<UNiumaPlayerLocomotionAnimationData> LocomotionAnimationData;

	FPlayerRuntimeData* RuntimeData = nullptr;

	//TWeakInterfacePtr<InterfaceType> 是接口专用弱指针
	//专门用来弱持有实现了 UINTERFACE 的抽象接口，是 TWeakObjectPtr 的接口版本
	TWeakInterfacePtr<IPlayerAnimationFacade> AnimationFacade;

};